//
//  keplerivworld.cpp
//  wakuseiboukan
//
//  Dynamic World Model
//
//  Created by Rodrigo Ramele on 24/05/14.
//  Copyright (c) 2014 Baufest. All rights reserved.
//

#define dSINGLE

#include <vector>
#include <mutex>
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <regex>

#include "container.h"

#include "odeutils.h"

#include "sounds/sounds.h"

#include "terrain/Terrain.h"
#include "camera.h"
#include "engine.h"

#include "keplerivworld.h"

// Add a new interface to an embodied object.
#include "units/BoxVehicle.h"
#include "units/Walrus.h"
#include "units/Manta.h"
#include "units/SimplifiedDynamicManta.h"
#include "units/Buggy.h"
#include "units/MultiBodyVehicle.h"
#include "units/Balaenidae.h"
#include "units/Beluga.h"
#include "units/AdvancedWalrus.h"
#include "units/Medusa.h"

#include "structures/Structure.h"
#include "structures/Runway.h"
#include "structures/Hangar.h"
#include "structures/Turret.h"
#include "structures/Warehouse.h"
#include "structures/Laserturret.h"
#include "structures/CommandCenter.h"
#include "structures/Launcher.h"

#include "actions/Gunshot.h"
#include "actions/Missile.h"

extern  Camera Camera;

extern  Controller controller;

/* dynamics and collision objects */

static dGeomID platform, ground;

unsigned long timer=0;

dWorldID world;
dSpaceID space;
dBodyID body[NUM];
dJointGroupID contactgroup;

container<Vehicle*> entities;

std::vector<BoxIsland*> islands;

std::vector<Message> messages;

int gamemode;


/**
 * Collision detection function.
 *
 * This is one of the most important pieces of code of this program.  It handles too many things.
 * vTC26: Islands now contain their own ODE Space, which depends on the world space.
 * The heightmap associated with each island is constructed on that space.  All the structures are associated with each
 * space from each island.
 *
 * All the units are generated in the world space.  Hence this function first check both geoms and calls dSpaceCollid2 which
 * verifies if some moving object may be colliding with some space (i.e. island).  If there is a collition this callback is called
 * again and those collisions are handled.
 *
 * This is much faster.  I verified that by doing this procedure (check TEST 26) the fps of 175 entities improves from 25 to 60 almost
 * the highest possible in this platform.
 *
 *
 * @brief nearCallback
 * @param data
 * @param o1
 * @param o2
 */
void nearCallback (void *data, dGeomID o1, dGeomID o2)
{
    int i,n;

    dBodyID b1,b2;

    assert(o1);
    assert(o2);

    if (dGeomIsSpace(o1) || dGeomIsSpace(o2))
    {
      //fprintf(stderr,"testing space %p %p\n", (void*)o1, (void*)o2);
      // colliding a space with something
      dSpaceCollide2(o1,o2,data,&nearCallback);
      // Note we do not want to test intersections within a space,
      // only between spaces.
      // @NOTE: If you ever want to test collisions within each space, call dSpaceCollide2((dSpaceID)o1) and the same for o2.
      return;
    }

    // only collide things with the ground
    int g1 = (o1 == ground );
    int g2 = (o2 == ground );
    if (!(g1 ^ g2))
    {
        //printf ("Ground colliding..\n");

        //return;
    }


    b1 = dGeomGetBody(o1);
    b2 = dGeomGetBody(o2);
    if (b1 && b2 && dAreConnected (b1,b2)) return;

    if (b1 && isAction(o1) && b2 && (isType(o2,WALRUS) || (isType(o2,MANTA))) && isMineFire(gVehicle(o2),(Gunshot*)gVehicle(o1)) ) return;
    if (b2 && isAction(o2) && b1 && (isType(o1,WALRUS) || (isType(o1,MANTA))) && isMineFire(gVehicle(o1),(Gunshot*)gVehicle(o2)) ) return;

    int val[]={CARRIER,WALRUS,MANTA};

    if (o1 && isRay(o1) && b2 && isType(o2,val,3) && rayHit(gVehicle(o2),(LaserRay*)gVehicle(o1))) {return;}
    if (o2 && isRay(o2) && b1 && isType(o1,val,3) && rayHit(gVehicle(o1),(LaserRay*)gVehicle(o2))) {return;}

    const int N = 10;
    dContact contact[N];
    n = dCollide (o1,o2,N,&contact[0].geom,sizeof(dContact));
    if (n > 0) {
        for (i=0; i<n; i++) {

            Vehicle *v1=NULL,*v2=NULL;
            Structure *s1=NULL, *s2=NULL;
            gVehicle(v1,v2,s1,s2,contact[i].geom.g1,contact[i].geom.g2);


            // Bullets
            if ((isAction(v1) && isWalrus(v2) && isMineFire(v2,(Gunshot*)v1))
                ||
               (isAction(v2) && isWalrus(v1) && isMineFire(v1,(Gunshot*)v2)))
            {
                // Water buyoncy reaction
                contact[i].surface.mode = dContactSlip1 | dContactSlip2 |
                dContactSoftERP | dContactSoftCFM | dContactApprox1;
                //printf("1\n");
                contact[i].surface.mu = 0.0f;
                contact[i].surface.slip1 = 1.0f;
                contact[i].surface.slip2 = 1.0f;
                contact[i].surface.soft_erp = 1.0f;   // 0 in both will force the surface to be tight.
                contact[i].surface.soft_cfm = 1.0f;
            } else
            if ( isAction(v1) || isAction(v2))
            {
                contact[i].surface.mode = dContactSlip1 | dContactSlip2 |
                dContactSoftERP | dContactSoftCFM | dContactApprox1;
                contact[i].surface.mu = 0;
                contact[i].surface.slip1 = 0.1f;
                contact[i].surface.slip2 = 0.1f;
                //printf("2\n");
                if (isAction(v1) && isCarrier(v2) && hit(v2,(Gunshot*)v1)) {}
                if (isAction(v2) && isCarrier(v1) && hit(v1,(Gunshot*)v2)) {}
                if (isAction(v1) && isManta(v2) && hit(v2,(Gunshot*)v1)) {}
                if (isAction(v2) && isManta(v1) && hit(v1,(Gunshot*)v2)) {}
                if (isAction(v1) && isWalrus(v2) && hit(v2,(Gunshot*)v1)) {}
                if (isAction(v2) && isWalrus(v1) && hit(v1,(Gunshot*)v2)) {}
                if (isAction(v1) && s2 && hit(s2, (Gunshot*)v1)) {}
                if (isAction(v2) && s1 && hit(s1, (Gunshot*)v2)) {}
            } else
            if ( ( isManta(v1) && isCarrier(v2) && releasecontrol(v1) ) ||
                 ( isManta(v2) && isCarrier(v1) && releasecontrol(v2) ) )
                {
                // Manta landing on Carrier
                contact[i].surface.mode = dContactBounce |
                dContactApprox1;
                //printf("3\n");
                contact[i].surface.mu = dInfinity;
                contact[i].surface.slip1 = 0.0f;
                contact[i].surface.slip2 = 0.0f;
                contact[i].surface.bounce = 0.2f;
            } else
            if  (isRunway(s1) || isRunway(s2))
            {
                // Manta landing on Runways.
                contact[i].surface.mode = dContactBounce |
                dContactApprox1;
                //printf("Landing on Runways...\n");

                contact[i].surface.mu = 0.99f;
                contact[i].surface.slip1 = 0.9f;
                contact[i].surface.slip2 = 0.9f;
                contact[i].surface.bounce = 0.2f;

                if ( isRunway(s1) && isManta(v2) && landed(v2, s1->island)) {}
                if ( isRunway(s2) && isManta(v1) && landed(v1, s2->island)) {}

            } else
            if ((v1 && isManta(v1) && s2) || (v2 && isManta(v2) && s1))
            {
                contact[i].surface.mode = dContactBounce |
                dContactApprox1;
                //printf("Hit structure\n");

                contact[i].surface.mu = 0;
                contact[i].surface.bounce = 0.2f;
                contact[i].surface.slip1 = 0.1f;
                contact[i].surface.slip2 = 0.1f;

                contact[i].surface.soft_erp = 0;   // 0 in both will force the surface to be tight.
                contact[i].surface.soft_cfm = 0;
            }
            if (isIsland(contact[i].geom.g1) || isIsland(contact[i].geom.g2))
            {
                 // Island reaction
                 contact[i].surface.mode = dContactBounce |
                 dContactApprox1;
                 //printf("5\n");
                 contact[i].surface.mu = 0;
                 contact[i].surface.bounce = 0.2f;
                 contact[i].surface.slip1 = 0.1f;
                 contact[i].surface.slip2 = 0.1f;

                 contact[i].surface.soft_erp = 0;   // 0 in both will force the surface to be tight.
                 contact[i].surface.soft_cfm = 0;


                 //if (v1 && isManta(v1) && groundcollisions(v1)) {printf("Hit manta against structure.\n");}
                 //if (v2 && isManta(v2) && groundcollisions(v2)) {printf("Hit manta against structure.\n");}

                 // Carrier stranded and Walrus arrived on island.
                 if (isIsland(contact[i].geom.g1) && isCarrier(v2) && stranded(v2,getIsland(contact[i].geom.g1))) {}
                 if (isIsland(contact[i].geom.g2) && isCarrier(v1) && stranded(v1,getIsland(contact[i].geom.g2))) {}
                 if (isIsland(contact[i].geom.g1) && isWalrus(v2)  && arrived(v2,getIsland(contact[i].geom.g1))) {}
                 if (isIsland(contact[i].geom.g2) && isWalrus(v1)  && arrived(v1,getIsland(contact[i].geom.g2))) {}

                 if (isIsland(contact[i].geom.g1) && isManta(v2)  && groundcollisions(v2)) {}
                 if (isIsland(contact[i].geom.g2) && isManta(v1)  && groundcollisions(v1)) {}

                 if (isIsland(contact[i].geom.g1) && isAction(v2) && v2->getType()==CONTROLABLEACTION) { ((Missile*)v2)->setVisible(false);}
                 if (isIsland(contact[i].geom.g2) && isAction(v1) && v1->getType()==CONTROLABLEACTION) { ((Missile*)v1)->setVisible(false);}


            } else
            if (ground == contact[i].geom.g1 || ground == contact[i].geom.g2 ) {

                 // Water buyoncy reaction
                 contact[i].surface.mode = dContactSlip1 | dContactSlip2 |
                 dContactSoftERP | dContactSoftCFM | dContactApprox1;

                 //printf("6\n");
                 contact[i].surface.mu = 0.0f;
                 contact[i].surface.slip1 = 0.1f;
                 contact[i].surface.slip2 = 0.1f;
                 contact[i].surface.soft_erp = .5f;   // 0 in both will force the surface to be tight.
                 contact[i].surface.soft_cfm = .3f;

                 // Walrus reaching shore.
                 if (ground == contact[i].geom.g1 && isWalrus(v2) && departed(v2)) {}
                 if (ground == contact[i].geom.g2 && isWalrus(v1) && departed(v1)) {}

                 if (ground == contact[i].geom.g1 && v2 && isManta(v2) && groundcollisions(v2)) {}
                 if (ground == contact[i].geom.g2 && v1 && isManta(v1) && groundcollisions(v1)) {}

                 if (v1 && isWalrus(v1)) { v1->inert = false;}
                 if (v2 && isWalrus(v2)) { v2->inert = false;}

            } else {
                // Object against object collision.
                 //printf("7\n");
                if (v1 && !isRunway(s2) && isManta(v1) && groundcollisions(v1)) {}
                if (v2 && !isRunway(s1) && isManta(v2) && groundcollisions(v2)) {}
            }

            dJointID c = dJointCreateContact (world,contactgroup,&contact[i]);
            dJointAttach (c,
                          dGeomGetBody(contact[i].geom.g1),
                          dGeomGetBody(contact[i].geom.g2));
        }
    }
}

void inline initIslands()
{
    BoxIsland *statera = new BoxIsland(&entities);
    statera->setName("Statera");
    statera->setLocation(0.0f,-1.0,0.0f);
    statera->buildTerrainModel(space,"terrain/thermopilae.bmp");

    BoxIsland *thermopilae = new BoxIsland(&entities);
    thermopilae->setName("Thermopilae");
    thermopilae->setLocation(580 kmf, -1.0, -350 kmf);
    thermopilae->buildTerrainModel(space,"terrain/thermopilae.bmp");

    BoxIsland *nonsquareisland = new BoxIsland(&entities);
    nonsquareisland->setName("Atolon");
    nonsquareisland->setLocation(0.0f,-1.0f,-100 kmf);
    nonsquareisland->buildTerrainModel(space,"terrain/nonsquareisland.bmp");

    BoxIsland *vulcano = new BoxIsland(&entities);
    vulcano->setName("Vulcano");
    vulcano->setLocation(145 kmf, -1.0f, 89 kmf);
    vulcano->buildTerrainModel(space,"terrain/vulcano.bmp");

    BoxIsland *nemesis = new BoxIsland(&entities);
    nemesis->setName("Nemesis");
    nemesis->setLocation(-450 kmf, -1.0, 300 kmf);
    nemesis->buildTerrainModel(space,"terrain/nemesis.bmp");

    BoxIsland *hera = new BoxIsland(&entities);
    hera->setName("Hera");
    hera->setLocation(-200 kmf, -1.0, 200 kmf);
    hera->buildTerrainModel(space,"terrain/nemesis.bmp");

    BoxIsland *hestia = new BoxIsland(&entities);
    hestia->setName("Hestia");
    hestia->setLocation(-250 kmf, -1.0, 250 kmf);
    hestia->buildTerrainModel(space,"terrain/vulcano.bmp");

    BoxIsland *atom = new BoxIsland(&entities);
    atom->setName("Atom");
    atom->setLocation( 500 kmf, -1.0, -100 kmf);
    atom->buildTerrainModel(space,"terrain/atom.bmp");

    BoxIsland *island = new BoxIsland(&entities);
    island->setName("Island");
    island->setLocation(-500 kmf, -1.0, 200 kmf);
    island->buildTerrainModel(space,"terrain/island.bmp");

    BoxIsland *baltimore = new BoxIsland(&entities);
    baltimore->setName("Baltimore");
    baltimore->setLocation(-450 kmf, -1.0, 250 kmf);
    baltimore->buildTerrainModel(space,"terrain/baltimore.bmp");

    BoxIsland *fulcrum = new BoxIsland(&entities);
    fulcrum->setName("Fulcrum");
    fulcrum->setLocation(70 kmf, -1.0, 70 kmf);
    fulcrum->buildTerrainModel(space,"terrain/fulcrum.bmp");


    BoxIsland *vulcrum = new BoxIsland(&entities);
    vulcrum->setName("Vulcrum");
    vulcrum->setLocation(450 kmf, -1.0, -300 kmf);
    vulcrum->buildTerrainModel(space,"terrain/fulcrum.bmp");

    BoxIsland *lunae = new BoxIsland(&entities);
    lunae->setName("Lunae");
    lunae->setLocation(490 kmf, -1.0, 320 kmf);
    lunae->buildTerrainModel(space,"terrain/heightmap.bmp");

    BoxIsland *mururoa = new BoxIsland(&entities);
    mururoa->setName("Mururoa");
    mururoa->setLocation(-200 kmf, -1.0, 320 kmf);
    mururoa->buildTerrainModel(space,"terrain/thermopilae.bmp");

    BoxIsland *bikini = new BoxIsland(&entities);
    bikini->setName("Bikini");
    bikini->setLocation(-150 kmf, -1.0, -235 kmf);
    bikini->buildTerrainModel(space,"terrain/atom.bmp");

    BoxIsland *parentum = new BoxIsland(&entities);
    parentum->setName("Parentum");
    parentum->setLocation(-150 kmf, -1.0, 435 kmf);
    parentum->buildTerrainModel(space,"terrain/parentum.bmp");

    BoxIsland *goku = new BoxIsland(&entities);
    goku->setName("SonGoku");
    goku->setLocation(-200 kmf, -1.0, -435 kmf);
    goku->buildTerrainModel(space,"terrain/goku.bmp");

    BoxIsland *gaijin = new BoxIsland(&entities);
    gaijin->setName("Gaijin-shima");
    gaijin->setLocation(150 kmf, -1.0, -339 kmf);
    gaijin->buildTerrainModel(space,"terrain/gaijin.bmp");

    BoxIsland *tristan = new BoxIsland(&entities);
    tristan->setName("Tristan da Cunha");
    tristan->setLocation(250 kmf, -1.0, 10 kmf);
    tristan->buildTerrainModel(space,"terrain/tristan.bmp");

    BoxIsland *sentinel = new BoxIsland(&entities);
    sentinel->setName("North Sentinel");
    sentinel->setLocation(150 kmf, -1.0, 390 kmf);
    sentinel->buildTerrainModel(space,"terrain/sentinel.bmp");

    BoxIsland *midway = new BoxIsland(&entities);
    midway->setName("Midway");
    midway->setLocation(-150 kmf, -1.0, -290 kmf);
    midway->buildTerrainModel(space,"terrain/heightmap.bmp");

    BoxIsland *enewetak = new BoxIsland(&entities);
    enewetak->setName("Enewetak");
    enewetak->setLocation(-250 kmf, -1.0, -90 kmf);
    enewetak->buildTerrainModel(space,"terrain/thermopilae.bmp");

    islands.push_back(thermopilae);
    islands.push_back(nonsquareisland);
    islands.push_back(vulcano);
    islands.push_back(nemesis);
    islands.push_back(hestia);
    islands.push_back(hera);
    islands.push_back(atom);
    islands.push_back(island);
    islands.push_back(baltimore);
    islands.push_back(fulcrum);
    islands.push_back(vulcrum);
    islands.push_back(lunae);
    islands.push_back(mururoa);
    islands.push_back(bikini);
    islands.push_back(parentum);
    islands.push_back(goku);
    islands.push_back(gaijin);
    islands.push_back(tristan);
    islands.push_back(sentinel);
    islands.push_back(midway);
    islands.push_back(enewetak);
    islands.push_back(statera);

}

void _nearCallback (void *data, dGeomID o1, dGeomID o2)
{
    int i,n;

    dBodyID b1,b2;

    // only collide things with the ground
    int g1 = (o1 == ground );
    int g2 = (o2 == ground );
    if (!(g1 ^ g2))
    {
        //printf ("Ground colliding..\n");

        //return;
    }


    b1 = dGeomGetBody(o1);
    b2 = dGeomGetBody(o2);
    if (b1 && b2 && dAreConnected (b1,b2)) return;

    const int N = 10;
    dContact contact[N];
    n = dCollide (o1,o2,N,&contact[0].geom,sizeof(dContact));
    if (n > 0) {
        for (i=0; i<n; i++) {

            dJointID c = dJointCreateContact (world,contactgroup,&contact[i]);
            dJointAttach (c,
                          dGeomGetBody(contact[i].geom.g1),
                          dGeomGetBody(contact[i].geom.g2));
        }
    }
}

void savegame()
{
    std::ofstream ss("savegame.w", std::ios_base::binary);

    // Version
    ss << 0x01 << std::endl;

    int entitiessize = 0;
    // Get flying entities.
    for(size_t i=entities.first();entities.hasMore(i);i=entities.next(i))
    {
        if (entities[i]->getType() == CARRIER || entities[i]->getType() == MANTA || entities[i]->getType() == WALRUS)
        {
            entitiessize++;
        }
    }

    ss << entitiessize << std::endl;
    for(size_t i=entities.first();entities.hasMore(i);i=entities.next(i))
    {
        if (entities[i]->getType() == CARRIER || entities[i]->getType() == MANTA || entities[i]->getType() == WALRUS)
        {
            ss << entities[i]->getFaction() << std::endl;
            ss << entities[i]->getType() << std::endl;

            int subtype = 0;


            if (AdvancedWalrus *lb = dynamic_cast<AdvancedWalrus*>(entities[i]))
                subtype = 1;
            else if (Walrus *lb = dynamic_cast<Walrus*>(entities[i]))
                subtype = 2;
            else if (Medusa *lb = dynamic_cast<Medusa*>(entities[i]))
                subtype = 6;
            else if (SimplifiedDynamicManta *lb = dynamic_cast<SimplifiedDynamicManta*>(entities[i]))
                subtype = 3;
            else if (Beluga *lb = dynamic_cast<Beluga*>(entities[i]))
                subtype = 4;
            else if(Balaenidae* lb = dynamic_cast<Balaenidae*>(entities[i]))
                subtype = 5;

            ss << subtype << std::endl;

            std::cout << "Subtype saving:" << subtype << std::endl;

            Vec3f p= entities[i]->getPos();
            ss << p[0] << std::endl << p[1] << std::endl << p[2] << std::endl;
            ss << entities[i]->getHealth() << std::endl;
            ss << entities[i]->getPower() << std::endl;


            float R[12];
            entities[i]->getR(R);
            for(int j=0;j<12;j++) ss << R[j] << std::endl;

            ss << entities[i]->isAuto() << std::endl;
            p = entities[i]->getDestination();
            ss << p[0] << std::endl << p[1] << std::endl << p[2] << std::endl;


        }
    }


    ss << islands.size() << std::endl;
    for (int j=0;j<islands.size();j++)
    {
        std::string s;
        s = islands[j]->getName();

        s = std::regex_replace(s, std::regex(" "), "-");
        ss << s << std::endl;
        ss << islands[j]->getX() << std::endl;
        ss << islands[j]->getZ() << std::endl;
        ss << islands[j]->getModelName() << std::endl;
        std::cout << "Name:" << islands[j]->getName() << std::endl;

        Structure *c =  islands[j]->getCommandCenter();

        if (c)
        {
            std::vector<size_t> strs = islands[j]->getStructures();
            ss << 0x3f << std::endl;
            ss << strs.size() << std::endl;
            for(int i=0;i<strs.size();i++)
            {
                ss << entities[strs[i]]->getFaction() << std::endl;
                ss << entities[strs[i]]->getType() << std::endl;
                int subtype = 0;

                if (Artillery *lb = dynamic_cast<Artillery*>(entities[strs[i]]))
                    subtype = 1;
                else if (CommandCenter *lb = dynamic_cast<CommandCenter*>(entities[strs[i]]))
                    subtype = 2;
                else if (Hangar *lb = dynamic_cast<Hangar*>(entities[strs[i]]))
                    subtype = 3;
                else if (Warehouse *lb = dynamic_cast<Warehouse*>(entities[strs[i]]))
                    subtype = 4;
                else if (Runway *lb = dynamic_cast<Runway*>(entities[strs[i]]))
                    subtype = 5;
                else if (LaserTurret *lb = dynamic_cast<LaserTurret*>(entities[strs[i]]))
                    subtype = 6;
                else if (Turret *lb = dynamic_cast<Turret*>(entities[strs[i]]))
                    subtype = 7;
                else if (Launcher *l = dynamic_cast<Launcher*>(entities[strs[i]]))
                    subtype = 9;
                else if(Structure* lb = dynamic_cast<Structure*>(entities[strs[i]]))
                    subtype = 8;

                ss << subtype << std::endl;
                std::cout << "Subtype saving:" << subtype << std::endl;


                Vec3f p= entities[strs[i]]->getPos();
                ss << p[0] << std::endl << p[1] << std::endl << p[2] << std::endl;
                float orientation = getAzimuthRadians(entities[strs[i]]->getForward());
                ss << orientation << std::endl;
                ss << entities[strs[i]]->getHealth() << std::endl;
                ss << entities[strs[i]]->getPower() << std::endl;

            }
        } else {
            ss << 0x4f << std::endl;
        }


    }

    ss.flush();
    ss.close();


}



void loadgame()
{
    /**std::ifstream ss("savegame.w", std::ios_base::binary);

    Vec3f f(0,0,0);
    ss >> f[0] >> f[1] >> f[2] ;

    std::cout << f << std::endl;

    ss.close();**/

    std::ifstream ss("savegame.w", std::ios_base::binary);

    int version;
    ss >> version;

    int size;
    ss >> size;
    std::cout << "Size:" << size << std::endl;
    for(int i=0;i<size;i++)
    {
        Vehicle *v = NULL;
        int faction;
        ss >> faction;
        int type, subtype;
        ss >> type;
        ss >> subtype;
        std::cout << "Type:" << type << " subtype:" << subtype << std::endl;

        if (type == CARRIER || type == MANTA || type == WALRUS)
        {
            switch (type) {
            case CARRIER:
            {
                Balaenidae *b = NULL;
                if (subtype==5)
                    b = new Balaenidae(faction);
                else if (subtype==4)
                    b = new Beluga(faction);

                b->init();
                b->embody(world,space);
                v = b;
                break;
            }
            case MANTA:
            {
                SimplifiedDynamicManta *_manta1 = NULL;

                if (subtype == 6)
                    _manta1 = new Medusa(faction);
                else
                    _manta1 = new SimplifiedDynamicManta(faction);

                _manta1->init();
                _manta1->setNumber(findNextNumber(MANTA));
                _manta1->embody(world, space);
                _manta1->setStatus(Manta::ON_DECK);
                _manta1->inert = true;
                v = _manta1;
                break;
            }
            case WALRUS:
                Walrus *_walrus = NULL;
                if (subtype == 1)
                    _walrus = new AdvancedWalrus(faction);
                else if (subtype == 2)
                    _walrus = new Walrus(faction);
                _walrus->init();
                _walrus->setNumber(findNextNumber(WALRUS));
                _walrus->embody(world, space);
                _walrus->setStatus(Walrus::SAILING);
                _walrus->inert = true;
                v = _walrus;

            }
            Vec3f f(0,0,0);
            ss >> f[0] >> f[1] >> f[2] ;
            v->setPos(f);
            float health;ss >> health ;
            float power; ss >> power ;

            float R[12];
            for(int j=0;j<12;j++) ss >> R[j];
            v->setR(R);

            // Destination and auto
            bool isauto;
            ss >> isauto;
            ( isauto ? v->enableAuto() : v->disableAuto());

            ss >> f[0] >> f[1] >> f[2] ;
            v->setDestination(f);

            entities.push_back(v, v->getGeom());
        }
    }

    ss >> size;
    std::cout << "Size:" << size << std::endl;
    for (int j=0;j<size;j++)
    {
        BoxIsland *is = new BoxIsland(&entities);
        std::string name,modelname;
        Vec3f loc;
        ss >> name;is->setName(name);
        ss >> loc[0];
        ss >> loc[2];
        is->setLocation(loc[0],-1,loc[2]);
        ss >> modelname;
        std::cout << "Reading Island:" << name << "\t" << modelname << std::endl;
        is->buildTerrainModel(space,modelname.c_str());

        islands.push_back(is);

        int moredata;

        ss >> moredata;

        if (moredata == 0x3f)
        {
            int sze;
            ss >> sze;
            for (int i=0;i<sze;i++)
            {
                Structure *v = NULL;
                int faction;
                ss >> faction;
                int type, subtype;
                ss >> type;
                ss >> subtype;
                std::cout << "Type:" << type << " subtype:" << subtype << std::endl;

                switch (subtype) {
                case 1:
                    v = new Artillery(faction);
                    break;
                case 2:
                    v = new CommandCenter(faction);
                    break;
                case 3:
                    v = new Hangar(faction);
                    break;
                case 4:
                    v = new Warehouse(faction);
                    break;
                case 5:
                    v = new Runway(faction);
                    break;
                case 6:
                    v = new LaserTurret(faction);
                    break;
                case 7:
                    v = new Turret(faction);
                    break;
                case 9:
                    v = new Launcher(faction);
                    break;
                case 8:
                    v = new Structure(faction);
                    break;
                }

                Vec3f f(0,0,0);
                ss >> f[0] >> f[1] >> f[2] ;
                float orientation; ss >> orientation;
                float health;ss >> health ;
                float power; ss >> power ;

                is->addStructure(v   ,       is->getX()-f[0],    is->getZ()-f[2],orientation,world);

            }
        }

    }


    ss.close();

}



void initWorldPopulation()
{

    if (gamemode == STRATEGYGAME)
    {
        // Strategy Game
        Balaenidae *_b = new Balaenidae(GREEN_FACTION);
        _b->init();
        _b->embody(world,space);
        //_b->setPos(0.0f + 0.0 kmf,20.5f,-4000.0f + 0.0 kmf);
        _b->setPos(580 kmf, 20.5f, -350 kmf - 4000.0f);
        _b->stop();

        entities.push_back(_b, _b->getGeom());


        Beluga *_bg = new Beluga(BLUE_FACTION);
        _bg->init();
        _bg->embody(world,space);
        _bg->setPos(-450 kmf, -1.0, 300 kmf - 6000.0f);
        //_bg->setPos(0.0f + 0.0 kmf,20.5f,-6000.0f + 0.0 kmf);
        _bg->stop();

        entities.push_back(_bg, _bg->getGeom());
    }
    else if (gamemode == ACTIONGAME)
    {
        // Action Game
        Balaenidae *_b = new Balaenidae(GREEN_FACTION);
        _b->init();
        _b->embody(world,space);
        _b->setPos(0.0f + 0.0 kmf,20.5f,-4000.0f + 0.0 kmf);
        _b->setPos(0.0f + 0.0 kmf,20.5f,-16000.0f + 0.0 kmf);
        //_b->setPos(580 kmf, 20.5f, -350 kmf - 4000.0f);
        _b->stop();

        entities.push_back(_b,_b->getGeom());


        Beluga *_bg = new Beluga(BLUE_FACTION);
        _bg->init();
        _bg->embody(world,space);
        _bg->setPos(-450 kmf, -1.0, 300 kmf - 6000.0f);
        //_bg->setPos(0.0f + 0.0 kmf,20.5f,-6000.0f + 0.0 kmf);
        _bg->setPos(150 kmf, -1.0, -340 kmf - 4000.0f);
        _bg->stop();

        entities.push_back(_bg, _bg->getGeom());


        for (int j=0;j<islands.size();j++)
        {
            if (islands[j]->getX()<10 || (islands[j]->getName().find("Gaijin") != std::string::npos))   //@FIXME
                captureIsland(islands[j],BLUE_FACTION,space,world);
            else if (islands[j]->getZ()< (200 kmf))
                captureIsland(islands[j],GREEN_FACTION,space,world);
        }
    }

}

void initWorldModelling(int testcase)
{
    initWorldModelling();
}
void setupWorldModelling()
{
    /* create world */
    dRandSetSeed(1);
    dInitODE();
    //dInitODE2(dInitFlagManualThreadCleanup);
    //dAllocateODEDataForThread(dAllocateMaskAll);
    world = dWorldCreate();
    space = dHashSpaceCreate (0);

    //dWorldSetAutoDisableFlag(World, 1);

    // The parameter needs to be zero.
    contactgroup = dJointGroupCreate (0);
    dWorldSetGravity (world,0,-9.81f,0);
    dWorldSetCFM (world,1e-2f);   //1e-5f was the originally value used.
    dWorldSetERP(world,1.0f);   // 1 is Error Correction is applied.

    // Set Damping
    dWorldSetLinearDamping(world, 0.01f);  // 0.00001
    dWorldSetAngularDamping(world, 0.005f);     // 0.005
    dWorldSetMaxAngularSpeed(world, 200);

    // Set and get the depth of the surface layer around all geometry objects. Contacts are allowed to sink into the surface layer up to the given depth before coming to rest. The default value is zero. Increasing this to some small value (e.g. 0.001) can help prevent jittering problems due to contacts being repeatedly made and broken.
    dWorldSetContactSurfaceLayer (world,0.001f);

    ground = dCreatePlane (space,0,1,0,0);

}
void initWorldModelling()
{
    initIslands();

    initWorldPopulation();
}


void update(int value);

void worldStep(int value)
{
    timer++;
    update(value);
}

void endWorldModelling()
{
    dJointGroupDestroy (contactgroup);
    dSpaceDestroy (space);
    dWorldDestroy (world);
    dCloseODE();
    
    printf("God knows his rules and he has determined that this world must be terminated.\n");
    printf("The World has been terminated.\n");
}



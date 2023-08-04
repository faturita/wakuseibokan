//
//  keplerivworld.cpp
//  wakuseiboukan
//
//  Dynamic World Model
//
//  Created by Rodrigo Ramele on 24/05/14.
//

#define dSINGLE

#include <vector>
#include <mutex>
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <regex>

#include "profiling.h"
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
#include "units/AdvancedManta.h"
#include "units/Otter.h"

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
#include "actions/Explosion.h"

#include "weapons/CarrierArtillery.h"
#include "weapons/CarrierTurret.h"
#include "weapons/CarrierLauncher.h"

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

int tracemode;

int peermode;

int aiplayer;


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
 * THIS FUNCTION MUST BE IN THIS FILE.  Otherwise, there are a lot of problems (wellcome to C++).
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

    // @NOTE Uncomment me if you have OUT OF MEMORY problems with ODE's heightmaps.
    //Vehicle *k = gVehicle(o1);
    //Vehicle *l = gVehicle(o2);

    //if (k) dout << "Vehicle1:" << k->getName() << std::endl;
    //if (l) dout << "Vehicle2:" << l->getName() << std::endl;

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
                if (isAction(v1) && s2 && !isAction(s2) && hit(s2, (Gunshot*)v1)) {}
                if (isAction(v2) && s1 && !isAction(s1) && hit(s1, (Gunshot*)v2)) {}
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
                contact[i].surface.mode = dContactFDir1 | dContactBounce |
                dContactApprox1 | dContactMu2;
                //printf("Landing on Runways...\n");

                Vec3f f;
                if      (isManta(v1)) f = v1->getForward();
                else if (isManta(v2)) f = v2->getForward();

                contact[i].fdir1[0] = f[0];
                contact[i].fdir1[1] = f[1];
                contact[i].fdir1[2] = f[2];

                contact[i].surface.mu = 0.99;
                contact[i].surface.mu2 = dInfinity;             // This prevents the side slipping while landing.
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
                 contact[i].surface.mu = 0.9;  //dInfinity;     // This enable rolling on islands, and no more slipping.  Change to 0.9, because
                 contact[i].surface.bounce = 0.2f;              //   otherwise rolling is very unstable on ODE.
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


                 if (isIsland(contact[i].geom.g1) && isType(v2, WEAPON) && arrived(dGeomGetSpace(contact[i].geom.g2),getIsland(contact[i].geom.g1))) {}
                 if (isIsland(contact[i].geom.g2) && isType(v1, WEAPON) && arrived(dGeomGetSpace(contact[i].geom.g1),getIsland(contact[i].geom.g2))) {}


                 if (isIsland(contact[i].geom.g1) && isSubType(v2, CEPHALOPOD) && arrived(v2,getIsland(contact[i].geom.g1))) {}
                 if (isIsland(contact[i].geom.g2) && isSubType(v1, CEPHALOPOD) && arrived(v1,getIsland(contact[i].geom.g2))) {}

                 if (isIsland(contact[i].geom.g1) && isManta(v2)  && groundcollisions(v2)) {}
                 if (isIsland(contact[i].geom.g2) && isManta(v1)  && groundcollisions(v1)) {}

                 if (isIsland(contact[i].geom.g1) && isAction(v2) && v2->getType()==CONTROLABLEACTION) { ((Missile*)v2)->setVisible(false);groundexplosion(v2,world, space);}
                 if (isIsland(contact[i].geom.g2) && isAction(v1) && v1->getType()==CONTROLABLEACTION) { ((Missile*)v1)->setVisible(false);groundexplosion(v1,world, space);}

                 if (isIsland(contact[i].geom.g1) && isAction(v2) && v2->getType() == EXPLOTABLEACTION) {groundexplosion(v2,world, space);}
                 if (isIsland(contact[i].geom.g2) && isAction(v1) && v1->getType() == EXPLOTABLEACTION) {groundexplosion(v1,world, space);}


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

                 if (ground == contact[i].geom.g1 && isType(v2, WEAPON) && departed(dGeomGetSpace(contact[i].geom.g2))) {}
                 if (ground == contact[i].geom.g2 && isType(v1, WEAPON) && departed(dGeomGetSpace(contact[i].geom.g1))) {}

                 if (ground == contact[i].geom.g1 && v2 && isManta(v2) && groundcollisions(v2)) {}
                 if (ground == contact[i].geom.g2 && v1 && isManta(v1) && groundcollisions(v1)) {}

                 if (v1 && isWalrus(v1)) { v1->inert = false;}
                 if (v2 && isWalrus(v2)) { v2->inert = false;}

                 if (ground == contact[i].geom.g1 && isAction(v2) && v2->getType() == EXPLOTABLEACTION) {waterexplosion(v2,world, space);}
                 if (ground == contact[i].geom.g2 && isAction(v1) && v1->getType() == EXPLOTABLEACTION) {waterexplosion(v1,world, space);}


            } else {
                // Object against object collision.
                //printf("7\n");
                if (v1 && !isRunway(s2) && isManta(v1) && s2 && structurecollisions(s2,v1)) {}
                if (v2 && !isRunway(s1) && isManta(v2) && s1 && structurecollisions(s1,v2)) {}

                contact[i].surface.mu = 0.9;  //dInfinity;
                contact[i].surface.bounce = 0.2f;
                contact[i].surface.slip1 = 0.1f;
                contact[i].surface.slip2 = 0.1f;

                contact[i].surface.soft_erp = 0;   // 0 in both will force the surface to be tight.
                contact[i].surface.soft_cfm = 0;
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

    BoxIsland *arachnid = new BoxIsland(&entities);
    arachnid->setName("Arachnid");
    arachnid->setLocation(-450 kmf, -1.0, -300 kmf);
    arachnid->buildTerrainModel(space,"terrain/thermopilae.bmp");

    BoxIsland *outcrop = new BoxIsland(&entities);
    outcrop->setName("Outcrop");
    outcrop->setLocation(-450 kmf, -1.0, -210 kmf);
    outcrop->buildTerrainModel(space,"terrain/atom.bmp");

    BoxIsland *taksaven = new BoxIsland(&entities);
    taksaven->setName("Taksaven");
    taksaven->setLocation(-420 kmf, -1.0, -370 kmf);
    taksaven->buildTerrainModel(space,"terrain/sentinel.bmp");

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
    islands.push_back(arachnid);
    islands.push_back(outcrop);
    islands.push_back(taksaven);

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


void initWorldPopulation()
{

    if (gamemode == STRATEGYGAME)
    {
        // Strategy Game
        Balaenidae *_b = new Balaenidae(GREEN_FACTION);
        _b->init();
        dSpaceID carrier_space = _b->embody_in_space(world, space);
        //b->setPos(0.0f + 0.0 kmf,20.5f,-4000.0f + 0.0 kmf);
        _b->setPos(580 kmf, 20.5f, -350 kmf - 4000.0f);
        _b->stop();

        entities.push_back(_b, _b->getGeom());

        CarrierTurret * _bo= new CarrierTurret(GREEN_FACTION);
        _bo->init();
        _bo->embody(world, carrier_space);
        _bo->attachTo(world,_b, -40.0f, 20.0f + 5, -210.0f);
        _bo->stop();

        _b->addWeapon(entities.push_back(_bo, _bo->getGeom()));


        CarrierArtillery * _w1= new CarrierArtillery(GREEN_FACTION);
        _w1->init();
        _w1->embody(world, carrier_space);
        _w1->attachTo(world,_b, -40.0, 27.0f, +210.0f);
        _w1->stop();

        _b->addWeapon(entities.push_back(_w1, _w1->getGeom()));


        Beluga *_bg = new Beluga(BLUE_FACTION);
        _bg->init();
        dSpaceID carrier_space_beluga = _bg->embody_in_space(world, space);
        _bg->embody(world,space);
        // @NOTE Let the carriers fight against each other.
        _bg->setPos(-450 kmf, -1.0, 300 kmf - 6000.0f);
        //_bg->setPos(0.0f + 0.0 kmf,20.5f,-6000.0f + 0.0 kmf);
        _bg->stop();

        _bg->addWeapon(entities.push_back(_bg, _bg->getGeom()));


        CarrierTurret * _bl= new CarrierTurret(BLUE_FACTION);
        _bl->init();
        _bl->embody(world, carrier_space_beluga);
        _bl->attachTo(world,_bg, +30.0f, 20.0f - 3, +204.0f);
        _bl->stop();

        _bg->addWeapon(entities.push_back(_bl, _bl->getGeom()));

        CarrierTurret * _br= new CarrierTurret(BLUE_FACTION);
        _br->init();
        _br->embody(world, carrier_space_beluga);
        _br->attachTo(world,_bg, -45.0f, 20.0f - 3, +204.0f);
        _br->stop();

        _bg->addWeapon(entities.push_back(_br, _br->getGeom()));


        CarrierArtillery * _wr= new CarrierArtillery(BLUE_FACTION);
        _wr->init();
        _wr->embody(world, carrier_space_beluga);
        _wr->attachTo(world,_bg, -40.0, 27.0f+5, -230.0f);
        _wr->stop();

        _bg->addWeapon(entities.push_back(_wr, _wr->getGeom()));

        CarrierArtillery * _wl= new CarrierArtillery(BLUE_FACTION);
        _wl->init();
        _wl->embody(world, carrier_space_beluga);
        _wl->attachTo(world,_bg, +40.0, 27.0f+2, -230.0f);
        _wl->stop();

        _bg->addWeapon(entities.push_back(_wl, _wl->getGeom()));

        CarrierLauncher * _cf= new CarrierLauncher(BLUE_FACTION);
        _cf->init();
        _cf->embody(world, carrier_space_beluga);
        _cf->attachTo(world,_bg, +40.0, 27.0f+2, 0.0);
        _cf->stop();

        _bg->addWeapon(entities.push_back(_cf, _cf->getGeom()));



    }
    else if (gamemode == ACTIONGAME)
    {
        // Action Game
        Balaenidae *_b = new Balaenidae(GREEN_FACTION);
        _b->init();
        dSpaceID carrier_space = _b->embody_in_space(world, space);
        _b->setPos(0.0f + 0.0 kmf,20.5f,-4000.0f + 0.0 kmf);
        _b->setPos(0.0f + 0.0 kmf,20.5f,-16000.0f + 0.0 kmf);
        //_b->setPos(580 kmf, 20.5f, -350 kmf - 4000.0f);
        _b->stop();

        entities.push_back(_b,_b->getGeom());


        Beluga *_bg = new Beluga(BLUE_FACTION);
        _bg->init();
        dSpaceID carrier_space_beluga = _bg->embody_in_space(world, space);
        _bg->setPos(-450 kmf, -1.0, 300 kmf - 6000.0f);
        //_bg->setPos(0.0f + 0.0 kmf,20.5f,-6000.0f + 0.0 kmf);
        _bg->setPos(150 kmf, -1.0, -340 kmf - 4000.0f);
        _bg->stop();

        entities.push_back(_bg, _bg->getGeom());


        CarrierTurret * _bo= new CarrierTurret(GREEN_FACTION);
        _bo->init();
        _bo->embody(world, carrier_space);
        _bo->attachTo(world,_b, -40.0f, 20.0f + 5, -210.0f);
        _bo->stop();

        _b->addWeapon(entities.push_back(_bo, _bo->getGeom()));

        CarrierArtillery * _w1= new CarrierArtillery(GREEN_FACTION);
        _w1->init();
        _w1->embody(world, carrier_space);
        _w1->attachTo(world,_b, -40.0, 25.0f, +210.0f);
        _w1->stop();

        _b->addWeapon(entities.push_back(_w1, _w1->getGeom()));


        CarrierTurret * _bl= new CarrierTurret(BLUE_FACTION);
        _bl->init();
        _bl->embody(world, carrier_space_beluga);
        _bl->attachTo(world,_bg, +30.0f, 20.0f - 3, +204.0f);
        _bl->stop();

        _bg->addWeapon(entities.push_back(_bl, _bl->getGeom()));

        CarrierTurret * _br= new CarrierTurret(BLUE_FACTION);
        _br->init();
        _br->embody(world, carrier_space_beluga);
        _br->attachTo(world,_bg, -45.0f, 20.0f - 3, +204.0f);
        _br->stop();

        _bg->addWeapon(entities.push_back(_br, _br->getGeom()));


        CarrierArtillery * _wr= new CarrierArtillery(BLUE_FACTION);
        _wr->init();
        _wr->embody(world, carrier_space_beluga);
        _wr->attachTo(world,_bg, -40.0, 27.0f+5, -230.0f);
        _wr->stop();

        _bg->addWeapon(entities.push_back(_wr, _wr->getGeom()));

        CarrierArtillery * _wl= new CarrierArtillery(BLUE_FACTION);
        _wl->init();
        _wl->embody(world, carrier_space_beluga);
        _wl->attachTo(world,_bg, +40.0, 27.0f+2, -230.0f);
        _wl->stop();

        _bg->addWeapon(entities.push_back(_wl, _wl->getGeom()));

        CarrierLauncher * _cf= new CarrierLauncher(BLUE_FACTION);
        _cf->init();
        _cf->embody(world, carrier_space_beluga);
        _cf->attachTo(world,_bg, +40.0, 27.0f+2, 0.0);
        _cf->stop();

        _bg->addWeapon(entities.push_back(_cf, _cf->getGeom()));


        for (size_t j=0;j<islands.size();j++)
        {
            // @FIXME Decide which island to create.
            int which = (rand() % 3);
            if (islands[j]->getX()<10 || (islands[j]->getName().find("Gaijin") != std::string::npos))   //@FIXME Randomize how islands are distribuited
                captureIsland(islands[j],BLUE_FACTION,which, space,world);
            else if (islands[j]->getZ()< (200 kmf))
                captureIsland(islands[j],GREEN_FACTION,which, space,world);
        }

        // Accelerate time
        for (int j=0;j<1000;j++)
        {
            buildAndRepair(true,space,world);
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

    // Default disable parameters for newly created objects.
    //dWorldSetAutoDisableFlag(world, 1);

    //dWorldSetAutoDisableLinearThreshold(world, 0.01);
    //dWorldSetAutoDisableAngularThreshold(world, 0.01);
    //dWorldSetAutoDisableTime(world, 20);

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

   if (tracemode != REPLAY && peermode != CLIENT) initWorldPopulation();
}


void update(int value);
void replayupdate(int value);

void worldStep(int value)
{
    timer++;
    if (tracemode==REPLAY || peermode==CLIENT)
        replayupdate(value);
    else
        update(value);
}

void endWorldModelling()
{
    dJointGroupDestroy (contactgroup);
    dSpaceDestroy (space);
    dWorldDestroy (world);
    dCloseODE();
    
    CLog::Write(CLog::Debug,"God knows his rules and he has determined that this world must be terminated.\n");
    CLog::Write(CLog::Debug,"The World has been terminated.\n");
}



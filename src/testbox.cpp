/* ============================================================================
**
** TestBox Program - Wakuseiboukan - 22/05/2014
**
** Copyright (C) 2014  Rodrigo Ramele
**
** For personal, educationnal, and research purpose only, this software is
** provided under the Gnu GPL (V.3) license. To use this software in
** commercial application, please contact the author.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License V.3 for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
**
** ========================================================================= */

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <stdarg.h>
#include <math.h>

#ifdef __APPLE__
#include <GLUT/glut.h>
#elif __linux
#include <GL/glut.h>
#endif

#include <ode/ode.h>

#include <vector>
#include <mutex>
#include <regex>

#include <unordered_map>

#include "ThreeMaxLoader.h"

#include "font/DrawFonts.h"

#include "math/yamathutil.h"

#include "container.h"
#include "profiling.h"

#include "usercontrols.h"
#include "camera.h"

#include "openglutils.h"
#include "odeutils.h"

#include "imageloader.h"
#include "terrain/Terrain.h"

#include "sounds/sounds.h"

#include "engine.h"
#include "keplerivworld.h"
#include "usercontrols.h"
#include "savegame.h"

#include "units/Vehicle.h"
#include "units/BoxVehicle.h"
#include "units/Manta.h"
#include "units/Walrus.h"
#include "units/AdvancedWalrus.h"
#include "units/Balaenidae.h"
#include "units/SimplifiedDynamicManta.h"
#include "units/Medusa.h"
#include "units/Cephalopod.h"
#include "units/AdvancedManta.h"

#include "actions/Gunshot.h"
#include "actions/Missile.h"

#include "structures/Structure.h"
#include "structures/Warehouse.h"
#include "structures/Hangar.h"
#include "structures/Runway.h"
#include "structures/Laserturret.h"
#include "structures/CommandCenter.h"
#include "structures/Turret.h"
#include "structures/Artillery.h"
#include "structures/Launcher.h"
#include "structures/Antenna.h"
#include "structures/Dock.h"
#include "structures/Radar.h"
#include "structures/Factory.h"

#include "weapons/Weapon.h"
#include "weapons/CarrierTurret.h"
#include "weapons/CarrierArtillery.h"

#include "units/Wheel.h"
#include "units/WheeledManta.h"
#include "units/Otter.h"

#include "actions/ArtilleryAmmo.h"
#include "actions/Debris.h"
#include "actions/Explosion.h"
#include "actions/Torpedo.h"
#include "actions/Bomb.h"

#include "map.h"

#include "math/uuid.h"

extern  Camera Camera;

extern  Controller controller;

/* dynamics and collision objects */

static dGeomID ground;

unsigned long timer=0;

dWorldID world;
dSpaceID space;
dJointGroupID contactgroup;

container<Vehicle*> entities;

std::vector<BoxIsland*> islands;

std::vector<Message> messages;

extern float fps;
extern clock_t elapsedtime;

int gamemode;
int aiplayer;
int tracemode;
int peermode;

extern bool wincondition;

extern std::unordered_map<std::string, GLuint> textures;
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
                if (v1 && !isRunway(s2) && isManta(v1) && groundcollisions(v1)) {}
                if (v2 && !isRunway(s1) && isManta(v2) && groundcollisions(v2)) {}

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
void __nearCallback (void *data, dGeomID o1, dGeomID o2)
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

void inline initIslands()
{
    BoxIsland *thermopilae = new BoxIsland(&entities);
    thermopilae->setName("Thermopilae");
    thermopilae->setLocation(0.0f,-1.0,0.0f);
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

    BoxIsland *atom = new BoxIsland(&entities);
    atom->setName("Atom");
    atom->setLocation( 500 kmf, -1.0, -100 kmf);
    atom->buildTerrainModel(space,"terrain/atom.bmp");

    BoxIsland *island = new BoxIsland(&entities);
    island->setName("Island");
    island->setLocation(-500 kmf, -1.0, 200 kmf);
    island->buildTerrainModel(space,"terrain/island.bmp");

    islands.push_back(thermopilae);
    islands.push_back(nonsquareisland);
    islands.push_back(vulcano);
    islands.push_back(nemesis);
    islands.push_back(atom);
    islands.push_back(island);
}


void test1()
{
    // Entities will be added later in time.
    Balaenidae *_b = new Balaenidae(GREEN_FACTION);
    _b->init();
    _b->embody(world,space);
    _b->setPos(0.0f,20.5f,-4000.0f);
    _b->stop();

    entities.push_back(_b, _b->getGeom());
}

void test2()
{
    SimplifiedDynamicManta *_manta1 = new AdvancedManta(GREEN_FACTION);

    _manta1->init();
    _manta1->embody(world, space);
    _manta1->setPos(0.0f,20.5f,-6000.0f);
    _manta1->setStatus(0);
    _manta1->inert = false;
    _manta1->setStatus(FlyingStatus::FLYING);
    _manta1->elevator = +12;
    struct controlregister c;
    c.thrust = 1000.0f/(10.0);
    c.pitch = 12;
    _manta1->setControlRegisters(c);
    _manta1->setThrottle(1000.0f);

    entities.push_back(_manta1, _manta1->getGeom());
}

void checktest2(unsigned long timer)
{
    if (timer==320)
    {
        SimplifiedDynamicManta *_manta1 = (SimplifiedDynamicManta*)entities[2];
        _manta1->elevator = -4;
        struct controlregister c;
        c.thrust = 0.0f/(10.0);
        c.pitch = -4;
        _manta1->setControlRegisters(c);
        _manta1->setThrottle(0.0f);
        _manta1->inert = true;
    }
    if (timer==1600)
    {
        Vehicle *_b = entities[2];
        Vec3f val = _b->getPos();

        dReal *v = (dReal *)dBodyGetLinearVel(_b->getBodyID());
        Vec3f vec3fV;
        vec3fV[0]= v[0];vec3fV[1] = v[1]; vec3fV[2] = v[2];


        if (val[1]<4.0f)
        {
            printf("Test failed: Height bellow expected.\n");
            endWorldModelling();
            exit(-1);
        } else if (vec3fV.magnitude()>5)
        {
            printf("Test failed: Object is moving.\n");
            endWorldModelling();
            exit(-1);

        } else {
            printf("Test passed OK!\n");
            endWorldModelling();
            exit(1);
        }
    }
}




void test6()
{
    Vec3f pos(0.0f,10.0f,-4400.0f);
    Camera.setPos(pos);

    Balaenidae *b = (Balaenidae*)entities[1];

    b->setPos(0.0f,20.5f,-3000.0f);

}

void test7()
{
    Vec3f pos(0.0f,10.0f,-3700.0f);
    Camera.setPos(pos);
}

void test8()
{
    Walrus *_walrus = new Walrus(GREEN_FACTION);

    _walrus->init();
    _walrus->embody(world, space);
    _walrus->setPos(200.0f,1.32f,-6000.0f);
    _walrus->setStatus(SailingStatus::SAILING);
    _walrus->enableAuto();
    struct controlregister c;
    c.thrust = 200.0f;
    c.roll = 0;
    _walrus->setControlRegisters(c);
    _walrus->setThrottle(200.0f);

    entities.push_back(_walrus, _walrus->getGeom());
}

void checktest8(unsigned long  timer)      // Check Walrus entering and leaving an island.
{
    static bool isWalrusInIsland = false;
    static bool didWalrusLeftIsland = false;

    if (timer>100)
    {
        Walrus *_walrus = (Walrus*)entities[2];

        if (_walrus->getIsland() != NULL)
        {
            isWalrusInIsland = true;
            _walrus->stop();
            struct controlregister c;
            c.thrust = -200.0f;
            c.roll = 0;
            _walrus->setControlRegisters(c);
            _walrus->setThrottle(-200.0f);
        }

        if (_walrus->getStatus() == SailingStatus::OFFSHORING)
        {
            didWalrusLeftIsland = true;
        }
    }

    if (timer>=4000)
    {
        Walrus *_walrus = (Walrus*)entities[2];

        if (!isWalrusInIsland)
        {
            printf("Test failed: Walrus has not associated island.\n");
            endWorldModelling();
            exit(-1);
        } else if (!didWalrusLeftIsland) {
            printf("Test failed: Walrus never left the island.\n");
            endWorldModelling();
            exit(-1);
        } else {
            printf("Test succedded\n");
            endWorldModelling();
            exit(1);
        }
    }

}









void test15()
{
    // Entities will be added later in time.
    Balaenidae *_b = new Balaenidae(GREEN_FACTION);
    _b->init();
    _b->embody(world,space);
    _b->setPos(0.0f,20.5f,-9000.0f);
    _b->stop();

    entities.push_back(_b, _b->getGeom());
}

void checktest15(unsigned long timer)
{
    static bool reached = false;


    if (timer == 100)
    {
        size_t idx = 0;
        spawnManta(space,world,entities[1],idx);
    }

    if (timer == 320)
    {
        // launch
        launchManta(entities[1]);
    }



    if (timer == 420)
    {
        Vehicle *_b = findManta(GREEN_FACTION,FlyingStatus::FLYING);
        SimplifiedDynamicManta *_manta1 = (SimplifiedDynamicManta*)_b;
        _manta1->inert = false;
        _manta1->enableAuto();
        _manta1->setStatus(FlyingStatus::FLYING);
        _manta1->elevator = +5;
        struct controlregister c;
        c.thrust = 400.0f/(10.0);
        c.pitch = 5;
        _manta1->setControlRegisters(c);
        _manta1->setThrottle(400.0f);
        _manta1->disableAuto();
    }




    if (timer == 650)
    {
        SimplifiedDynamicManta *_manta1 = (SimplifiedDynamicManta*)findManta(GREEN_FACTION,FlyingStatus::FLYING);

        Balaenidae *_b = (Balaenidae*)findCarrier(GREEN_FACTION);

        _manta1->goTo(_b->getPos()-_b->getForward().normalize()*(10 kmf));
        _manta1->setAttitude(_b->getForward());
        _manta1->enableAuto();
    }

    if (timer > 650)
    {
        SimplifiedDynamicManta *_manta1 = (SimplifiedDynamicManta*)findManta(GREEN_FACTION,FlyingStatus::FLYING);

        Balaenidae *_b = (Balaenidae*)findCarrier(GREEN_FACTION);

        if (_manta1)
           _manta1->setAttitude( _b->getForward());

    }


    if (timer > 700)
    {
        // Auto control
        SimplifiedDynamicManta *_manta1 = (SimplifiedDynamicManta*)findManta(GREEN_FACTION,FlyingStatus::HOLDING);

        Balaenidae *_b = (Balaenidae*)findCarrier(GREEN_FACTION);

        if (_manta1)
        {
            runonce {
                _b->stop();
                _manta1->land(_b->getPos(),_b->getForward() );
                _manta1->enableAuto();
            }
        }
    }


    if (timer>1000)
    {
        SimplifiedDynamicManta *_manta1 = (SimplifiedDynamicManta*)findManta(GREEN_FACTION,FlyingStatus::ON_DECK);

        if (_manta1)
        {
            printf("Test passed OK!\n");
            endWorldModelling();
            exit(1);
        }
    }

    if (timer>10000)
    {
        // Timeout
        printf("Test failed.\n");
        endWorldModelling();
        exit(0);
    }
}


void checktest1(unsigned long timer)
{
    if (timer>500)
    {
        Vehicle *_b = entities[1];
        Vec3f val = _b->getPos()-Vec3f(0.0f,20.5f,-4000.0f);

        dReal *v = (dReal *)dBodyGetLinearVel(_b->getBodyID());
        Vec3f vec3fV;
        vec3fV[0]= v[0];vec3fV[1] = v[1]; vec3fV[2] = v[2];


        dReal *av = (dReal *)dBodyGetAngularVel(_b->getBodyID());
        Vec3f vav;
        vav[0]= av[0];vav[1] = av[1]; vav[2] = av[2];

        if (val.magnitude()>100)
        {
            printf("Test failed.\n");
            endWorldModelling();
            exit(-1);
        } else if (vav.magnitude()>10)
        {
            printf("Test failed.\n");
            endWorldModelling();
            exit(-1);
        } else if (vec3fV.magnitude()>5)
        {
            printf("Test failed.\n");
            endWorldModelling();
            exit(-1);

        } else {
            printf("Test passed OK!\n");
            endWorldModelling();
            exit(1);
        }
    }
}



void test3()
{
    islands[0]->addStructure(new Structure()  ,           0.0f,-1000.0f,0,world);
}


void checktest3(unsigned long timer)
{

    if (timer==90)  // Slow down Manta so that it can hit the structure.
    {
        SimplifiedDynamicManta *_manta1 = (SimplifiedDynamicManta*)entities[2];
        _manta1->elevator = -4;
        struct controlregister c;
        c.thrust = 400.0f/(10.0);
        c.pitch = -4;
        _manta1->setControlRegisters(c);
        _manta1->setThrottle(400.0f);
    }
    if (timer==1000)   // In case it hit the structure, their health will be lower.
    {
        SimplifiedDynamicManta *_manta1 = (SimplifiedDynamicManta*)entities[2];

        if (_manta1->getHealth()==1000)
        {
            printf("Test failed.\n");
            endWorldModelling();
            exit(-1);
        } else {
            printf("Test succedded\n");
            endWorldModelling();
            exit(1);
        }
    }
}


void test4()
{
    islands[0]->addStructure(new Runway(GREEN_FACTION)     ,           0.0f,    0.0f,0,world);
    islands[0]->addStructure(new Hangar(GREEN_FACTION)     ,        -550.0f,    0.0f,0,world);
}

void checktest4(unsigned long  timer)
{
    if (timer==100)   // Slow down Manta so that it can land on the runway
    {
        SimplifiedDynamicManta *_manta1 = (SimplifiedDynamicManta*)entities[2];
        _manta1->elevator = -4;
        struct controlregister c;
        c.thrust = 400.0f/(10.0);
        c.pitch = -4;
        _manta1->setControlRegisters(c);
        _manta1->setThrottle(400.0f);
        _manta1->setStatus(FlyingStatus::FLYING);
    }
    if (timer==1000)
    {
        SimplifiedDynamicManta *_manta1 = (SimplifiedDynamicManta*)entities[2];

        if (_manta1->getStatus()!=FlyingStatus::LANDED)
        {
            printf("Test failed.\n");
            endWorldModelling();
            exit(-1);
        } else {
            printf("Test succedded\n");
            endWorldModelling();
            exit(1);
        }
    }
}


void checktest5(unsigned long  timer)   // Manta lands on carrier.
{
    if (timer==70)
    {
        SimplifiedDynamicManta *_manta1 = (SimplifiedDynamicManta*)entities[2];
        _manta1->elevator = -4;
        struct controlregister c;
        c.thrust = 400.0f/(10.0);
        c.pitch = -4;
        _manta1->setControlRegisters(c);
        _manta1->setThrottle(400.0f);
        _manta1->setStatus(FlyingStatus::FLYING);
    }
    if (timer==240)
    {
        SimplifiedDynamicManta *_manta1 = (SimplifiedDynamicManta*)entities[2];
        _manta1->elevator = -4;
        struct controlregister c;
        c.thrust = 0.0f/(10.0);
        c.pitch = 0;
        _manta1->setControlRegisters(c);
        _manta1->setThrottle(0.0f);
        _manta1->stop();
        _manta1->setStatus(FlyingStatus::FLYING);
    }
    if (timer==1000)
    {
        SimplifiedDynamicManta *_manta1 = (SimplifiedDynamicManta*)entities[2];

        if (_manta1->getStatus()!=FlyingStatus::ON_DECK)
        {
            printf("Test failed.\n");
            endWorldModelling();
            exit(-1);
        } else {
            printf("Test succedded\n");
            endWorldModelling();
            exit(1);
        }
    }

}

void checktest6(unsigned long timer)   // Carrier offshoring.
{
    static bool isOffshoring = false;

    if (timer==100)  // This is not autopilot.  If you control de carrier, you will override the controlregister parameters.
    {
        Balaenidae *b = (Balaenidae*)entities[1];
        struct controlregister c;
        memset(&c,0,sizeof(struct controlregister));
        c.thrust = 10000.0f;
        c.pitch = 0;
        c.roll = 10;
        b->stop();
        b->setControlRegisters(c);
        b->setThrottle(1000.0f);
    }
    if (timer >100 )
    {
        Balaenidae *b = (Balaenidae*)entities[1];

        if (b->getStatus()==Balaenidae::OFFSHORING)
        {
            isOffshoring = true;
        }
    }
    if (timer==3800)
    {
        Balaenidae *b = (Balaenidae*)entities[1];

        Vehicle *_b = entities[1];
        Vec3f val = _b->getPos();

        dReal *v = (dReal *)dBodyGetLinearVel(_b->getBodyID());
        Vec3f vec3fV;
        vec3fV[0]= v[0];vec3fV[1] = v[1]; vec3fV[2] = v[2];


        dReal *av = (dReal *)dBodyGetLinearVel(_b->getBodyID());
        Vec3f vav;
        vav[0]= av[0];vav[1] = av[1]; vav[2] = av[2];

        if (!isOffshoring){
            printf("Test failed: Carrier never offshored.\n");
            endWorldModelling();
            exit(-1);
        }
        if (b->getStatus() != Balaenidae::SAILING || vav.magnitude()>100)
        {
            printf("Test failed: Carrier is still moving.\n");
            endWorldModelling();
            exit(-1);
        } else {
            printf("Test passed OK!\n");
            endWorldModelling();
            exit(1);
        }
    }
}


void checktest7(unsigned long  timer)    // Manta crashing on water (reducing its health).
{

    if (timer>380)
    {
        SimplifiedDynamicManta *_manta1 = (SimplifiedDynamicManta*)findManta(GREEN_FACTION,FlyingStatus::FLYING);

        if (_manta1)
        {
            _manta1->elevator -= 29;
            struct controlregister c;
            c.thrust = 3100.0f/(10.0);
            c.pitch = -35;
            _manta1->setControlRegisters(c);
            _manta1->setThrottle(310.0f);
            _manta1->setStatus(FlyingStatus::FLYING);
            _manta1->doControl(c);
        }

    }
    if (timer==900)
    {
        SimplifiedDynamicManta *_manta1 = (SimplifiedDynamicManta*)findManta(GREEN_FACTION,FlyingStatus::FLYING);

        if (!_manta1)
        {
            printf("Test succedded\n");
            endWorldModelling();
            exit(1);
        }

        // @FIXME Health reduction should be a parameter and this test should minimize it to make it work.
        // Manta should be immediately destroyed at this speed.
        if (_manta1->getHealth()==1000)
        {
            printf("Test failed.\n");
            endWorldModelling();
            exit(-1);
        } else {
            printf("Test succedded\n");
            endWorldModelling();
            exit(1);
        }
    }

}

void test9()
{
    AdvancedWalrus *_walrus = new AdvancedWalrus(GREEN_FACTION);
    _walrus->init();
    _walrus->embody(world, space);
    _walrus->setPos(200.0f,1.32f,-6000.0f);
    _walrus->setStatus(SailingStatus::SAILING);
    _walrus->stop();

    entities.push_back(_walrus, _walrus->getGeom());

    Vec3f pos(200.0,1.32,-6000 - 60);
    Camera.setPos(pos);
}

void checktest9(unsigned long timer)     // Check walrus stability.
{
    if (timer>500)
    {
        Vehicle *_b = entities[2];
        Vec3f posVector = _b->getPos()-Vec3f(200.0f,1.32f,-6000.0f);

        dReal *v = (dReal *)dBodyGetLinearVel(_b->getBodyID());
        Vec3f vec3fV;
        vec3fV[0]= v[0];vec3fV[1] = v[1]; vec3fV[2] = v[2];


        dReal *av = (dReal *)dBodyGetAngularVel(_b->getBodyID());
        Vec3f vav;
        vav[0]= av[0];vav[1] = av[1]; vav[2] = av[2];

        if (posVector.magnitude()>100)
        {
            printf("Test failed: Walrus is not in their expected position.\n");
            endWorldModelling();
            exit(-1);
        } else if (vav.magnitude()>10)
        {
            printf("Test failed: Walrus is still moving.\n");
            endWorldModelling();
            exit(-1);
        } else if (vec3fV.magnitude()>5)
        {
            printf("Test failed: Walrus is still circling.\n");
            endWorldModelling();
            exit(-1);

        } else {
            printf("Test passed OK!\n");
            endWorldModelling();
            exit(1);
        }
    }
}

void test10()
{
    AdvancedWalrus *_walrus = new AdvancedWalrus(GREEN_FACTION);
    _walrus->init();
    _walrus->embody(world, space);
    _walrus->setPos(200.0f,1.32f,-6000.0f);
    _walrus->setStatus(SailingStatus::SAILING);
    _walrus->setName("Intrepid");
    _walrus->stop();

    entities.push_back(_walrus, _walrus->getGeom());
}

void checktest10(unsigned long timer)     // Check Walrus arriving to an island and creating the command center.
{
    static unsigned long timerstep = 0;
    Walrus *_walrus = (Walrus*)entities[2];

    static int stateMachine = 0;

    if (timer == 100)
    {
        _walrus->enableAuto();
        struct controlregister c;
        c.thrust = 0.0f;
        c.roll = 20.0f;
        _walrus->setControlRegisters(c);
    }


    if (timer == 275)
    {
        _walrus->enableAuto();
        struct controlregister c;
        c.thrust = 0.0f;
        c.roll = 0.0f;
        _walrus->setControlRegisters(c);
        _walrus->stop();
    }

    if (timer == 300)
    {

        _walrus->enableAuto();
        struct controlregister c;
        c.thrust = 200.0f;
        c.roll = -1;
        _walrus->setControlRegisters(c);
        _walrus->setThrottle(200.0f);
    }

    if (stateMachine == 0 && _walrus->getStatus()==SailingStatus::ROLLING)
    {
        timerstep = timer;
        stateMachine = 1;
    }

    if (stateMachine == 1 && timerstep>0 && timer == (timerstep + 50))
    {
        struct controlregister c;
        c.thrust = 0.0f;
        c.roll = 0;
        _walrus->setControlRegisters(c);
        _walrus->setThrottle(0.0f);
        _walrus->stop();

        timerstep = timer;
        stateMachine = 2;
    }

    if (stateMachine == 2 && timerstep>0 && timer == (timerstep + 150))
    {
        BoxIsland *island = _walrus->getIsland();
        int x = (rand() % 2000 + 1); x -= 1000;
        int z = (rand() % 2000 + 1); z -= 1000;

        Structure *s = island->addStructure(new CommandCenter(GREEN_FACTION, DEFENSE_ISLAND),x,z,0,world);

        timerstep = timer;
        stateMachine = 3;
    }

    if (stateMachine == 3 && timerstep>0 && timer == (timerstep + 100))
    {
        CommandCenter *c = findCommandCenter(islands[0]);

        if (!c)
        {
            printf("Test failed: CommandCenter was not built on the island.\n");
            endWorldModelling();
            exit(-1);

        } else {
            printf("Test passed OK!\n");
            endWorldModelling();
            exit(1);
        }

    }


}

void test11()
{
    // Entities will be added later in time.
    Balaenidae *_b = new Balaenidae(GREEN_FACTION);
    _b->init();
    _b->embody(world,space);
    _b->setPos(-400 kmf,20.5f,-400 kmf);
    _b->stop();

    entities.push_back(_b, _b->getGeom());
}

void checktest11(unsigned long timer)     // Check Carrier stability.
{
    if (timer>1500)
    {
        Vehicle *_b = entities[1];
        Vec3f val = _b->getPos()-Vec3f(-400 kmf,20.5f,-400 kmf);

        dReal *v = (dReal *)dBodyGetLinearVel(_b->getBodyID());
        Vec3f vec3fV;
        vec3fV[0]= v[0];vec3fV[1] = v[1]; vec3fV[2] = v[2];


        dReal *av = (dReal *)dBodyGetAngularVel(_b->getBodyID());
        Vec3f vav;
        vav[0]= av[0];vav[1] = av[1]; vav[2] = av[2];

        if (val.magnitude()>100)
        {
            printf("Test failed.\n");
            endWorldModelling();
            exit(-1);
        } else if (vav.magnitude()>10)
        {
            printf("Test failed.\n");
            endWorldModelling();
            exit(-1);
        } else if (vec3fV.magnitude()>5)
        {
            printf("Test failed.\n");
            endWorldModelling();
            exit(-1);

        } else {
            printf("Test passed OK!\n");
            endWorldModelling();
            exit(1);
        }
    }
}

void test13()
{
    // Entities will be added later in time.
    Balaenidae *_b = new Balaenidae(GREEN_FACTION);
    _b->init();
    _b->embody(world,space);
    _b->setPos(0.0f,20.5f,+4000.0f);
    _b->stop();

    entities.push_back(_b, _b->getGeom());

    islands[0]->addStructure(new LaserTurret(GREEN_FACTION)     ,             0.0f,      0.0f,0,world);
}


void checktest13(unsigned long timer)    // Laser firing and hitting Carrier.
{
    if (timer==100)
    {
        LaserTurret *l=(LaserTurret*)entities[2];
        l->enableAuto();
        l->elevation = 0.3f; // Pushing down the turret.
        struct controlregister c;
        c.pitch = 0.0;
        l->setControlRegisters(c);
    }
    if (timer==500)
    {
        //dMatrix3 R;
        //ray = dCreateRay(space,4000.0f);
        //dGeomSetPosition(ray,1000.0f,20.5f,-4000.0f);   // 0 20 -4000
        //dRFromAxisAndAngle (R,0.0f,1.0f,0.0f,-90.0f);
        //dGeomSetRotation (ray,R);

        LaserTurret *l=(LaserTurret*)entities[2];
        Vehicle *action = (l)->fire(0,world,space);
        //int *idx = new int();
        //*idx = vehicles.push_back(action);
        //dBodySetData( action->getBodyID(), (void*)idx);
        if (action != NULL)
        {
            entities.push_back(action, action->getGeom());
            gunshot();
        }

    }

    if (timer == 700)
    {
        Vehicle *_b = entities[1];

        if (_b->getHealth() == 1000.0f)
        {
            printf("Test failed: The laser did nothing to the Carrier.\n");
            endWorldModelling();
            exit(-1);

        } else {
            printf("Test passed OK!\n");
            endWorldModelling();
            exit(1);
        }

    }

}

void test12()
{
    islands[0]->addStructure(new Turret(GREEN_FACTION)     ,         1550.0f,    0.0f,0,world);
    islands[0]->addStructure(new Turret(GREEN_FACTION)     ,        -1550.0f,    0.0f,0,world);

    Walrus *_walrus = new Walrus(GREEN_FACTION);
    _walrus->init();
    _walrus->embody(world, space);
    _walrus->setPos(200.0f,1.32f,-6000.0f);
    _walrus->setStatus(SailingStatus::SAILING);
    _walrus->stop();

    entities.push_back(_walrus, _walrus->getGeom());
}

void checktest12(unsigned long timer)
{
    if (timer==100)
    {
        Turret *l=(Turret*)entities[2];
        l->enableAuto();
        l->elevation = 0.12f; // Pushing down the turret.
        l->azimuth = 159.0f;   // Moving around the turret.
        struct controlregister c;
        c.pitch = 0.0;
        l->setControlRegisters(c);

        // Extra
        controller.controllingid = 0;
    }
    if (timer==500)
    {
        //dMatrix3 R;
        //ray = dCreateRay(space,4000.0f);
        //dGeomSetPosition(ray,1000.0f,20.5f,-4000.0f);   // 0 20 -4000
        //dRFromAxisAndAngle (R,0.0f,1.0f,0.0f,-90.0f);
        //dGeomSetRotation (ray,R);

        LaserTurret *l=(LaserTurret*)entities[2];
        Vehicle *action = (l)->fire(0,world,space);
        //int *idx = new int();
        //*idx = vehicles.push_back(action);
        //dBodySetData( action->getBodyID(), (void*)idx);
        if (action != NULL)
        {
            entities.push_back(action, action->getGeom());
            gunshot();
        }

    }

    if (timer == 700)
    {
        Vehicle *_b = entities[1];

        if (_b->getHealth() == 1000.0f)
        {
            printf("Test failed: The laser did nothing to the Carrier.\n");
            endWorldModelling();
            exit(-1);

        } else {
            printf("Test passed OK!\n");
            endWorldModelling();
            exit(1);
        }

    }
}

void test14()
{
    // Entities will be added later in time.
    Balaenidae *_b = new Balaenidae(GREEN_FACTION);
    _b->init();
    _b->embody(world,space);
    _b->setPos(0.0f,20.5f,-9000.0f);
    _b->stop();

    entities.push_back(_b, _b->getGeom());

    islands[5]->addStructure(new Runway(GREEN_FACTION)     ,           0.0f,    0.0f,0,world);
    islands[5]->addStructure(new Hangar(GREEN_FACTION)     ,           0.0f, +550.0f,0,world);
}

void checktest14(unsigned long timer)
{
    static bool reached = false;


    if (timer == 100)
    {
        size_t idx = 0;
        Manta *m = spawnManta(space,world,entities[1],idx);
        m->setSignal(4);
    }

    if (timer == 320)
    {
        // launch
        launchManta(entities[1]);
    }


    if (timer == 420)
    {
        Vehicle *_b = entities[4];
        SimplifiedDynamicManta *_manta1 = (SimplifiedDynamicManta*)_b;
        _manta1->inert = false;
        _manta1->setStatus(FlyingStatus::FLYING);
        _manta1->elevator = +5;
        struct controlregister c;
        c.thrust = 400.0f/(10.0);
        c.pitch = 5;
        _manta1->setControlRegisters(c);
        _manta1->setThrottle(400.0f);
        _manta1->doControl(c);
    }

    if (timer > 501)
    {
        // Auto control
        Vehicle *_b = entities[4];
        SimplifiedDynamicManta *_manta1 = (SimplifiedDynamicManta*)_b;

        Vec3f Po = _manta1->getPos();

        float height = Po[1];

        Po[1] = 0.0f;

        Vec3f Pf(-100 kmf, 0.0f, 100 kmf);

        Vec3f T = Pf - Po;

        float eh, midpointpitch;


        if (!reached && T.magnitude()>200)
        {
            float distance = T.magnitude();

            Vec3f F = _manta1->getForward();

            F = F.normalize();
            T = T.normalize();


            float e = _acos(  T.dot(F) );

            float signn = T.cross(F) [1];


            printf("T: %10.3f %10.3f %10.3f\n", distance, e, signn);


            struct controlregister c = _manta1->getControlRegisters();

            if (abs(e)>=0.5f)
            {
                c.roll = 3.0 * (signn>0?+1:-1) ;
            } else
            if (abs(e)>=0.4f)
            {
                c.roll = 2.0 * (signn>0?+1:-1) ;
            } else
            if (abs(e)>=0.2f)
                c.roll = 1.0 * (signn>0?+1:-1) ;
            else {
                c.roll = 0.0f;
            }

            eh = height-500.0f;
            midpointpitch = -15;
            c.thrust = 150.0f;

            if (distance<10000.0f)
            {
                c.thrust = 30.0f;
                midpointpitch = 17;
            }


            if ((abs(eh))>10.0f)
            {
                c.pitch = midpointpitch+1.0 * (eh>0 ? -1 : +1);
            } else {
                c.pitch = midpointpitch;
            }

            _manta1->setControlRegisters(c);
            _manta1->doControl(c);
        } else
        {
            printf("Manta arrived to destination...\n");
            SimplifiedDynamicManta *_manta1 = (SimplifiedDynamicManta*)entities[4];
            _manta1->elevator = 35;
            struct controlregister c;
            c.thrust = 300.0f/(10.0);

            midpointpitch = 36;
            eh=height-500.0f;
            static float diff = eh;

            c.roll = -13;

            if ((abs(eh))>10.0f)
            {
                c.pitch = midpointpitch+1.0 * (eh>0 ? -1 : +1);
            } else {
                c.pitch = midpointpitch;
            }

            _manta1->setThrottle(30.0f);
            _manta1->setControlRegisters(c);
            _manta1->doControl(c);

            //_manta1->stop();

            if (!reached)
            {
                char str[256];
                Message mg;
                mg.faction = _manta1->getFaction();
                sprintf(str, "Manta %d has arrived to destination.", _manta1->getNumber()+1);
                mg.msg = std::string(str);
                messages.insert(messages.begin(), mg);
                reached = true;
            }
        }



    }

    if (timer>10000)
    {
        if (reached)
        {
            printf("Test passed OK!\n");
            endWorldModelling();
            exit(1);
        }
        else
        {
            printf("Test failed.\n");
            endWorldModelling();
            exit(0);
        }
    }


}




void checktest16(unsigned long timer)
{
    if (timer==100)
    {
        Turret *l=(Turret*)entities[2];
        l->enableAuto();
        l->elevation = 0.23f; // Pushing down the turret.
        l->azimuth = 90.0f;   // Moving around the turret.
        struct controlregister c;
        c.pitch = 0.0;
        l->setControlRegisters(c);

        // Extra
        controller.controllingid = 2;
    }
    if (timer==500)
    {
        //dMatrix3 R;
        //ray = dCreateRay(space,4000.0f);
        //dGeomSetPosition(ray,1000.0f,20.5f,-4000.0f);   // 0 20 -4000
        //dRFromAxisAndAngle (R,0.0f,1.0f,0.0f,-90.0f);
        //dGeomSetRotation (ray,R);

        LaserTurret *l=(LaserTurret*)entities[2];
        Vehicle *action = (l)->fire(0,world,space);
        //int *idx = new int();
        //*idx = vehicles.push_back(action);
        //dBodySetData( action->getBodyID(), (void*)idx);
        if (action != NULL)
        {
            entities.push_back(action, action->getGeom());
            gunshot();
        }

    }

    if (timer == 700)
    {
        LaserTurret *_b = (LaserTurret*)entities[3];

        if (_b->getHealth() == 1000.0f)
        {
            printf("Test failed: The laser turret on the island has not been hit.\n");
            endWorldModelling();
            exit(-1);

        } else {
            printf("Test passed OK!\n");
            endWorldModelling();
            exit(1);
        }

    }
}

void checktest17(unsigned long timer)
{
    Turret *l=(Turret*)entities[2];

    Vehicle *_b = entities[1];


    // Find the vector between them, and the parameters for the turret to hit the carrier, regardless of its random position.

    Vec3f pos=_b->getPos();
    Vec3f tur = l->getPos();

    Vec3f aim = pos-tur;

    aim = aim.normalize();


    float azimuth = atan2(aim[2], aim[0])*180.0/PI;

    if (azimuth>=90) azimuth -= 90;
    else azimuth += 270;


    float incl = atan2(aim[1], aim[0]) * 180.0/PI;

    if (azimuth < 180.0f)
        incl += 180.0;

    printf ("Incl:%10.5f    Bg: %10.5f\n", incl, azimuth);

    if (timer==500)   // You should wait until the carrier stops moving.  This is a very interesting unexpected (but realistic) result.
    {
        // @NOTE The real test should be performed controlling the parameters to control the turret instead of the internal parameters of the turret.
        // I mean, this is not a real turret that should have mass and inertia.  This is straightforward aiming.
        l->enableAuto();
        l->elevation = incl;
        l->azimuth = azimuth;
        struct controlregister c;
        c.pitch = 0.0;
        l->setControlRegisters(c);

        // Extra
        controller.controllingid = 0;   // Shift screen to carrier.
    }
    if (timer==700)
    {
        //dMatrix3 R;
        //ray = dCreateRay(space,4000.0f);
        //dGeomSetPosition(ray,1000.0f,20.5f,-4000.0f);   // 0 20 -4000
        //dRFromAxisAndAngle (R,0.0f,1.0f,0.0f,-90.0f);
        //dGeomSetRotation (ray,R);

        Vehicle *action = (l)->fire(0,world,space);
        //int *idx = new int();
        //*idx = vehicles.push_back(action);
        //dBodySetData( action->getBodyID(), (void*)idx);
        if (action != NULL)
        {
            entities.push_back(action, action->getGeom());
            gunshot();
        }

    }

    if (timer == 900)
    {
        Vehicle *_b = entities[1];

        if (_b->getHealth() == 1000.0f)
        {
            printf("Test failed: Carrier has not been hit. Either aiming or fire was wrong..\n");
            endWorldModelling();
            exit(-1);

        } else {
            printf("Test passed OK!\n");
            endWorldModelling();
            exit(1);
        }

    }


}

void checktest18(unsigned long timer)
{
    Turret *l=(Turret*)entities[3];

    Vehicle *_b = entities[1];


    // Find the vector between them, and the parameters for the turret to hit the carrier, regardless of its random position.

    Vec3f pos=_b->getPos();
    Vec3f tur = l->getPos();

    Vec3f aim = pos-tur;

    aim = aim.normalize();


    float azimuth = atan2(aim[2], aim[0])*180.0/PI;

    if (azimuth>=90) azimuth -= 90;
    else azimuth += 270;


    float incl = atan2(aim[1], aim[0]) * 180.0/PI;

    if (azimuth < 180.0f)
        incl += 180.0;

    printf ("Incl:%10.5f    Bg: %10.5f\n", incl, azimuth);

    if (timer==500)   // You should wait until the carrier stops moving.  This is a very interesting unexpected (but realistic) result.
    {
        // @NOTE The real test should be performed controlling the parameters to control the turret instead of the internal parameters of the turret.
        // I mean, this is not a real turret that should have mass and inertia.  This is straightforward aiming.
        l->enableAuto();
        l->elevation = incl+10*PI/180.0;
        l->azimuth = azimuth;
        struct controlregister c;
        c.pitch = 0.0;
        l->setControlRegisters(c);

        // Extra
        controller.controllingid = 0;   // Shift screen to carrier.
    }
    if (timer==700)
    {
        //dMatrix3 R;
        //ray = dCreateRay(space,4000.0f);
        //dGeomSetPosition(ray,1000.0f,20.5f,-4000.0f);   // 0 20 -4000
        //dRFromAxisAndAngle (R,0.0f,1.0f,0.0f,-90.0f);
        //dGeomSetRotation (ray,R);

        Vehicle *action = (l)->fire(0,world,space);
        //int *idx = new int();
        //*idx = vehicles.push_back(action);
        //dBodySetData( action->getBodyID(), (void*)idx);
        if (action != NULL)
        {
            entities.push_back(action, action->getGeom());
            gunshot();
        }

    }

    if (timer == 900)
    {
        Vehicle *_b = entities[1];

        if (_b->getHealth() == 1000.0f)
        {
            printf("Test failed: Carrier has not been hit. Either aiming or fire was wrong..\n");
            endWorldModelling();
            exit(-1);

        } else {
            printf("Test passed OK!\n");
            endWorldModelling();
            exit(1);
        }

    }

}

void checktest19(unsigned long timer)
{
    Turret *l1=(Turret*)entities[2];
    Turret *l2=(Turret*)entities[3];

    Vehicle *b=NULL;
    if (entities.isValid(1))
        b = entities[1];
    else
    {
        printf("Test passed OK!\n");
        endWorldModelling();
        exit(1);
    }

    // Find the vector between them, and the parameters for the turret to hit the carrier, regardless of its random position.
    float azimuth1, inclination1;
    float azimuth2, inclination2;

    azimuth1 = getAzimuth((b->getPos())-(l1->getPos()));
    inclination1 = getDeclination((b->getPos())-(l1->getPos()));

    azimuth2 = getAzimuth((b->getPos())-(l2->getPos()));
    inclination2 = getDeclination((b->getPos())-(l2->getPos()));

    printf ("1:Incl:%10.5f    Bg: %10.5f\tIncl:%10.5f    Bg: %10.5f\n", inclination1, azimuth1, inclination2, azimuth2);

    if (timer>500 && (timer % 100 == 0))
    {

        l1->enableAuto();
        l1->elevation = inclination1;
        l1->azimuth = azimuth1;
        struct controlregister c;
        c.pitch = 0.0;
        l1->setControlRegisters(c);

        Vehicle *action = (l1)->fire(0,world,space);

        if (action != NULL)
        {
            entities.push_back(action, action->getGeom());
            gunshot();
        }

    }


    if (timer>600 && (timer % 54 == 0))
    {
        l2->enableAuto();
        l2->elevation = inclination2;
        l2->azimuth = azimuth2;
        struct controlregister c;
        c.pitch = 0.0;
        l2->setControlRegisters(c);


        Vehicle *action = (l2)->fire(0,world,space);

        if (action != NULL)
        {
            entities.push_back(action, action->getGeom());
            gunshot();
        }
    }


    if (timer == 3000)
    {
        Vehicle *_b = entities[1];

        if (_b->getHealth() == 1000.0f)
        {
            printf("Test failed: Carrier has not been hit. Either aiming or fire was wrong..\n");
            endWorldModelling();
            exit(-1);

        } else {
            printf("Test passed OK!\n");
            endWorldModelling();
            exit(1);
        }

    }
}

void test20()
{
    BoxIsland *nemesis = new BoxIsland(&entities);
    nemesis->setName("Nemesis");
    nemesis->setLocation(-450 kmf, -1.0, 300 kmf);
    nemesis->buildTerrainModel(space,"terrain/nm.bmp");

    islands.push_back(nemesis);

    // Entities will be added later in time.
    Balaenidae *_b = new Balaenidae(GREEN_FACTION);
    _b->init();
    _b->embody(world,space);
    _b->setPos(-450 kmf, -1.0, 300 kmf - 3000.0f);
    _b->stop();

    entities.push_back(_b, _b->getGeom());

    islands[0]->addStructure(new Turret(GREEN_FACTION)     ,          550.0f,    0.0f,0,world);
    islands[0]->addStructure(new Turret(GREEN_FACTION)     ,         -550.0f,    0.0f,0,world);

}

void checktest20(unsigned long timer)
{
    Turret *l1=(Turret*)entities[2];
    Turret *l2=(Turret*)entities[3];

    Vehicle *b=NULL;
    if (entities.isValid(1))
        b = entities[1];
    else
    {
        printf("Test passed OK!\n");
        endWorldModelling();
        exit(1);
    }

    // Find the vector between them, and the parameters for the turret to hit the carrier, regardless of its random position.
    float azimuth1, inclination1;
    float azimuth2, inclination2;

    azimuth1 = getAzimuth((b->getPos())-(l1->getPos()));
    inclination1 = getDeclination((b->getPos())-(l1->getPos()));

    azimuth2 = getAzimuth((b->getPos())-(l2->getPos()));
    inclination2 = getDeclination((b->getPos())-(l2->getPos()));

    printf ("1:Incl:%10.5f    Bg: %10.5f\tIncl:%10.5f    Bg: %10.5f\n", inclination1, azimuth1, inclination2, azimuth2);

    if (timer>500 && (timer % 100 == 0))
    {

        l1->enableAuto();
        l1->elevation = inclination1;
        l1->azimuth = azimuth1;
        struct controlregister c;
        c.pitch = 0.0;
        l1->setControlRegisters(c);

        Vehicle *action = (l1)->fire(0,world,space);

        if (action != NULL)
        {
            entities.push_back(action, action->getGeom());
            gunshot();
        }

    }


    if (timer>600 && (timer % 54 == 0))
    {
        l2->enableAuto();
        l2->elevation = inclination2;
        l2->azimuth = azimuth2;
        struct controlregister c;
        c.pitch = 0.0;
        l2->setControlRegisters(c);


        Vehicle *action = (l2)->fire(0,world,space);

        if (action != NULL)
        {
            entities.push_back(action, action->getGeom());
            gunshot();
        }
    }


    if (timer == 3000)
    {
        Vehicle *_b = entities[1];

        if (_b->getHealth() == 1000.0f)
        {
            printf("Test failed: Carrier has not been hit. Either aiming or fire was wrong..\n");
            endWorldModelling();
            exit(-1);

        } else {
            printf("Test passed OK!\n");
            endWorldModelling();
            exit(1);
        }

    }
}

void test21()
{
    BoxIsland *nemesis = new BoxIsland(&entities);
    nemesis->setName("Thermopilae");
    nemesis->setLocation(-450 kmf, -1.0, 300 kmf);
    nemesis->buildTerrainModel(space,"terrain/thermopilae.bmp");

    islands.push_back(nemesis);

    Balaenidae *_b = new Balaenidae(GREEN_FACTION);
    _b->init();
    _b->embody(world,space);
    _b->setPos(-450 kmf, -1.0, 300 kmf - 3000.0f);
    _b->stop();

    entities.push_back(_b, _b->getGeom());

    Structure *t = islands[0]->addStructure(new Turret(BLUE_FACTION)     ,          550.0f,    -1600.0f,0,world);


    Walrus *_walrus = new Walrus(GREEN_FACTION);

    _walrus->init();
    _walrus->embody(world, space);
    _walrus->setPos(-450 kmf+500.0f, -1.0, 300 kmf - 3000.0f);
    _walrus->setStatus(SailingStatus::SAILING);

    entities.push_back(_walrus, _walrus->getGeom());

    _walrus->goTo(t->getPos()-Vec3f(0.0f,0.0f,-100.0f));
    _walrus->enableAuto();

}

void checktest21(unsigned long timer)
{
    if (timer == 2000)
    {
        Walrus *b = findWalrus(GREEN_FACTION);
        Structure *t = (Structure*) entities[islands[0]->getStructures()[0]]; // Risky

        Vec3f d (b->getPos()-t->getPos());
        if (d.magnitude()>600.0f)
        {
            printf("Test failed: Walrus couldn't reach destination.\n");
            endWorldModelling();
            exit(-1);

        } else {
            printf("Test passed OK!\n");
            endWorldModelling();
            exit(1);
        }

    }
}

void test22()
{
    BoxIsland *nemesis = new BoxIsland(&entities);
    nemesis->setName("Nemesis");
    nemesis->setLocation(-450 kmf, -1.0, 300 kmf);
    nemesis->buildTerrainModel(space,"terrain/nemesis.bmp");

    islands.push_back(nemesis);

    Balaenidae *_b = new Balaenidae(GREEN_FACTION);
    _b->init();
    _b->embody(world,space);
    _b->setPos(-450 kmf, -1.0, 300 kmf - 3000.0f);
    _b->stop();

    entities.push_back(_b, _b->getGeom());

    Structure *t = islands[0]->addStructure(new Turret(BLUE_FACTION)     ,          550.0f,    0.0f,0,world);


    Walrus *_walrus = new Walrus(GREEN_FACTION);

    _walrus->init();
    _walrus->embody(world, space);
    _walrus->setPos(-450 kmf+500.0f, -1.0, 300 kmf - 3000.0f);
    _walrus->setStatus(SailingStatus::SAILING);

    entities.push_back(_walrus, _walrus->getGeom());

    _walrus->goTo(t->getPos()-Vec3f(0.0f,0.0f,-100.0f));
    _walrus->enableAuto();
}

void checktest22(unsigned long timer)
{
    Turret *l2=(Turret*)entities[islands[0]->getStructures()[0]]; // Risky
    Walrus *b = findWalrus(GREEN_FACTION);

    if (!b)
    {
        printf("Test passed OK!\n");
        endWorldModelling();
        exit(1);
    }

    // Find the vector between them, and the parameters for the turret to hit the carrier, regardless of its random position.
    float azimuth2, inclination2;

    azimuth2 = getAzimuth((b->getPos())-(l2->getPos()));
    inclination2 = getDeclination((b->getPos())-(l2->getPos()));


    printf ("Incl:%10.5f    Bg: %10.5f\n", inclination2, azimuth2);

    if (timer>600 && (timer % 54 == 0))
    {
        Vec3f firingloc = l2->getFiringPort();
        l2->setForward((b->getPos())-(firingloc));


        Vehicle *action = (l2)->fire(0,world,space);

        if (action != NULL)
        {
            entities.push_back(action, action->getGeom());
            gunshot();
        }
    }
}

void test23()
{
    BoxIsland *nemesis = new BoxIsland(&entities);
    nemesis->setName("Nemesis");
    //nemesis->setLocation(-450 kmf, -1.0, 300 kmf);
    nemesis->setLocation(0.0f,-1.0,0.0f);
    nemesis->buildTerrainModel(space,"terrain/nemesis.bmp");

    islands.push_back(nemesis);

    Balaenidae *_b = new Balaenidae(GREEN_FACTION);
    _b->init();
    _b->embody(world,space);
    _b->setPos(nemesis->getPos()-Vec3f(0.0f,0.0f,3000.0f));
    _b->stop();

    entities.push_back(_b, _b->getGeom());

    Structure *t = islands[0]->addStructure(new Turret(BLUE_FACTION)     ,          550.0f,    0.0f,0,world);


    Walrus *_walrus = new Walrus(GREEN_FACTION);

    _walrus->init();
    _walrus->embody(world, space);
    _walrus->setPos(_b->getPos()+Vec3f(-500.0f,0.0f,0.0f));
    _walrus->setStatus(SailingStatus::SAILING);

    entities.push_back(_walrus, _walrus->getGeom());

    _walrus->goTo(t->getPos()-Vec3f(0.0f,0.0f,100.0f));
    _walrus->enableAuto();
}


void checktest23(unsigned long timer)
{
    Turret *l2=(Turret*)entities[islands[0]->getStructures()[0]]; // Risky
    Walrus *b = findWalrus(GREEN_FACTION);
    //BoxVehicle *b = (BoxVehicle*)entities[3];

    if (!b)
    {
        printf("Test passed OK!\n");
        endWorldModelling();
        exit(1);
    }

    // Find the vector between them, and the parameters for the turret to hit the carrier, regardless of its random position.
    float azimuth2, inclination2;

    Vec3f firingloc = l2->getFiringPort();

    inclination2 = getDeclination((b->getPos())-(firingloc));
    azimuth2 = getAzimuth((b->getPos())-(firingloc));
    l2->setForward((b->getPos())-(firingloc));

    //l2->inclination = inclination2;
    //l2->azimuth = azimuth2;
    //struct controlregister c;
    //c.pitch = 0.0;
    //l2->setControlRegisters(c);

    if (timer>600 && (timer % 17 == 0))
    {
        //l2->enableAuto();
        /**l2->inclination = inclination2;
        l2->azimuth = azimuth2;
        struct controlregister c;
        c.pitch = 0.0;
        l2->setControlRegisters(c);
        **/

        std::cout << "Azimuth: " << azimuth2 << " Inclination: " << inclination2 << std::endl;


        Vehicle *action = (l2)->fire(0,world,space);

        if (action != NULL)
        {
            entities.push_back(action, action->getGeom());
            gunshot();
        }
    }
}


void test24()
{
    BoxIsland *nemesis = new BoxIsland(&entities);
    nemesis->setName("Nemesis");
    nemesis->setLocation(0.0f,-1.0,0.0f);
    nemesis->buildTerrainModel(space,"terrain/nemesis.bmp");

    islands.push_back(nemesis);

    Balaenidae *_b = new Balaenidae(GREEN_FACTION);
    _b->init();
    _b->embody(world,space);
    _b->setPos(nemesis->getPos()-Vec3f(0.0f,0.0f,3000.0f));
    _b->stop();

    entities.push_back(_b, _b->getGeom());

    Structure *t = islands[0]->addStructure(new Turret(BLUE_FACTION)     ,         0.0f,    0.0f,0,world);


    BoxVehicle * _bo= new BoxVehicle();
    _bo->init();
    _bo->embody(world, space);
    _bo->setPos(_b->getPos()+Vec3f(-500.0f,0.0f,0.0f));
    _bo->setPos(_bo->getPos()[0],20.1765f, _bo->getPos()[2]);
    _bo->stop();

    entities.push_back(_bo, _bo->getGeom());

}

void checktest24(unsigned long timer)
{
    Turret *l2=(Turret*)entities[islands[0]->getStructures()[0]]; // Risky
    BoxVehicle *b = (BoxVehicle*)entities[3];

    static Vec3f *p = NULL;

    if (!b)
    {
        printf("Test Failed.\n");
        endWorldModelling();
        exit(0);
    }

    // Find the vector between them, and the parameters for the turret to hit the carrier, regardless of its random position.
    float azimuth, elevation;

    Vec3f firingloc = l2->getFiringPort();

    elevation = getDeclination((b->getPos())-(firingloc));
    azimuth = getAzimuth((b->getPos())-(firingloc));
    l2->setForward((b->getPos())-(firingloc));

    std::cout << "Azimuth: " << azimuth << " Inclination: " << elevation << std::endl;

    if (timer>100 && !p)
    {
        p = new Vec3f(b->getPos());
    }


    if (timer>600 && (timer % 17 == 0))
    {
        //l2->enableAuto();
        /**l2->inclination = inclination2;
        l2->azimuth = azimuth2;
        struct controlregister c;
        c.pitch = 0.0;
        l2->setControlRegisters(c);
        **/


        Vehicle *action = (l2)->fire(0,world,space);

        if (action != NULL)
        {
            entities.push_back(action, action->getGeom());
            gunshot();
        }
    }


    if (timer>1100)
    {
        Vec3f l=*p;
        // The vehicle got hit, hence it has moved.
        if (!(l.isEquals(b->getPos())))
        {
            printf("Test passed OK!\n");
            endWorldModelling();
            exit(1);
        }

    }
}

void test25()
{
    // Entities will be added later in time.
    Balaenidae *_b = new Balaenidae(GREEN_FACTION);
    _b->init();
    _b->embody(world,space);
    _b->setPos(0.0f,20.5f,-4000.0f);
    _b->stop();

    entities.push_back(_b, _b->getGeom());
}

void checktest25(unsigned long timer)
{
    Balaenidae *b = (Balaenidae*)entities[1];

    std::cout << entities.size() << "," <<  fps << "," << elapsedtime << std::endl;

    if (timer==100)  // This is not autopilot.  If you control the carrier, you will override the controlregister parameters (it wont move).
    {
        struct controlregister c;
        memset(&c,0,sizeof(struct controlregister));
        c.thrust = 10000.0f;
        c.pitch = 0;
        c.roll = 10;
        b->stop();
        b->setControlRegisters(c);
        b->setThrottle(1000.0f);
        b->enableAuto();
    }
    if (timer % 300 == 0)
    {
        Walrus* w = spawnWalrus(space,world,b);
        w->setSignal(4);
    }

    // @NOTE: Remember that Walruses now have wheels, so you need more entities.
    if (timer > 200 && entities.size()>19*5)
    {
        if (fps>20.0)
        {
            printf("Test passed OK!\n");
            endWorldModelling();
            exit(1);
        } else
        {
            printf("Test failed: FPS is too slow. \n");
            endWorldModelling();
            exit(0);
        }
    }

}

void test26()
{
    // Entities will be added later in time.
    Balaenidae *_b = new Balaenidae(GREEN_FACTION);
    _b->init();
    _b->embody(world,space);
    _b->setPos(0.0f,20.5f,-4000.0f);
    _b->stop();

    entities.push_back(_b, _b->getGeom());

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

void checktest26(unsigned long timer)
{
    static std::ofstream fpsfile;
    if (timer == 1)
    {
        fpsfile.open ("fps.dat");
    }

    fpsfile << entities.size() << "," <<  fps << "," << elapsedtime << std::endl;
    fpsfile.flush();

    if (timer == 200)
    {
        for (int j=0;j<islands.size();j++)
        {
            captureIsland(islands[j],GREEN_FACTION,DEFENSE_ISLAND,space,world);
        }
    }

    if (timer > 3500)
    {
        if (fps > 40)
        {
            fpsfile.close();
            printf("Test passed OK!\n");
            endWorldModelling();
            exit(1);
        } else {
            fpsfile.close();
            printf("Test failed: FPS is too slow. \n");
            endWorldModelling();
            exit(0);
        }
    }
}

void test27()
{
    // Entities will be added later in time.
    Balaenidae *_b = new Balaenidae(GREEN_FACTION);
    _b->init();
    _b->embody(world,space);
    _b->setPos(0.0f,20.5f,-4000.0f);
    _b->stop();

    entities.push_back(_b, _b->getGeom());

    Vec3f pos(0.0f,10.0f,-4400.0f);
    Camera.setPos(pos);

    controller.controllingid = CONTROLLING_NONE;
}

void checktest27(unsigned long timer)
{
    Balaenidae *b = (Balaenidae*)entities[1];

    if (timer==100)  // This is not autopilot.  If you control the carrier, you will override the controlregister parameters (it wont move).
    {
        struct controlregister c;
        memset(&c,0,sizeof(struct controlregister));
        c.thrust = 0.0f;
        c.pitch = 0;
        c.roll = -60;
        c.yaw = 0;
        b->setControlRegisters(c);
        b->setThrottle(0.0f);
        b->doControl(c);
        //b->enableAuto();
    }

    if (timer == 120)
    {
        struct controlregister c;
        memset(&c,0,sizeof(struct controlregister));
        c.thrust = 0.0f;
        c.pitch = 0;
        c.roll = 0;
        c.yaw = 0;
        b->setControlRegisters(c);
        b->setThrottle(0.0f);
        b->doControl(c);
        //b->enableAuto();
    }

    if (timer == 490)
    {
        b->stop();
    }
    if (timer == 500)
    {
        size_t idx = 0;
        spawnManta(space,world,b,idx);
    }

    if (timer == 800)
    {
        // launch
        launchManta(b);
    }


    if (timer == 1000)
    {
        Manta *m = (Manta*)findManta(b->getFaction(),FlyingStatus::FLYING);
        if ((b->getBearing()-(m->getBearing()))<5)
        {
            printf("Test passed OK!\n");
            endWorldModelling();
            exit(1);
        } else {
            printf("Test failed: Manta bearing do not match carrier bearing.\n");
            endWorldModelling();
            exit(0);
        }
    }


}

void test28()
{
    BoxIsland *nemesis = new BoxIsland(&entities);
    nemesis->setName("Nemesis");
    nemesis->setLocation(0.0f,-1.0,0.0f);
    nemesis->buildTerrainModel(space,"terrain/nemesis.bmp");

    islands.push_back(nemesis);

    Balaenidae *_b = new Balaenidae(GREEN_FACTION);
    _b->init();
    _b->embody(world,space);
    _b->setPos(nemesis->getPos()-Vec3f(0.0f,0.0f,3000.0f));
    _b->stop();

    entities.push_back(_b, _b->getGeom());

    Structure *t = islands[0]->addStructure(new Artillery(BLUE_FACTION)     ,         0.0f,    -650.0f,0,world);

    Vec3f pos(4000,60.0f,-1500);
    Camera.setPos(pos);

    Camera.dy = 0;
    Camera.dz = 0;
    Camera.xAngle = 90;
    Camera.yAngle = 0;

    controller.controllingid = CONTROLLING_NONE;


}

void checktest28(unsigned long timer)
{
    Turret *l2=(Turret*)entities[islands[0]->getStructures()[0]]; // Risky

    if (timer == 100)
    {

        l2->elevation = -5;
        l2->azimuth = 180;

        struct controlregister c;
        c.pitch = 0.0;
        c.roll = 0.0;
        l2->setControlRegisters(c);
        l2->setForward(toVectorInFixedSystem(0,0,1,l2->azimuth, -l2->elevation));

    }

    if (timer == 300)
    {
        Vehicle *action = (l2)->fire(0,world,space);

        if (action != NULL)
        {
            entities.push_back(action, action->getGeom());
        }
    }

    if (timer == 1200)
    {

        if (!entities.isValid(1) || entities[1]->getHealth()<1000)
        {
            printf("Test passed OK!\n");
            endWorldModelling();
            exit(1);
        } else {
            printf("Test failed: Artillery did not hit carrier.\n");
            endWorldModelling();
            exit(0);
        }

    }

}

void test29()
{
    BoxIsland *nemesis = new BoxIsland(&entities);
    nemesis->setName("Nemesis");
    nemesis->setLocation(0.0f,-1.0,0.0f);
    nemesis->buildTerrainModel(space,"terrain/nemesis.bmp");

    islands.push_back(nemesis);

    Balaenidae *_b = new Balaenidae(GREEN_FACTION);
    _b->init();
    _b->embody(world,space);
    _b->setPos(nemesis->getPos()-Vec3f(0.0f,0.0f,17000.0f));
    _b->stop();

    entities.push_back(_b, _b->getGeom());

    Structure *t1 = islands[0]->addStructure(new CommandCenter(BLUE_FACTION, DEFENSE_ISLAND)    ,       200.0f,    -100.0f,0,world);
    Structure *t2 = islands[0]->addStructure(new Turret(BLUE_FACTION)           ,         0.0f,    -650.0f,0,world);
    Structure *t3 = islands[0]->addStructure(new LaserTurret(BLUE_FACTION)      ,         0.0f,    650.0f,0,world);
    Structure *t4 = islands[0]->addStructure(new Artillery(BLUE_FACTION)        ,       100.0f,    -650.0f,0,world);
    Structure *t5 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)        ,        20.0f,    80.0f,0,world);
    Structure *t6 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)        ,         -60.0f,    -80.0f,0,world);
    Structure *t7 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)        ,         0.0f,    120.0f,0,world);
    Structure *t8 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)        ,         -230.0f,    230.0f,0,world);

    //t1->enableAuto();
    t2->enableAuto();
    t3->enableAuto();

    Vec3f pos(0.0f,60.0f,-71.0f);
    Camera.setPos(pos);
}

void checktest29(unsigned long timer)
{
    // Launch Manta, set Destination to island, identify where a target is located, align the forward direction with the target, open fire

    if (timer == 200)
    {
        size_t idx=0;
        Balaenidae *b = (Balaenidae*)findCarrier(GREEN_FACTION);
        Manta *m = spawnManta(space,world,b,idx);

    }

    if (timer == 300)
    {
        Balaenidae *b = (Balaenidae*)findCarrier(GREEN_FACTION);
        launchManta(b);
    }

    if (timer == 400)
    {
        Balaenidae *b = (Balaenidae*)findCarrier(GREEN_FACTION);
        BoxIsland *i = findNearestIsland(b->getPos());

        Manta *m = findManta(GREEN_FACTION,FlyingStatus::FLYING);

        //m->setDestination(Vec3f(i->getX(),0.0, i->getZ()));
        m->attack(Vec3f(200.0, 0.5, -100.0f));
        m->enableAuto();

        size_t pos;
        findMantaByFactionAndNumber(pos,GREEN_FACTION,1);
        std::cout << "-----------:" << m << " / " << m->getNumber() << std::endl;

        size_t id = entities.indexAt(pos);
        controller.controllingid = id;
    }


    if (timer == 450)
    {
        Balaenidae *b = (Balaenidae*)findCarrier(GREEN_FACTION);
        size_t idx=0;
        Manta *m = spawnManta(space,world,b,idx);

    }

    if (timer == 600)
    {
        Balaenidae *b = (Balaenidae*)findCarrier(GREEN_FACTION);
        launchManta(b);
    }

    if (timer == 700)
    {
        Balaenidae *b = (Balaenidae*)findCarrier(GREEN_FACTION);
        BoxIsland *i = findNearestIsland(b->getPos());

        size_t pos;
        Manta *m = findMantaByFactionAndNumber(pos,GREEN_FACTION,2);
        std::cout << "-----------:" << m << " / " << m->getNumber() << std::endl;

        //m->setDestination(Vec3f(i->getX(),0.0, i->getZ()));
        m->attack(Vec3f(200.0, 0.5, -100.0f));
        m->enableAuto();
    }


    if (timer == 800)
    {
        Balaenidae *b = (Balaenidae*)findCarrier(GREEN_FACTION);
        size_t idx=0;
        Manta *m = spawnManta(space,world,b,idx);

    }

    if (timer == 890)
    {
        Balaenidae *b = (Balaenidae*)findCarrier(GREEN_FACTION);
        launchManta(b);

    }

    if (timer == 940)
    {
        Balaenidae *b = (Balaenidae*)findCarrier(GREEN_FACTION);
        BoxIsland *i = findNearestIsland(b->getPos());

        size_t pos;
        Manta *m = findMantaByFactionAndNumber(pos,GREEN_FACTION,3);

        std::cout << "-----------:" << m << " / " << m->getNumber() << std::endl;

        //m->setDestination(Vec3f(i->getX(),0.0, i->getZ()));
        m->attack(Vec3f(200.0, 0.5, -100.0f));
        m->enableAuto();
    }


    if (timer == 2500)
    {
        Balaenidae *b = (Balaenidae*)findCarrier(GREEN_FACTION);
        BoxIsland *i = findNearestIsland(b->getPos());

        CommandCenter *c = findCommandCenter(i);

        if (!c || c->getHealth()<1000)
        {
            printf("Test passed OK!\n");
            endWorldModelling();
            exit(1);
        } else {
            printf("Test failed: CommandCenter is intact.\n");
            endWorldModelling();
            exit(0);
        }
    }




}


void test30()
{
    BoxIsland *nemesis = new BoxIsland(&entities);
    nemesis->setName("Nemesis");
    nemesis->setLocation(0.0f,-1.0,0.0f);
    nemesis->buildTerrainModel(space,"terrain/nemesis.bmp");

    islands.push_back(nemesis);

    Balaenidae *_b = new Balaenidae(GREEN_FACTION);
    _b->init();
    _b->embody(world,space);
    _b->setPos(nemesis->getPos()-Vec3f(0.0f,0.0f,3000.0f));
    _b->stop();

    entities.push_back(_b, _b->getGeom());

    Structure *t = islands[0]->addStructure(new LaserTurret(BLUE_FACTION)     ,         0.0f,    0.0f,0,world);

    BoxVehicle * _bo= new BoxVehicle();
    _bo->init();
    _bo->embody(world, space);
    _bo->setPos(_b->getPos()+Vec3f(-500.0f,0.0f,0.0f));
    _bo->setPos(_bo->getPos()[0],20.1765f, _bo->getPos()[2]);
    _bo->stop();

    entities.push_back(_bo, _bo->getGeom());

}

void checktest30(unsigned long timer)
{
    LaserTurret *l2=(LaserTurret*)entities[islands[0]->getStructures()[0]]; // Risky
    BoxVehicle *b = (BoxVehicle*)entities[3];

    static Vec3f *p = NULL;

    if (!b)
    {
        printf("Test Failed.\n");
        endWorldModelling();
        exit(0);
    }

    // Find the vector between them, and the parameters for the turret to hit the carrier, regardless of its random position.
    float azimuth, elevation;

    Vec3f firingloc = l2->getFiringPort();

    elevation = getDeclination((b->getPos())-(firingloc));
    azimuth = getAzimuth((b->getPos())-(firingloc));
    l2->setForward((b->getPos())-(firingloc));

    l2->elevation = elevation;
    l2->azimuth = azimuth;

    struct controlregister c;
    c.pitch = 0.0;
    c.roll = 0.0;
    l2->setControlRegisters(c);

    std::cout << "Azimuth: " << azimuth << " Inclination: " << elevation << std::endl;

    if (timer>100 && !p)
    {
        p = new Vec3f(b->getPos());
    }


    if (timer==600)
    {

        Vehicle *action = (l2)->fire(0,world,space);

        if (action != NULL)
        {
            entities.push_back(action, action->getGeom());
            gunshot();
        }
    }

    if (timer==1800)
    {

        Vehicle *action = (l2)->fire(0,world,space);

        if (action != NULL)
        {
            entities.push_back(action, action->getGeom());
            gunshot();
        }
    }


    if (timer>28000)
    {
        Vec3f l=*p;
        // The vehicle got hit, hence it has moved.
        if (!(l.isEquals(b->getPos())))
        {
            printf("Test passed OK!\n");
            endWorldModelling();
            exit(1);
        }

    }
}

void test31()
{
    // Entities will be added later in time.
    Balaenidae *_b = new Balaenidae(GREEN_FACTION);
    _b->init();
    _b->embody(world,space);
    _b->setPos(0.0f,20.5f,-4000.0f);
    _b->stop();

    entities.push_back(_b, _b->getGeom());

    BoxIsland *atom = new BoxIsland(&entities);
    atom->setName("Atom");
    atom->setLocation(0.0f,-1.0,0.0f);
    atom->buildTerrainModel(space,"terrain/nonsquareisland.bmp");

    islands.push_back(atom);
}

void checktest31(unsigned long timer)
{
    Walrus *_walrus = findWalrus(GREEN_FACTION);

    if (!_walrus)
    {
        printf("Test passed OK!\n");
        endWorldModelling();
        exit(1);
    }
    static int stateMachine = 0;

    if (timer == 100)
    {
        _walrus->enableAuto();
        struct controlregister c;
        memset(&c,0,sizeof(struct controlregister));
        c.thrust = 0.0f;
        c.roll = 8.0f;
        _walrus->setControlRegisters(c);
    }


    if (timer == 275)
    {
        _walrus->enableAuto();
        struct controlregister c;
        memset(&c,0,sizeof(struct controlregister));
        c.thrust = 0.0f;
        c.roll = 0.0f;
        _walrus->setControlRegisters(c);
        _walrus->stop();
    }

    if (timer > 300 && timer < 900)
    {

        _walrus->enableAuto();
        struct controlregister c;
        memset(&c,0,sizeof(struct controlregister));
        c.thrust = ((timer-300)/12.0)*10.0f;
        c.roll = 0;
        _walrus->setControlRegisters(c);
        _walrus->setThrottle(((timer-300)/12.0)*10.0f);
    }

    if (timer == 3000)
    {
        printf("Test failed.\n");
        endWorldModelling();
        exit(0);
    }
}

void test32()
{

}

void checktest32(unsigned long timer)
{
    // Launch Manta, set Destination to island, identify where a target is located, align the forward direction with the target, open fire

    Vec3f orientation(100.0f, 0.0, 100.0f);
    float sp=0;

    float az = getAzimuth(orientation);
    float arz = getAzimuthRadians(orientation);
    float quadrant = 0;
    float arq = getContinuosAzimuthRadians(orientation);

    std::cout << "Orientation:" << orientation << std::endl;
    std::cout << "az:" << az << std::endl;
    std::cout << "arz:" << arz << std::endl;
    std::cout << "qad:" << quadrant << std::endl;
    std::cout << "arq:" << arq << std::endl ;

    orientation = Vec3f(-100.0f, 0.0, 100.0f);

    az = getAzimuth(orientation);
    arz = getAzimuthRadians(orientation);
    quadrant = 0;
    arq = getContinuosAzimuthRadians(orientation);

    std::cout << "Orientation:" << orientation << std::endl;
    std::cout << "az:" << az << std::endl;
    std::cout << "arz:" << arz << std::endl;
    std::cout << "qad:" << quadrant << std::endl;
    std::cout << "arq:" << arq << std::endl ;


    orientation = Vec3f(-100.0f, 0.0, -100.0f);

    az = getAzimuth(orientation);
    arz = getAzimuthRadians(orientation);
    arq = getContinuosAzimuthRadians(orientation);

    std::cout << "Orientation:" << orientation << std::endl;
    std::cout << "az:" << az << std::endl;
    std::cout << "arz:" << arz << std::endl;
    std::cout << "qad:" << quadrant << std::endl;
    std::cout << "arq:" << arq << std::endl ;

    orientation = Vec3f(100.0f, 0.0, -100.0f);

    az = getAzimuth(orientation);
    arz = getAzimuthRadians(orientation);
    float arq2 = getContinuosAzimuthRadians(orientation);

    std::cout << "Orientation:" << orientation << std::endl;
    std::cout << "az:" << az << std::endl;
    std::cout << "arz:" << arz << std::endl;
    std::cout << "qad:" << quadrant << std::endl;
    std::cout << "arq:" << arq << std::endl ;



    orientation = Vec3f(-100.0f, 0.0, -100.0f);

    az = getAzimuth(orientation);
    arz = getAzimuthRadians(orientation);
    float arq3 = getContinuosAzimuthRadians(orientation);

    std::cout << "Orientation:" << orientation << std::endl;
    std::cout << "az:" << az << std::endl;
    std::cout << "arz:" << arz << std::endl;
    std::cout << "qad:" << quadrant << std::endl;
    std::cout << "arq:" << arq << std::endl ;
}

void test33()
{
    BoxIsland *nemesis = new BoxIsland(&entities);
    nemesis->setName("Nemesis");
    nemesis->setLocation(-10000.0f,-1.0,-4000.0f);
    nemesis->buildTerrainModel(space,"terrain/thermopilae.bmp");

    islands.push_back(nemesis);

    Balaenidae *_b = new Balaenidae(GREEN_FACTION);
    _b->init();
    _b->embody(world,space);
    _b->setPos(nemesis->getPos()-Vec3f(0.0f,0.0f,17000.0f));
    _b->setPos(Vec3f(0.0f,0.0f,17000.0f));
    _b->stop();

    entities.push_back(_b, _b->getGeom());

    Structure *t1 = islands[0]->addStructure(new CommandCenter(BLUE_FACTION, DEFENSE_ISLAND)    ,       200.0f,    -100.0f,0,world);
    Structure *t2 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)           ,         0.0f,    -650.0f,0,world);
    Structure *t3 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)      ,         0.0f,    650.0f,0,world);
    Structure *t4 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)        ,       100.0f,    -650.0f,0,world);
    Structure *t5 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)        ,        20.0f,    80.0f,0,world);
    Structure *t6 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)        ,         -60.0f,    -80.0f,0,world);
    Structure *t7 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)        ,         0.0f,    120.0f,0,world);
    Structure *t8 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)        ,         -230.0f,    230.0f,0,world);
    Structure *t9 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)        ,         -230.0f,    1230.0f,0,world);


    Vec3f pos(0.0f,60.0f,-71.0f);
    Camera.setPos(pos);
}


void checktest33(unsigned long timer)
{
    if (timer == 50)
    {
        Balaenidae *b = (Balaenidae*)findCarrier(GREEN_FACTION);
        size_t idx=0;
        Manta *m = spawnManta(space,world,b,idx);
    }

    if (timer == 100)
    {
        Balaenidae *b = (Balaenidae*)findCarrier(GREEN_FACTION);
        launchManta(b);
    }

    if (timer == 310)
    {
        Balaenidae *b = (Balaenidae*)findCarrier(GREEN_FACTION);
        BoxIsland *i = findNearestIsland(b->getPos());

        Manta *m = findManta(GREEN_FACTION,FlyingStatus::FLYING);

        m->goTo(Vec3f(i->getX(),1000.0, i->getZ()));
        m->enableAuto();

        //m->attack(Vec3f(200.0, 0.0, -100.0f));
        m->enableAuto();

        size_t pos;
        findMantaByFactionAndNumber(pos,GREEN_FACTION,1);

        controller.controllingid = pos;
    }

    if (timer > 1000)
    {
        Balaenidae *b = (Balaenidae*)findCarrier(GREEN_FACTION);
        BoxIsland *i = findNearestIsland(b->getPos());

        Manta *m = findManta(GREEN_FACTION,FlyingStatus::FLYING);

        if (m)
        {
            Vec3f pos = m->getPos();pos[1]=0;
            Vec3f center = i->getPos();center[1]=0;

            dout << (pos-center).magnitude() << std::endl;

            if ( (pos-center).magnitude() < 500 )
            {
                printf("Test passed OK!\n");
                endWorldModelling();
                exit(1);
            }
        }
    }

    if (timer == 9000)
    {
        printf("Test failed. Manta should travel from the carrier to the center of the enemy island.\n");
        endWorldModelling();
        exit(0);
    }

}

void test34()
{
    // Entities will be added later in time.
    Balaenidae *_b = new Balaenidae(GREEN_FACTION);
    _b->init();
    _b->embody(world,space);
    _b->setPos(0.0f,20.5f,-4000.0f);
    _b->stop();

    entities.push_back(_b, _b->getGeom());

    AdvancedWalrus *_walrus = new AdvancedWalrus(GREEN_FACTION);
    _walrus->init();
    _walrus->embody(world, space);
    _walrus->setPos(200.0f,1.32f,-6000.0f);
    _walrus->setStatus(SailingStatus::SAILING);
    _walrus->stop();

    entities.push_back(_walrus, _walrus->getGeom());

    Vec3f pos(200.0,1.32,-6000 - 60);
    Camera.setPos(pos);
}

void checktest34(unsigned long timer)
{
    // Free test...

    if (timer==2000)
    {
        printf("Test passed OK!\n");
        endWorldModelling();
        exit(1);
    }

}


void test35()
{
    BoxIsland *nemesis = new BoxIsland(&entities);
    nemesis->setName("Nemesis");
    nemesis->setLocation(-10000.0f,-1.0,-4000.0f);
    nemesis->buildTerrainModel(space,"terrain/thermopilae.bmp");

    islands.push_back(nemesis);

    Structure *t1 = islands[0]->addStructure(new CommandCenter(BLUE_FACTION, DEFENSE_ISLAND)    ,       200.0f,    -100.0f,0,world);
    Structure *t2 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)           ,         0.0f,    -650.0f,0,world);
    Structure *t3 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)      ,         0.0f,    650.0f,0,world);
    Structure *t4 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)        ,       100.0f,    -650.0f,0,world);
    Structure *t5 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)        ,        20.0f,    80.0f,0,world);
    Structure *t6 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)        ,         -60.0f,    -80.0f,0,world);
    Structure *t7 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)        ,         0.0f,    120.0f,0,world);
    Structure *t8 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)        ,         -230.0f,    230.0f,0,world);



    // Entities will be added later in time.
    Balaenidae *_b = new Balaenidae(GREEN_FACTION);
    _b->init();
    _b->embody(world,space);
    _b->setPos(0.0f,20.5f,-6000.0f);
    _b->stop();

    entities.push_back(_b, _b->getGeom());

    Beluga *_bg = new Beluga(BLUE_FACTION);
    _bg->init();
    _bg->embody(world,space);
    _bg->setPos(-4000.0f,20.5f,11000.0f);
    //_bg->setPos(0.0f + 0.0 kmf,20.5f,-6000.0f + 0.0 kmf);
    _bg->stop();

    entities.push_back(_bg, _bg->getGeom());

    AdvancedWalrus *_walrus = new AdvancedWalrus(GREEN_FACTION);
    _walrus->init();
    _walrus->embody(world, space);
    _walrus->setPos(200.0f,1.32f,-6000.0f);
    _walrus->setStatus(SailingStatus::SAILING);
    _walrus->stop();

    entities.push_back(_walrus, _walrus->getGeom());

    Vec3f pos(0.0,1.32, - 60);
    Camera.setPos(pos);
}

void checktest35(unsigned long timer)
{
    // Free test...

    if (timer == 200)
    {
        Balaenidae *b = (Balaenidae *)findCarrier(GREEN_FACTION);

        Beluga *bg = (Beluga*) findCarrier(BLUE_FACTION);

        Missile *a = (Missile*) b->fire(0,world, space);

        size_t i = CONTROLLING_NONE;
        if (a)
            i = entities.push_back(a, a->getGeom());



        Vec3f ff = Vec3f(-10000.0f,-1.0,-4000.0f) + Vec3f(+200,0,-100);


        a->goTo(ff);
        a->enableAuto();

        if (a->getType()==CONTROLABLEACTION)
        {
            switchControl(i);

        }


    }

    if (timer == 1600)
    {
        Balaenidae *b = (Balaenidae *)findCarrier(GREEN_FACTION);

        Beluga *bg = (Beluga*) findCarrier(BLUE_FACTION);

        Missile *a = (Missile*) b->fire(0,world, space);
        size_t i = CONTROLLING_NONE;
        if (a)
            i = entities.push_back(a, a->getGeom());


        a->goTo(bg->getPos());
        a->enableAuto();

        if (a->getType()==CONTROLABLEACTION)
        {
            switchControl(i);

        }
    }

    if (timer==3000)
    {
        BoxIsland *nem = findIslandByName(std::string("Nemesis"));

        if (!(nem->getCommandCenter()))
        {
            Beluga *bg = (Beluga*) findCarrier(BLUE_FACTION);
            if (!bg)
            {
                printf("Test passed OK!\n");
                endWorldModelling();
                exit(1);
            }
            else
            {
                printf("Test failed:  Carries was not destroyed.\n");
                endWorldModelling();
                exit(0);
            }
        }
        else
        {
            printf("Test failed: Island's command center was not destroyed.\n");
            endWorldModelling();
            exit(0);
        }

    }

}

void test36()
{
    // Entities will be added later in time.
    Balaenidae *_b = new Balaenidae(GREEN_FACTION);
    _b->init();
    _b->embody(world,space);
    _b->setPos(0.0f,20.5f,-17000.0f);
    _b->stop();

    entities.push_back(_b, _b->getGeom());

    BoxIsland *nemesis = new BoxIsland(&entities);
    nemesis->setName("Nemesis");
    nemesis->setLocation(0.0f,-1.0,-0.0f);
    nemesis->buildTerrainModel(space,"terrain/thermopilae.bmp");

    islands.push_back(nemesis);

    Structure *t8 = islands[0]->addStructure(new Launcher(BLUE_FACTION)        ,         -230.0f,    230.0f,0,world);
    Structure *t9 = islands[0]->addStructure(new Turret(BLUE_FACTION)        ,           -330.0f,    230.0f,0,world);





    Vec3f pos(-230,1.32, 0);
    Camera.setPos(pos);
}

void checktest36(unsigned long timer)
{
    static Vehicle *action = NULL;

    if (timer == 200)
    {
        BoxIsland *island = findIslandByName(std::string("Nemesis"));
        Launcher *lb = (Launcher*) entities[islands[0]->getStructures()[0]];

        Vehicle *target = findNearestEnemyVehicle(BLUE_FACTION,island->getPos(), 9 * 3.6 kmf);


        if (!target)
            return;

        printf("Found target %p\n",  target);

        Vehicle *b = target;

        Vec3f firingloc = lb->getPos();

        std::cout << lb <<  ":Loc: " << firingloc << " Target: " << b->getPos() << std::endl;

        lb->elevation = -5; // A little bit up.
        lb->azimuth = getAzimuth((b->getPos())-(firingloc));

        struct controlregister c;
        c.pitch = 0.0;
        c.roll = 0.0;
        //lb->setControlRegisters(c);
        lb->setForward(toVectorInFixedSystem(0,0,1,lb->azimuth, -lb->elevation));

        std::cout << lb <<  ":Azimuth: " << lb->azimuth << " Inclination: " << lb->elevation << std::endl;

        action = (lb)->fireGround(world,space);

        if (action != NULL)
        {
            size_t i = entities.push_back(action, action->getGeom());
            //gunshot();

            //action->setDestination(b->getPos());

            //action->enableAuto();

            if (action->getType()==CONTROLABLEACTION)
            {
                switchControl(i);

            }
        }
    }

    if (timer > 240)
    {
        if (action)
        {
            BoxIsland *island = findIslandByName(std::string("Nemesis"));
            Vehicle *target = findNearestEnemyVehicle(BLUE_FACTION,island->getPos(), 9 * 3.6 kmf);


            if (target)
            {

                std::cout << target <<  ":Loc: " << action->getPos() << " Target: " << target->getPos() << std::endl;

                action->goTo(target->getPos());
                action->enableAuto();
            }
        }
    }

    if (timer == 1200)
    {
        Beluga *bg = (Beluga*) findCarrier(GREEN_FACTION);
        if (!bg)
        {
            printf("Test passed OK!\n");
            endWorldModelling();
            exit(1);
        }
        else
        {
            printf("Test failed: Carrier has not been destroyed.\n");
            endWorldModelling();
            exit(0);
        }
    }

}


void test37()
{
    // Entities will be added later in time.
    Balaenidae *_b = new Balaenidae(GREEN_FACTION);
    _b->init();
    _b->embody(world,space);
    _b->setPos(0.0f,20.5f,-3000.0f);
    _b->stop();

    entities.push_back(_b, _b->getGeom());

    BoxIsland *nemesis = new BoxIsland(&entities);
    nemesis->setName("Nemesis");
    nemesis->setLocation(0.0f,-1.0,-0.0f);
    nemesis->buildTerrainModel(space,"terrain/thermopilae.bmp");

    islands.push_back(nemesis);

    Structure *t8 = islands[0]->addStructure(new Launcher(BLUE_FACTION)        ,         -230.0f,    230.0f,0,world);
    Structure *t9 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)        ,           -330.0f,    230.0f,0,world);


    Vec3f pos(-230,1.32, 0);
    Camera.setPos(pos);
}


void checktest37(unsigned long timer)
{
    static Vehicle *action = NULL;

    if (timer == 100)
    {
        Balaenidae *_b = (Balaenidae*)findCarrier(GREEN_FACTION);
        size_t idx=0;
        spawnManta(space,world,_b,idx);
    }

    if (timer == 320)
    {
        Balaenidae *_b = (Balaenidae*)findCarrier(GREEN_FACTION);
        // launch
        launchManta(_b);
    }


    if (timer == 420)
    {
        Vehicle *_b = findManta(GREEN_FACTION,FlyingStatus::FLYING);
        SimplifiedDynamicManta *_manta1 = (SimplifiedDynamicManta*)_b;
        _manta1->inert = false;
        _manta1->enableAuto();
        _manta1->setStatus(FlyingStatus::FLYING);
        _manta1->elevator = +5;
        struct controlregister c;
        c.thrust = 400.0f/(10.0);
        c.pitch = 5;
        _manta1->setControlRegisters(c);
        _manta1->setThrottle(400.0f);
        _manta1->disableAuto();
    }

/**
    if (timer == 650)
    {
        SimplifiedDynamicManta *_manta1 = (SimplifiedDynamicManta*)findManta(FlyingStatus::FLYING);

        Balaenidae *_b = (Balaenidae*)findCarrier(GREEN_FACTION);

        _manta1->setDestination(Vec3f(1000,1000,1000));
        _manta1->enableAuto();
    }**/

    if (timer == 1220)
    {
        BoxIsland *island = findIslandByName(std::string("Nemesis"));
        Launcher *lb = (Launcher*) entities[islands[0]->getStructures()[0]];

        Vehicle *target = findNearestEnemyVehicle(BLUE_FACTION, VehicleTypes::MANTA, island->getPos(), 9 * 3.6 kmf);


        if (!target)
            return;

        printf("Found target %p\n",  target);

        Vehicle *b = target;

        Vec3f firingloc = lb->getPos();

        std::cout << lb <<  ":Loc: " << firingloc << " Target: " << b->getPos() << std::endl;

        lb->elevation = -5; // A little bit up.
        lb->azimuth = getAzimuth((b->getPos())-(firingloc));

        struct controlregister c;
        c.pitch = 0.0;
        c.roll = 0.0;
        //lb->setControlRegisters(c);
        lb->setForward(toVectorInFixedSystem(0,0,1,lb->azimuth, -lb->elevation));

        std::cout << lb <<  ":Azimuth: " << lb->azimuth << " Inclination: " << lb->elevation << std::endl;

        action = (lb)->fireAir(world,space);

        if (action != NULL)
        {
            size_t i = entities.push_back(action, action->getGeom());
            //gunshot();

            //action->setDestination(b->getPos());

            //action->enableAuto();

            if (action->getType()==CONTROLABLEACTION)
            {
                switchControl(i);

            }
        }
    }

    if (timer > 1021)
    {
        if (action)
        {
            BoxIsland *island = findIslandByName(std::string("Nemesis"));
            Vehicle *target = findNearestEnemyVehicle(BLUE_FACTION, VehicleTypes::MANTA, island->getPos(), 9 * 3.6 kmf);

            if (!target)
                return;

            std::cout << target <<  ":Loc: " << action->getPos() << " Target: " << target->getPos() << std::endl;

            action->goTo(target->getPos());
            action->enableAuto();

        }
    }

}

void test38()
{
    BoxIsland *nemesis = new BoxIsland(&entities);
    nemesis->setName("Nemesis");
    nemesis->setLocation(0.0f,-1.0,0.0f);
    nemesis->buildTerrainModel(space,"terrain/thermopilae.bmp");

    islands.push_back(nemesis);

    // Entities will be added later in time.
    Balaenidae *_b = new Balaenidae(GREEN_FACTION);
    _b->init();
    _b->embody(world,space);
    _b->setPos(0.0f,20.5f,-16000.0f);
    _b->stop();

    entities.push_back(_b, _b->getGeom());

    Beluga *_bg = new Beluga(BLUE_FACTION);
    _bg->init();
    _bg->embody(world,space);
    _bg->setPos(-4000.0f,20.5f,11000.0f);
    //_bg->setPos(0.0f + 0.0 kmf,20.5f,-6000.0f + 0.0 kmf);
    _bg->stop();

    entities.push_back(_bg, _bg->getGeom());

    Structure *t1 = islands[0]->addStructure(new CommandCenter(BLUE_FACTION, DEFENSE_ISLAND)    ,       200.0f,    -100.0f,0,world);
    Structure *t2 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)           ,         0.0f,    -650.0f,0,world);
    Structure *t3 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)      ,         0.0f,    650.0f,0,world);
    Structure *t4 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)        ,       100.0f,    -650.0f,0,world);
    Structure *t5 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)        ,        20.0f,    80.0f,0,world);
    Structure *t6 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)        ,         -60.0f,    -80.0f,0,world);
    Structure *t7 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)        ,         0.0f,    120.0f,0,world);
    Structure *t8 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)        ,         -230.0f,    230.0f,0,world);


    Vec3f pos(0.0,1.32, - 60);
    Camera.setPos(pos);

    aiplayer = GREEN_AI;
}

void checktest38(unsigned long timer)
{
    // Just check that after a while, the command center is destroyed.

    if (timer > 25000)
    {
        BoxIsland *b = islands[0];
        CommandCenter *c = (CommandCenter*)b->getCommandCenter();

        if (c && c->getFaction() == GREEN_FACTION)
        {
            printf("Test passed OK!\n");
            endWorldModelling();
            exit(1);
        }
        else
        {
            printf("Test failed: Island has not been conquered.\n");
            endWorldModelling();
            exit(0);
        }

    }

}

void test39()
{
    BoxIsland *nemesis = new BoxIsland(&entities);
    nemesis->setName("Nemesis");
    nemesis->setLocation(0.0f,-1.0,0.0f);
    nemesis->buildTerrainModel(space,"terrain/thermopilae.bmp");

    islands.push_back(nemesis);

    // Entities will be added later in time.
    Balaenidae *_b = new Balaenidae(GREEN_FACTION);
    _b->init();
    _b->embody(world,space);
    _b->setPos(0.0f,20.5f,-16000.0f);
    _b->stop();

    entities.push_back(_b, _b->getGeom());

    Beluga *_bg = new Beluga(BLUE_FACTION);
    _bg->init();
    _bg->embody(world,space);
    _bg->setPos(-4000.0f,20.5f,-12000.0f);
    //_bg->setPos(0.0f + 0.0 kmf,20.5f,-6000.0f + 0.0 kmf);
    _bg->stop();

    entities.push_back(_bg, _bg->getGeom());

    Structure *t1 = islands[0]->addStructure(new CommandCenter(BLUE_FACTION, DEFENSE_ISLAND)    ,       200.0f,    -100.0f,0,world);
    Structure *t2 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)           ,         0.0f,    -650.0f,0,world);
    Structure *t3 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)      ,         0.0f,    650.0f,0,world);
    Structure *t4 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)        ,       100.0f,    -650.0f,0,world);
    Structure *t5 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)        ,        20.0f,    80.0f,0,world);
    Structure *t6 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)        ,         -60.0f,    -80.0f,0,world);
    Structure *t7 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)        ,         0.0f,    120.0f,0,world);
    Structure *t8 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)        ,         -230.0f,    230.0f,0,world);


    Vec3f pos(0.0,1.32, - 60);
    Camera.setPos(pos);

    //aiplayer = GREEN_AI;
}

void checktest39(unsigned long timer)
{
    static int number = 0;
    if (timer==500)
    {
        // Detect enemy carrier
        // Move towards it
        // Aim and shoot

        Vehicle *b = findCarrier(GREEN_FACTION);

        AdvancedWalrus* w = (AdvancedWalrus*)spawnWalrus(space,world,b);
        number = w->getNumber();

        Vehicle *v = findNearestEnemyVehicle(GREEN_FACTION,w->getPos(),8000);

        // FIXME 50 meters before from my point of view, along the difference vector.
        w->attack(v->getPos());
        w->enableAuto();


    }

    if (timer==9000)
    {
        Vehicle *b = findCarrier(BLUE_FACTION);

        if (!b)
        {
            printf("Test Ok!.\n");
            endWorldModelling();
            exit(1);
        }
        else
        {
            printf("Test failed: Enemy vehicle is not destroyed.\n");
            endWorldModelling();
            exit(0);
        }

    }

}

void test40()
{
    BoxIsland *nemesis = new BoxIsland(&entities);
    nemesis->setName("Nemesis");
    nemesis->setLocation(0.0f,-1.0,0.0f);
    nemesis->buildTerrainModel(space,"terrain/thermopilae.bmp");

    islands.push_back(nemesis);

    // Entities will be added later in time.
    Balaenidae *_b = new Balaenidae(GREEN_FACTION);
    _b->init();
    _b->embody(world,space);
    _b->setPos(0.0f,20.5f,-16000.0f);
    _b->stop();

    entities.push_back(_b, _b->getGeom());

    Beluga *_bg = new Beluga(BLUE_FACTION);
    _bg->init();
    _bg->embody(world,space);
    _bg->setPos(-4000.0f,20.5f,-12000.0f);
    //_bg->setPos(0.0f + 0.0 kmf,20.5f,-6000.0f + 0.0 kmf);
    _bg->stop();

    entities.push_back(_bg, _bg->getGeom());

    Structure *t1 = islands[0]->addStructure(new CommandCenter(BLUE_FACTION, DEFENSE_ISLAND)    ,       200.0f,    -100.0f,0,world);
    Structure *t2 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)           ,         0.0f,    -650.0f,0,world);
    Structure *t3 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)      ,         0.0f,    650.0f,0,world);
    Structure *t4 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)        ,       100.0f,    -650.0f,0,world);
    Structure *t5 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)        ,        20.0f,    80.0f,0,world);
    Structure *t6 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)        ,         -60.0f,    -80.0f,0,world);
    Structure *t7 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)        ,         0.0f,    120.0f,0,world);
    Structure *t8 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)        ,         -230.0f,    230.0f,0,world);


    Vec3f pos(0.0,1.32, - 60);
    Camera.setPos(pos);

    //aiplayer = GREEN_AI;
}

void checktest40(unsigned long timer)
{
    static int number = 0;
    if (timer==500)
    {
        // Detect enemy carrier
        // Move towards it
        // Aim and shoot

        Vehicle *b = findCarrier(BLUE_FACTION);

        Walrus* w = spawnWalrus(space,world,b);
        number = w->getNumber();

        Vehicle *v = findNearestEnemyVehicle(BLUE_FACTION,w->getPos(),8000);

        // FIXME 50 meters before from my point of view, along the difference vector.
        w->attack(v->getPos());
        w->enableAuto();

    }


    if (timer==15000)
    {
        Vehicle *b = findCarrier(GREEN_FACTION);

        if (!b)
        {
            printf("Test Ok!.\n");
            endWorldModelling();
            exit(1);
        }
        else
        {
            printf("Test failed: Enemy vehicle is not destroyed.\n");
            endWorldModelling();
            exit(0);
        }

    }

}

void test41()
{
    BoxIsland *nemesis = new BoxIsland(&entities);
    nemesis->setName("Nemesis");
    nemesis->setLocation(0.0f,-1.0,0.0f);
    nemesis->buildTerrainModel(space,"terrain/thermopilae.bmp");

    islands.push_back(nemesis);

    // Entities will be added later in time.
    Balaenidae *_b = new Balaenidae(GREEN_FACTION);
    _b->init();
    _b->embody(world,space);
    _b->setPos(0.0f,20.5f,-16000.0f);
    _b->stop();


    entities.push_back(_b, _b->getGeom());

    Beluga *_bg = new Beluga(BLUE_FACTION);
    _bg->init();
    _bg->embody(world,space);
    _bg->setPos(-4000.0f,20.5f,-12000.0f);
    //_bg->setPos(0.0f + 0.0 kmf,20.5f,-6000.0f + 0.0 kmf);
    _bg->stop();

    entities.push_back(_bg, _bg->getGeom());

    Structure *t1 = islands[0]->addStructure(new CommandCenter(BLUE_FACTION, DEFENSE_ISLAND)    ,       200.0f,    -100.0f,0,world);
    Structure *t2 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)           ,         0.0f,    -650.0f,0,world);
    Structure *t3 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)      ,         0.0f,    650.0f,0,world);
    Structure *t4 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)        ,       100.0f,    -650.0f,0,world);
    Structure *t5 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)        ,        20.0f,    80.0f,0,world);
    Structure *t6 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)        ,         -60.0f,    -80.0f,0,world);
    Structure *t7 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)        ,         0.0f,    120.0f,0,world);
    Structure *t8 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)        ,         -230.0f,    230.0f,0,world);


    Vec3f pos(0.0,1.32, - 60);
    Camera.setPos(pos);

    aiplayer = GREEN_FACTION;
}

void checktest41(unsigned long timer)
{

}

void test42()
{
    BoxIsland *nemesis = new BoxIsland(&entities);
    nemesis->setName("Nemesis");
    nemesis->setLocation(0.0f,-1.0,0.0f);
    nemesis->buildTerrainModel(space,"terrain/thermopilae.bmp");

    islands.push_back(nemesis);

    // Entities will be added later in time.
    Balaenidae *_b = new Balaenidae(GREEN_FACTION);
    _b->init();
    _b->embody(world,space);
    _b->setPos(0.0f,20.5f,-16000.0f);
    _b->stop();

    entities.push_back(_b, _b->getGeom());

    Beluga *_bg = new Beluga(BLUE_FACTION);
    _bg->init();
    _bg->embody(world,space);
    _bg->setPos(-2400.0f,20.5f,-12000.0f);
    //_bg->setPos(0.0f + 0.0 kmf,20.5f,-6000.0f + 0.0 kmf);
    _bg->stop();

    entities.push_back(_bg, _bg->getGeom());

    Structure *t1 = islands[0]->addStructure(new CommandCenter(BLUE_FACTION, DEFENSE_ISLAND)    ,       200.0f,    -100.0f,0,world);
    Structure *t2 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)           ,         0.0f,    -650.0f,0,world);
    Structure *t3 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)      ,         0.0f,    650.0f,0,world);
    Structure *t4 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)        ,       100.0f,    -650.0f,0,world);
    Structure *t5 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)        ,        20.0f,    80.0f,0,world);
    Structure *t6 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)        ,         -60.0f,    -80.0f,0,world);
    Structure *t7 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)        ,         0.0f,    120.0f,0,world);
    Structure *t8 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)        ,         -230.0f,    230.0f,0,world);


    Vec3f pos(0.0,1.32, - 60);
    Camera.setPos(pos);

    aiplayer = BOTH_AI;
}

void checktest42(unsigned long timer)
{

}


void test43()
{
    BoxIsland *nemesis = new BoxIsland(&entities);
    nemesis->setName("Nemesis");
    nemesis->setLocation(0.0f,-1.0,0.0f);
    nemesis->buildTerrainModel(space,"terrain/thermopilae.bmp");

    islands.push_back(nemesis);

    // Entities will be added later in time.
    Balaenidae *_b = new Balaenidae(GREEN_FACTION);
    _b->init();
    _b->embody(world,space);
    _b->setPos(0.0f,20.5f,-16000.0f);
    _b->stop();

    entities.push_back(_b, _b->getGeom());

    Beluga *_bg = new Beluga(BLUE_FACTION);
    _bg->init();
    _bg->embody(world,space);
    _bg->setPos(-2400.0f,20.5f,-12000.0f);
    //_bg->setPos(0.0f + 0.0 kmf,20.5f,-6000.0f + 0.0 kmf);
    _bg->stop();

    entities.push_back(_bg,_bg->getGeom());

    Structure *t1 = islands[0]->addStructure(new CommandCenter(BLUE_FACTION, DEFENSE_ISLAND)    ,       200.0f,    -100.0f,0,world);
    Structure *t2 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)           ,         0.0f,    -650.0f,0,world);
    Structure *t3 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)      ,         0.0f,    650.0f,0,world);
    Structure *t4 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)        ,       100.0f,    -650.0f,0,world);
    Structure *t5 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)        ,        20.0f,    80.0f,0,world);
    Structure *t6 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)        ,         -60.0f,    -80.0f,0,world);
    Structure *t7 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)        ,         0.0f,    120.0f,0,world);
    Structure *t8 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)        ,         -230.0f,    230.0f,0,world);


    Vec3f pos(0.0,1.32, - 60);
    Camera.setPos(pos);

    //aiplayer = BOTH_AI;
    controller.faction = BOTH_FACTION;
}

void checktest43(unsigned long timer)
{
    Vehicle *b = findCarrier(GREEN_FACTION);
    Vehicle *l = findCarrier(BLUE_FACTION);

    if (timer == 100)
    {
        size_t idx=0;
        spawnManta(space,world,b,idx);
    }

    if (timer == 320)
    {
        // launch
        launchManta(b);
    }


    if (timer == 420)
    {
        Vehicle *_b = findManta(GREEN_FACTION,FlyingStatus::FLYING);
        SimplifiedDynamicManta *_manta1 = (SimplifiedDynamicManta*)_b;
        _manta1->inert = false;
        _manta1->enableAuto();
        _manta1->setStatus(FlyingStatus::FLYING);
        _manta1->elevator = +5;
        struct controlregister c;
        c.thrust = 800.0f/(10.0);
        c.pitch = 5;
        _manta1->setControlRegisters(c);
        _manta1->setThrottle(400.0f);
        _manta1->disableAuto();
    }

    if (timer == 800)
    {
        size_t idx=0;
        spawnManta(space,world,l,idx);
    }

    if (timer == 920)
    {
        // launch
        launchManta(l);
    }


    if (timer == 1100)
    {
        Vehicle *_b = findManta(BLUE_FACTION,FlyingStatus::FLYING);
        SimplifiedDynamicManta *_manta1 = (SimplifiedDynamicManta*)_b;
        _manta1->inert = false;
        _manta1->enableAuto();
        _manta1->setStatus(FlyingStatus::FLYING);
        _manta1->elevator = +5;
        struct controlregister c;
        c.thrust = 400.0f/(10.0);
        c.pitch = 5;
        _manta1->setControlRegisters(c);
        _manta1->setThrottle(400.0f);
        _manta1->disableAuto();
    }

    if (timer == 1200)
    {
        Manta *_m1 = findManta(GREEN_FACTION,FlyingStatus::FLYING);
        Manta *_ma = findManta(BLUE_FACTION,FlyingStatus::FLYING);

        _ma->dogfight(_m1->getPos());
        _ma->enableAuto();

    }

    if (timer > 1200)
    {
        Manta *_m1 = findManta(GREEN_FACTION,FlyingStatus::FLYING);
        Manta *_ma = findManta(BLUE_FACTION,FlyingStatus::FLYING);

        if (_ma)
        {
            if (_m1)
                _ma->dogfight(_m1->getPos());
            else
            {
                printf("Test Ok!.\n");
                endWorldModelling();
                exit(1);

            }
        }
    }

    if (timer > 10000)
    {
        printf("Test failed: Enemy airplane is not destroyed.\n");
        endWorldModelling();
        exit(0);
    }
}

void test44()
{
    BoxIsland *nemesis = new BoxIsland(&entities);
    nemesis->setName("Nemesis");
    nemesis->setLocation(0.0f,-1.0,0.0f);
    nemesis->buildTerrainModel(space,"terrain/thermopilae.bmp");

    islands.push_back(nemesis);

    // Entities will be added later in time.
    Balaenidae *_b = new Balaenidae(GREEN_FACTION);
    _b->init();
    _b->embody(world,space);
    _b->setPos(0.0f,20.5f,+16000.0f);
    _b->stop();

    entities.push_back(_b, _b->getGeom());

    AdvancedWalrus *_walrus = new AdvancedWalrus(BLUE_FACTION);
    _walrus->init();
    _walrus->embody(world, space);
    _walrus->setPos(200.0f,1.32f,-16000.0f);
    _walrus->setStatus(SailingStatus::SAILING);
    _walrus->stop();

    entities.push_back(_walrus, _walrus->getGeom());

    Structure *t1 = islands[0]->addStructure(new CommandCenter(BLUE_FACTION, DEFENSE_ISLAND)    ,       200.0f,    -100.0f,0,world);
    Structure *t2 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)           ,         0.0f,    -650.0f,0,world);
    Structure *t3 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)      ,         0.0f,    650.0f,0,world);
    Structure *t4 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)        ,       100.0f,    -650.0f,0,world);
    Structure *t5 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)        ,        20.0f,    80.0f,0,world);
    Structure *t6 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)        ,         -60.0f,    -80.0f,0,world);
    Structure *t7 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)        ,         0.0f,    120.0f,0,world);
    Structure *t8 = islands[0]->addStructure(new Antenna(BLUE_FACTION)        ,         -230.0f,    230.0f,0,world);


    Vec3f pos(0.0,1.32, - 60);
    Camera.setPos(pos);

    //aiplayer = BOTH_AI;
    controller.faction = BOTH_FACTION;
}

void checktest44(unsigned long timer)
{
    Vehicle *b = findCarrier(GREEN_FACTION);

    if (timer == 100)
    {
        size_t idx=0;
        spawnManta(space,world,b,idx);
    }

    if (timer == 320)
    {
        // launch
        launchManta(b);
    }


    if (timer == 420)
    {
        Vehicle *_b = findManta(GREEN_FACTION,FlyingStatus::FLYING);
        SimplifiedDynamicManta *_manta1 = (SimplifiedDynamicManta*)_b;
        _manta1->inert = false;
        _manta1->enableAuto();
        _manta1->setStatus(FlyingStatus::FLYING);
        _manta1->elevator = +5;
        struct controlregister c;
        c.thrust = 800.0f/(10.0);
        c.pitch = 5;
        _manta1->setControlRegisters(c);
        _manta1->setThrottle(400.0f);
        _manta1->disableAuto();
    }

    if (timer == 800)
    {
        Walrus *_w1 = findWalrus(SailingStatus::SAILING,BLUE_FACTION,1);

        BoxIsland *is = findIslandByName("Nemesis");

        _w1->goTo(is->getPos());
        _w1->enableAuto();

    }


    if (timer == 1200)
    {
        Walrus *_w1 = findWalrus(SailingStatus::SAILING,BLUE_FACTION,1);
        Manta *_m1 = findManta(GREEN_FACTION,FlyingStatus::FLYING);

        _m1->dogfight(_w1->getPos());
        _m1->enableAuto();

    }

    if (timer > 1200)
    {
        Walrus *_w1 = findWalrus(SailingStatus::SAILING,BLUE_FACTION,1);
        Manta *_m1 = findManta(GREEN_FACTION,FlyingStatus::FLYING);

        if (_m1)
        {
            if (_w1)
                _m1->dogfight(_w1->getPos());
            else
            {
                printf("Test Ok!.\n");
                endWorldModelling();
                exit(1);

            }
        }
    }

    if (timer > 10000)
    {
        printf("Test failed: Enemy walrus is not destroyed.\n");
        endWorldModelling();
        exit(0);
    }
}

void test45()
{
    BoxIsland *nemesis = new BoxIsland(&entities);
    nemesis->setName("Nemesis");
    nemesis->setLocation(0.0f,-1.0,0.0f);
    nemesis->buildTerrainModel(space,"terrain/thermopilae.bmp");

    islands.push_back(nemesis);

    // Entities will be added later in time.
    Balaenidae *_b = new Balaenidae(GREEN_FACTION);
    _b->init();
    _b->embody(world,space);
    _b->setPos(0.0f,20.5f,+16000.0f);
    _b->stop();

    entities.push_back(_b, _b->getGeom());

    Structure *t1 = islands[0]->addStructure(new CommandCenter(BLUE_FACTION, DEFENSE_ISLAND)    ,       200.0f,    -100.0f,0,world);
    Structure *t2 = islands[0]->addStructure(new Runway(BLUE_FACTION)           ,         0.0f,    -650.0f,0,world);
    Structure *t3 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)      ,         0.0f,    650.0f,0,world);
    Structure *t4 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)        ,       100.0f,    -650.0f,0,world);
    Structure *t5 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)        ,        20.0f,    80.0f,0,world);
    Structure *t6 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)        ,         -60.0f,    -80.0f,0,world);
    Structure *t7 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)        ,         0.0f,    120.0f,0,world);
    Structure *t8 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)        ,         -230.0f,    230.0f,0,world);


    Vec3f pos(0.0,1.32, - 60);
    Camera.setPos(pos);

    //aiplayer = BOTH_AI;
    controller.faction = BOTH_FACTION;
}

void checktest45(unsigned long timer)
{
    unsigned long starttime = 300;
    static Manta *m;
    static bool found=false;

    if (timer == starttime + 100)
    {
        size_t idx=0;
        spawnManta(space,world,entities[1],idx);
    }

    if (timer == starttime + 320)
    {
        // launch
        m = launchManta(entities[1]);
    }

    if (timer == starttime + 600)
    {
        SimplifiedDynamicManta *_manta1 = (SimplifiedDynamicManta*)m;


        BoxIsland *is = findIslandByName("Nemesis");

        Structure *c = is->getCommandCenter();

        _manta1->attack(c->getPos());
        _manta1->enableAuto();
    }

    if (timer > starttime + 600)
    {
        Manta *enemy = findManta(BLUE_FACTION, FlyingStatus::FLYING);

        if (enemy)
        {
            printf ("Medusa: %p\n", enemy);
            printf ("Manta %p\n", m);

            if (!found)
            {
                m->dogfight(enemy->getPos());
                m->enableAuto();
                found=true;
            }

            m->dogfight(enemy->getPos());
        } else {
            BoxIsland *is = findIslandByName("Nemesis");

            Structure *c = is->getCommandCenter();

            m->attack(c->getPos());
            m->enableAuto();

            found = false;
        }


    }


    if (timer > starttime + 3000)
    {
        if (fps > 40)
        {
            printf("Test passed OK!\n");
            endWorldModelling();
            exit(1);
        } else {
            printf("Test failed: FPS is too slow. \n");
            endWorldModelling();
            exit(0);
        }
    }
}

void test46()
{
    // Entities will be added later in time.
    Balaenidae *_b = new Balaenidae(GREEN_FACTION);
    _b->init();
    _b->embody(world,space);
    _b->setPos(0.0f,20.5f,-12000.0f);
    _b->stop();

    entities.push_back(_b, _b->getGeom());

    CarrierTurret * _bo= new CarrierTurret(GREEN_FACTION);
    _bo->init();
    _bo->embody(world, space);
    _bo->attachTo(world,_b, -40.0f, 20.0f + 5, -210.0f);
    _bo->stop();

    entities.push_back(_bo, _bo->getGeom());


    CarrierArtillery * _w1= new CarrierArtillery(GREEN_FACTION);
    _w1->init();
    _w1->embody(world, space);
    _w1->attachTo(world,_b, -40.0, 27.0f, +210.0f);
    _w1->stop();

    entities.push_back(_w1, _w1->getGeom());

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

void checktest46(unsigned long timer)
{
    static std::ofstream fpsfile;

    long unsigned starttime = 2500;

    if (timer == 1)
    {
        fpsfile.open ("fps.dat");
    }

    fpsfile << entities.size() << "," <<  fps << "," << elapsedtime << std::endl;
    fpsfile.flush();


    if (timer == 200)
    {
        char msg[256];
        Message mg;
        sprintf(msg, "Schedule attack at %ld", starttime);
        mg.faction = BOTH_FACTION;
        mg.msg = std::string(msg);
        messages.insert(messages.begin(), mg);
    }

    if (timer == 100)
    {
        for (int j=0;j<islands.size();j++)
        {
            captureIsland(islands[j],BLUE_FACTION,DEFENSE_ISLAND,space,world);
        }

        BoxIsland *is = findIslandByName("Statera");
        Structure *t8 = is->addStructure(new Runway(BLUE_FACTION)        ,         -230.0f,    230.0f,0,world);

        // Accelerate time
        for (int j=0;j<1000;j++)
        {
            buildAndRepair(true,space,world);
        }



    }

    if (timer == 1000)
    {
        for(size_t i=entities.first();entities.hasMore(i);i=entities.next(i))
        {
            if (!((entities[i]->getPos() - Camera.getPos()).magnitude()<10000))
            {
                if (entities[i]->getBodyID() == NULL)
                {
                    dGeomDisable(entities[i]->getGeom());
                }
            }
        }
    }

    if (timer == starttime)
    {
        char msg[256];
        Message mg;
        sprintf(msg, "Air and amphibious attack started.");
        mg.faction = BOTH_FACTION;
        mg.msg = std::string(msg);
        messages.insert(messages.begin(), mg);
    }

    if (timer==starttime + 90)
    {

        Vehicle *b = findCarrier(GREEN_FACTION);

        BoxIsland *is = findIslandByName("Statera");

        Structure *c = is->getCommandCenter();

        for(int i=0;i<4;i++)
        {
            AdvancedWalrus* w = (AdvancedWalrus*)spawnWalrus(space,world,b);


            // FIXME 50 meters before from my point of view, along the difference vector.
            w->attack(c->getPos());
            w->enableAuto();
        }

    }


    if (timer == starttime + 100)
    {
        size_t idx=0;
        spawnManta(space,world,entities[1],idx);
    }

    if (timer == starttime + 320)
    {
        // launch
        launchManta(entities[1]);
    }


    if (timer == starttime + 420)
    {
        Vehicle *_b = findManta(GREEN_FACTION,FlyingStatus::FLYING);
        SimplifiedDynamicManta *_manta1 = (SimplifiedDynamicManta*)_b;
        _manta1->inert = false;
        _manta1->enableAuto();
        _manta1->setStatus(FlyingStatus::FLYING);
        _manta1->elevator = +5;
        struct controlregister c;
        c.thrust = 400.0f/(10.0);
        c.pitch = 5;
        _manta1->setControlRegisters(c);
        _manta1->setThrottle(400.0f);
        _manta1->disableAuto();
    }

    if (timer == starttime + 450)
    {
        Vehicle *_b = findManta(GREEN_FACTION,FlyingStatus::FLYING);
        SimplifiedDynamicManta *_manta1 = (SimplifiedDynamicManta*)_b;

        BoxIsland *is = findIslandByName("Statera");

        Structure *c = is->getCommandCenter();

        _manta1->attack(c->getPos());
        _manta1->enableAuto();
    }




    if (timer > starttime + 10000)
    {
        if (fps > 40)
        {
            fpsfile.close();
            printf("Test passed OK!\n");
            endWorldModelling();
            exit(1);
        } else {
            fpsfile.close();
            printf("Test failed: FPS is too slow. \n");
            endWorldModelling();
            exit(0);
        }
    }


}


void test47()
{
    BoxIsland *nemesis = new BoxIsland(&entities);
    nemesis->setName("Nemesis");
    nemesis->setLocation(0.0f,-1.0,0.0f);
    nemesis->buildTerrainModel(space,"terrain/thermopilae.bmp");

    islands.push_back(nemesis);

    // Entities will be added later in time.
    Balaenidae *_b = new Balaenidae(GREEN_FACTION);
    _b->init();
    _b->embody(world,space);
    _b->setPos(0.0f,20.5f,-16000.0f);
    _b->stop();

    entities.push_back(_b, _b->getGeom());

    Structure *t1 = islands[0]->addStructure(new CommandCenter(BLUE_FACTION, DEFENSE_ISLAND)    ,       200.0f,    -100.0f,0,world);
    Structure *t2 = islands[0]->addStructure(new Runway(BLUE_FACTION)           ,         0.0f,    -650.0f,0,world);
    Structure *t3 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)      ,         0.0f,    650.0f,0,world);
    Structure *t4 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)        ,       100.0f,    -650.0f,0,world);
    Structure *t5 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)        ,        20.0f,    80.0f,0,world);
    Structure *t6 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)        ,         -60.0f,    -80.0f,0,world);
    Structure *t7 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)        ,         0.0f,    120.0f,0,world);
    Structure *t8 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)        ,         -230.0f,    230.0f,0,world);


    Vec3f pos(0.0,1.32, - 60);
    Camera.setPos(pos);

    aiplayer = BOTH_AI;
    controller.faction = BOTH_FACTION;
}

void checktest47(unsigned long timer)
{
    unsigned long starttime = 300;

    if (timer > starttime + 30000)
    {
        if (fps > 40)
        {
            printf("Test passed OK!\n");
            endWorldModelling();
            exit(1);
        } else {
            printf("Test failed: FPS is too slow. \n");
            endWorldModelling();
            exit(0);
        }
    }
}


void test48()
{
    BoxIsland *nemesis = new BoxIsland(&entities);
    nemesis->setName("Nemesis");
    nemesis->setLocation(0.0f,-1.0,0.0f);
    nemesis->buildTerrainModel(space,"terrain/thermopilae.bmp");

    islands.push_back(nemesis);

    // Entities will be added later in time.
    Balaenidae *_b = new Balaenidae(GREEN_FACTION);
    _b->init();
    _b->embody(world,space);
    _b->setPos(0.0f,20.5f,-16000.0f);
    _b->stop();

    entities.push_back(_b, _b->getGeom());

    Structure *t1 = islands[0]->addStructure(new CommandCenter(BLUE_FACTION, DEFENSE_ISLAND)    ,       200.0f,    -100.0f,0,world);
    Structure *t2 = islands[0]->addStructure(new Runway(BLUE_FACTION)           ,         0.0f,    -650.0f,0,world);
    Structure *t3 = islands[0]->addStructure(new LaserTurret(BLUE_FACTION)      ,         0.0f,    650.0f,0,world);
    Structure *t4 = islands[0]->addStructure(new LaserTurret(BLUE_FACTION)        ,       100.0f,    -650.0f,0,world);
    Structure *t5 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)        ,        20.0f,    80.0f,0,world);
    Structure *t6 = islands[0]->addStructure(new Turret(BLUE_FACTION)        ,         -60.0f,    -80.0f,0,world);
    Structure *t7 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)        ,         0.0f,    120.0f,0,world);
    Structure *t8 = islands[0]->addStructure(new Turret(BLUE_FACTION)        ,         -230.0f,    230.0f,0,world);
    Structure *t9 = islands[0]->addStructure(new Antenna(BLUE_FACTION),world);


    Vec3f pos(0.0,1.32, - 60);
    Camera.setPos(pos);

    aiplayer = BLUE_AI;
    controller.faction = BOTH_FACTION;
}

void checktest48(unsigned long timer)
{
    unsigned long starttime = 300;

    if (timer == starttime + 500)
    {
        Vehicle *b = findCarrier(GREEN_FACTION);
        Walrus* w = spawnWalrus(space,world,b);

        w->goTo(Vec3f(0,0,0));
        w->enableAuto();
    }

    // Walrus will approach the island, and hopefully will be destroyed

    if (timer > starttime + 20000)
    {
        Manta *m = findMantaByOrder(BLUE_FACTION,DEFEND_ISLAND);


        if (m && m->getStatus()==FlyingStatus::LANDED)
        {
            printf("Test passed OK!\n");
            endWorldModelling();
            exit(1);
        } else {
            printf("Test failed: Medusa has not landed correctly or at least is not registered as landed.\n");
            endWorldModelling();
            exit(0);
        }
    }
}


void test49()
{
    BoxIsland *nemesis = new BoxIsland(&entities);
    nemesis->setName("Nemesis");
    nemesis->setLocation(0.0f,-1.0,0.0f);
    nemesis->buildTerrainModel(space,"terrain/thermopilae.bmp");

    islands.push_back(nemesis);

    // Entities will be added later in time.
    Balaenidae *_b = new Balaenidae(GREEN_FACTION);
    _b->init();
    _b->embody(world,space);
    _b->setPos(0.0f,20.5f,-16000.0f);
    _b->stop();

    entities.push_back(_b, _b->getGeom());

    Structure *t1 = islands[0]->addStructure(new CommandCenter(GREEN_FACTION, DEFENSE_ISLAND)    ,       800.0f,    -100.0f,0,world);
    Structure *t2 = islands[0]->addStructure(new Runway(GREEN_FACTION)           ,         0.0f,    -650.0f,-PI/4,world);
    Structure *t3 = islands[0]->addStructure(new Warehouse(GREEN_FACTION)      ,         0.0f,    650.0f,0,world);
    Structure *t4 = islands[0]->addStructure(new Warehouse(GREEN_FACTION)        ,       100.0f,    -650.0f,0,world);
    Structure *t5 = islands[0]->addStructure(new Warehouse(GREEN_FACTION)        ,        20.0f,    80.0f,0,world);
    Structure *t6 = islands[0]->addStructure(new Warehouse(GREEN_FACTION)        ,         -60.0f,    -80.0f,0,world);
    Structure *t7 = islands[0]->addStructure(new Warehouse(GREEN_FACTION)        ,         0.0f,    120.0f,0,world);
    Structure *t8 = islands[0]->addStructure(new Warehouse(GREEN_FACTION)        ,         -230.0f,    230.0f,0,world);

    Vec3f pos(0.0,1.32, - 60);
    Camera.setPos(pos);

    aiplayer = BLUE_AI;
    controller.faction = BOTH_FACTION;
}

void checktest49(unsigned long timer)
{

}

void test50()
{
    BoxIsland *nemesis = new BoxIsland(&entities);
    nemesis->setName("Nemesis");
    nemesis->setLocation(0.0f,-1.0,0.0f);
    nemesis->buildTerrainModel(space,"terrain/thermopilae.bmp");

    islands.push_back(nemesis);

    // Entities will be added later in time.
    Balaenidae *_b = new Balaenidae(GREEN_FACTION);
    _b->init();
    _b->embody(world,space);
    _b->setPos(0.0f,20.5f,-16000.0f);
    _b->stop();

    entities.push_back(_b, _b->getGeom());

    Structure *t1 = islands[0]->addStructure(new CommandCenter(GREEN_FACTION, DEFENSE_ISLAND)    ,       800.0f,    -100.0f,0,world);
    Structure *t2 = islands[0]->addStructure(new Runway(GREEN_FACTION)           ,         0.0f,    -650.0f,-PI/4,world);
    Structure *t3 = islands[0]->addStructure(new Warehouse(GREEN_FACTION)      ,         0.0f,    650.0f,0,world);
    Structure *t4 = islands[0]->addStructure(new Warehouse(GREEN_FACTION)        ,       100.0f,    -650.0f,0,world);
    Structure *t5 = islands[0]->addStructure(new Radar(GREEN_FACTION)        ,        20.0f,    80.0f,0,world);
    Structure *t6 = islands[0]->addStructure(new Dock(GREEN_FACTION)             ,         -0,    -1800,0,world);
    Structure *t7 = islands[0]->addStructure(new Factory(GREEN_FACTION)        ,         0.0f,    1000.0f,0,world);
    Structure *t8 = islands[0]->addStructure(new Antenna(GREEN_FACTION)        ,         -1000.0f,    230.0f,0,world);

    Vec3f pos(0.0,1.32, - 60);
    Camera.setPos(pos);

    aiplayer = BLUE_AI;
    controller.faction = BOTH_FACTION;
}

void checktest50(unsigned long timer)
{
    long unsigned starttime = 200;

    if (timer == starttime)
    {
        char msg[256];
        Message mg;
        sprintf(msg, "Check visually the structures (cannot do that from a unit test).");
        mg.faction = BOTH_FACTION;
        mg.msg = std::string(msg);
        messages.insert(messages.begin(), mg);
    }

    if (timer > starttime + 5000)
    {
        printf("Test passed OK!\n");
        endWorldModelling();
        exit(1);
    }
}


void test51()
{
    BoxIsland *nemesis = new BoxIsland(&entities);
    nemesis->setName("Nemesis");
    nemesis->setLocation(0.0f,-1.0,0.0f);
    nemesis->buildTerrainModel(space,"terrain/thermopilae.bmp");

    islands.push_back(nemesis);

    // Entities will be added later in time.
    Balaenidae *_b = new Balaenidae(GREEN_FACTION);
    _b->init();
    _b->embody(world,space);
    _b->setPos(0.0f,20.5f,-16000.0f);
    _b->stop();

    entities.push_back(_b, _b->getGeom());

    Structure *t1 = islands[0]->addStructure(new CommandCenter(GREEN_FACTION, DEFENSE_ISLAND)    ,       800.0f,    -100.0f,0,world);
    Structure *t2 = islands[0]->addStructure(new Runway(GREEN_FACTION)           ,         0.0f,    -650.0f,-PI/4,world);
    Structure *t3 = islands[0]->addStructure(new Warehouse(GREEN_FACTION)      ,         0.0f,    650.0f,0,world);
    Structure *t4 = islands[0]->addStructure(new Warehouse(GREEN_FACTION)        ,       100.0f,    -650.0f,0,world);
    Structure *t5 = islands[0]->addStructure(new Radar(GREEN_FACTION)        ,        20.0f,    80.0f,0,world);
    Structure *t6 = islands[0]->addStructure(new Dock(GREEN_FACTION)             ,         -0,    -1700,0,world);
    Structure *t7 = islands[0]->addStructure(new Factory(GREEN_FACTION)        ,         0.0f,    1000.0f,0,world);
    Structure *t8 = islands[0]->addStructure(new Antenna(GREEN_FACTION)        ,         -1000.0f,    230.0f,0,world);

    Vec3f pos(0.0,1.32, - 60);
    Camera.setPos(pos);

    aiplayer = BLUE_AI;
    controller.faction = BOTH_FACTION;
}

void message(char* mms)
{
    char msg[256];
    Message mg;
    sprintf(msg, "%s",mms);
    mg.faction = BOTH_FACTION;
    mg.msg = std::string(msg);
    messages.insert(messages.begin(), mg);
}


void checktest51(unsigned long timer)
{
    long unsigned starttime = 200;

    if (timer == starttime)
    {
        char msg[256];
        Message mg;
        sprintf(msg, "Initiating islands configuration");
        mg.faction = BOTH_FACTION;
        mg.msg = std::string(msg);
        messages.insert(messages.begin(), mg);
    }

    if (timer == starttime + 400)
    {

        Runway *r = (Runway*)entities[islands[0]->getStructures()[1]];

        int mantNumber = findNextNumber(GREEN_FACTION,MANTA, VehicleSubTypes::MEDUSA);
        Vehicle *manta = (r)->spawn(world,space,MANTA,mantNumber);
        size_t l = entities.push_back(manta, manta->getGeom());
        manta->setOrder(DEFEND_ISLAND);

        message("Medusa airplane has been spawned");

    }

    if (timer == starttime + 500)
    {
        savegame("savegames/test.w");
        message("Game saved.");
    }


    if (timer == starttime + 505)
    {
        islands.clear();
        for(size_t i=entities.first();entities.hasMore(i);i=entities.next(i))
        {
            entities[i]->damage(100000);
        }

    }


    if (timer == starttime + 1200)
    {
        loadgame("savegames/test.w");
    }




    if (timer > starttime + 5000)
    {
        printf("Test passed OK!\n");
        endWorldModelling();
        exit(1);
    }

}


void test52()
{
    BoxIsland *nemesis = new BoxIsland(&entities);
    nemesis->setName("Nemesis");
    nemesis->setLocation(0.0f,-1.0,0.0f);
    nemesis->buildTerrainModel(space,"terrain/thermopilae.bmp");

    islands.push_back(nemesis);

    // Entities will be added later in time.
    Beluga *_b = new Beluga(BLUE_FACTION);
    _b->init();
    _b->embody(world,space);
    _b->setPos(0.0f,20.5f,-16000.0f);
    _b->stop();

    entities.push_back(_b, _b->getGeom());

    Structure *t1 = islands[0]->addStructure(new CommandCenter(GREEN_FACTION, DEFENSE_ISLAND)    ,       800.0f,    -100.0f,0,world);
    Structure *t2 = islands[0]->addStructure(new Runway(GREEN_FACTION)           ,         0.0f,    -650.0f,-PI/4,world);
    Structure *t3 = islands[0]->addStructure(new Warehouse(GREEN_FACTION)      ,         0.0f,    650.0f,0,world);
    Structure *t4 = islands[0]->addStructure(new Warehouse(GREEN_FACTION)        ,       100.0f,    -650.0f,0,world);
    Structure *t5 = islands[0]->addStructure(new Radar(GREEN_FACTION)        ,        20.0f,    80.0f,0,world);
    Structure *t6 = islands[0]->addStructure(new Dock(GREEN_FACTION)             ,         -0,    -1700,0,world);
    Structure *t7 = islands[0]->addStructure(new Factory(GREEN_FACTION)        ,         0.0f,    1000.0f,0,world);
    Structure *t8 = islands[0]->addStructure(new Antenna(GREEN_FACTION)        ,         -1000.0f,    230.0f,0,world);

    Vec3f pos(0.0,1.32, - 60);
    Camera.setPos(pos);

    aiplayer = FREE_AI;
    controller.faction = BOTH_FACTION;
}



void checktest52(unsigned long timer)
{
    long unsigned starttime = 150;

    if (timer == starttime)
    {
        char msg[256];
        Message mg;
        sprintf(msg, "TC52: Cephalopod basic dynamics, hoovering, stability and basic auto destination.");
        mg.faction = BOTH_FACTION;
        mg.msg = std::string(msg);
        messages.insert(messages.begin(), mg);
    }

    if (timer == starttime + 10)
    {
        Beluga *b = (Beluga*)findCarrier(BLUE_FACTION);
        size_t idx=0;
        Cephalopod* m = (Cephalopod*)(b->spawn(world,space,CEPHALOPOD,findNextNumber(BLUE_FACTION,MANTA,CEPHALOPOD)));

        idx = entities.push_back(m, m->getGeom());

        controller.controllingid = idx;

        m->goTo(Vec3f(+20000,10.0,-4500));
    }

    if (timer == starttime + 150)
    {
        Beluga *b = (Beluga*)findCarrier(BLUE_FACTION);

        launchManta(b);
    }


    if (timer > starttime + 15000)
    {
        Cephalopod *m = (Cephalopod*)findManta(BLUE_FACTION,FlyingStatus::HOLDING);

        if (!m)
        {
            printf("Test failed: Cephalopod has been destroyed.\n");
            endWorldModelling();
            exit(0);
        }

        if ((m->getPos()-Vec3f(+20000,300,-4500)).magnitude()<1000)
        {
            printf("Test passed OK!\n");
            endWorldModelling();
            exit(1);
        }
    }

}


void test53()
{
    BoxIsland *nemesis = new BoxIsland(&entities);
    nemesis->setName("Nemesis");
    nemesis->setLocation(0.0f,-1.0,0.0f);
    nemesis->buildTerrainModel(space,"terrain/thermopilae.bmp");

    islands.push_back(nemesis);

    // Entities will be added later in time.
    Beluga *_b = new Beluga(BLUE_FACTION);
    _b->init();
    _b->embody(world,space);
    _b->setPos(0.0f,20.5f,-16000.0f);
    _b->stop();

    entities.push_back(_b, _b->getGeom());

    Structure *t1 = islands[0]->addStructure(new CommandCenter(GREEN_FACTION, FACTORY_ISLAND)    ,       800.0f,    -100.0f,0,world);
    //Structure *t2 = islands[0]->addStructure(new Runway(GREEN_FACTION)           ,         0.0f,    -650.0f,-PI/4,world);
    Structure *t3 = islands[0]->addStructure(new Warehouse(GREEN_FACTION)      ,         0.0f,    650.0f,0,world);
    Structure *t4 = islands[0]->addStructure(new Warehouse(GREEN_FACTION)        ,       100.0f,    -650.0f,0,world);
    Structure *t5 = islands[0]->addStructure(new Radar(GREEN_FACTION)        ,        20.0f,    80.0f,0,world);
    Structure *t6 = islands[0]->addStructure(new Dock(GREEN_FACTION)             ,         -0,    -1700,0,world);
    Structure *t7 = islands[0]->addStructure(new Factory(GREEN_FACTION)        ,         0.0f,    1000.0f,0,world);
    Structure *t8 = islands[0]->addStructure(new Antenna(GREEN_FACTION)        ,         -1000.0f,    230.0f,0,world);

    Vec3f pos(0.0,1.32, - 60);
    Camera.setPos(pos);

    aiplayer = FREE_AI;
    controller.faction = BOTH_FACTION;
}



void checktest53(unsigned long timer)
{
    long unsigned starttime = 150;

    if (timer == starttime)
    {
        char msg[256];
        Message mg;
        sprintf(msg, "TC53: Cephalopod positioning and aiming to target.");
        mg.faction = BOTH_FACTION;
        mg.msg = std::string(msg);
        messages.insert(messages.begin(), mg);
    }

    if (timer == starttime + 10)
    {
        Beluga *b = (Beluga*)findCarrier(BLUE_FACTION);
        size_t idx=0;
        Cephalopod* m = (Cephalopod*)(b->spawn(world,space,CEPHALOPOD,findNextNumber(BLUE_FACTION,MANTA,CEPHALOPOD)));

        idx = entities.push_back(m, m->getGeom());

        controller.controllingid = idx;

        m->goTo(Vec3f(+2000,10.0,-4500));
    }

    if (timer == starttime + 150)
    {
        Beluga *b = (Beluga*)findCarrier(BLUE_FACTION);

        launchManta(b);
    }

    if (timer > starttime + 250)
    {
        Cephalopod *m = (Cephalopod*)findManta(BLUE_FACTION,FlyingStatus::HOLDING);

        if (m && m->getStatus()==FlyingStatus::HOLDING)
        {
            CommandCenter *c = (CommandCenter*)findIslandByName("Nemesis")->getCommandCenter();

            if (c)
            {

                m->attack(c->getPos());
                m->enableAuto();
                m->setStatus(FlyingStatus::FLYING);

            }
        }

        m = (Cephalopod*)findManta(BLUE_FACTION,FlyingStatus::FLYING);

        if (m)
        {
            CommandCenter *c = (CommandCenter*)findIslandByName("Nemesis")->getCommandCenter();

            if (!c)
            {
                Beluga *b = (Beluga*)findCarrier(BLUE_FACTION);

                printf ("Distance to carrier %10.8f", (b->getPos()-m->getPos()).magnitude());
                if ( (b->getPos()-m->getPos()).magnitude() > 1000)
                {
                    m->goTo(b->getPos());
                    m->enableAuto();
                } else {
                    runonce {landManta(b,m);}
                    // @FIXME What happen if the carrier moves?
                }
            }
        }


    }


    if (timer > starttime + 15000)
    {
        Cephalopod *m = (Cephalopod*)findManta(BLUE_FACTION);

        if (!m)
        {
            printf("Test failed: Cephalopod has been destroyed.\n");
            endWorldModelling();
            exit(0);
        }

        CommandCenter *c = (CommandCenter*)findIslandByName("Nemesis")->getCommandCenter();

        if (!c)
        {
            printf("Test passed OK!\n");
            endWorldModelling();
            exit(1);
        }
    }

}


void test54()
{
    BoxIsland *nemesis = new BoxIsland(&entities);
    nemesis->setName("Nemesis");
    nemesis->setLocation(0.0f,-1.0,0.0f);
    nemesis->buildTerrainModel(space,"terrain/thermopilae.bmp");

    islands.push_back(nemesis);

    // Entities will be added later in time.
    Beluga *_b = new Beluga(BLUE_FACTION);
    _b->init();
    _b->embody(world,space);
    _b->setPos(0.0f,20.5f,-16000.0f);
    _b->stop();

    entities.push_back(_b, _b->getGeom());

    Structure *t1 = islands[0]->addStructure(new CommandCenter(GREEN_FACTION, FACTORY_ISLAND)    ,       800.0f,    -100.0f,0,world);
    //Structure *t2 = islands[0]->addStructure(new Runway(GREEN_FACTION)           ,         0.0f,    -650.0f,-PI/4,world);
    Structure *t3 = islands[0]->addStructure(new Warehouse(GREEN_FACTION)      ,         0.0f,    650.0f,0,world);
    Structure *t4 = islands[0]->addStructure(new Warehouse(GREEN_FACTION)        ,       100.0f,    -650.0f,0,world);
    Structure *t5 = islands[0]->addStructure(new Radar(GREEN_FACTION)        ,        20.0f,    80.0f,0,world);
    Structure *t6 = islands[0]->addStructure(new Dock(GREEN_FACTION)             ,         -0,    -1700,0,world);
    Structure *t7 = islands[0]->addStructure(new Factory(GREEN_FACTION)        ,         0.0f,    1000.0f,0,world);
    Structure *t8 = islands[0]->addStructure(new Antenna(GREEN_FACTION)        ,         -1000.0f,    230.0f,0,world);

    Vec3f pos(0.0,1.32, - 60);
    Camera.setPos(pos);

    aiplayer = BLUE_AI;
    controller.faction = BOTH_FACTION;
}



void checktest54(unsigned long timer)
{
    long unsigned starttime = 150;

    if (timer == starttime)
    {
        char msg[256];
        Message mg;
        sprintf(msg, "TC54: Stop aircraft from departing if the island is already free.");
        mg.faction = BOTH_FACTION;
        mg.msg = std::string(msg);
        messages.insert(messages.begin(), mg);
    }

    if (timer == starttime + 1300)
    {
        CommandCenter *c = (CommandCenter*)findIslandByName("Nemesis")->getCommandCenter();

        if (c)
        {
            // This is in case the missile do not destroy the command center.
            c->damage(10000);
        }
    }

    if (timer > starttime + 2500)
    {
        Manta *m = (Manta*)findManta(BLUE_FACTION);

        if (m)
        {
            printf("Test failed: Manta should have not been launched.\n");
            endWorldModelling();
            exit(0);
        } else
        {
            printf("Test passed OK!\n");
            endWorldModelling();
            exit(1);
        }
    }

}


void test55()
{
    BoxIsland *nemesis = new BoxIsland(&entities);
    nemesis->setName("Nemesis");
    nemesis->setLocation(0.0f,-1.0,0.0f);
    nemesis->buildTerrainModel(space,"terrain/thermopilae.bmp");

    islands.push_back(nemesis);

    // Entities will be added later in time.
    Beluga *_b = new Beluga(BLUE_FACTION);
    _b->init();
    _b->embody(world,space);
    _b->setPos(0.0f,20.5f,-16000.0f);
    _b->stop();

    entities.push_back(_b, _b->getGeom());

    Structure *t1 = islands[0]->addStructure(new CommandCenter(GREEN_FACTION, FACTORY_ISLAND)    ,       800.0f,    -100.0f,0,world);
    //Structure *t2 = islands[0]->addStructure(new Runway(GREEN_FACTION)           ,         0.0f,    -650.0f,-PI/4,world);
    Structure *t3 = islands[0]->addStructure(new Warehouse(GREEN_FACTION)      ,         0.0f,    650.0f,0,world);
    Structure *t4 = islands[0]->addStructure(new Warehouse(GREEN_FACTION)        ,       100.0f,    -650.0f,0,world);
    Structure *t5 = islands[0]->addStructure(new Radar(GREEN_FACTION)        ,        20.0f,    80.0f,0,world);
    Structure *t6 = islands[0]->addStructure(new Dock(GREEN_FACTION)             ,         -0,    -1700,0,world);
    Structure *t7 = islands[0]->addStructure(new Factory(GREEN_FACTION)        ,         0.0f,    1000.0f,0,world);
    Structure *t8 = islands[0]->addStructure(new Antenna(GREEN_FACTION)        ,         -1000.0f,    230.0f,0,world);

    Vec3f pos(0.0,1.32, - 60);
    Camera.setPos(pos);

    aiplayer = FREE_AI;
    controller.faction = BOTH_FACTION;
}



void checktest55(unsigned long timer)
{
    long unsigned starttime = 150;

    if (timer == starttime)
    {
        char msg[256];
        Message mg;
        sprintf(msg, "TC55: Checking comm link interruption.");
        mg.faction = BOTH_FACTION;
        mg.msg = std::string(msg);
        messages.insert(messages.begin(), mg);
    }

    if (timer == starttime + 300)
    {
        Beluga *b = (Beluga*)findCarrier(BLUE_FACTION);
        size_t idx = 0;
        spawnManta(space,world,b,idx);
    }

    if (timer == starttime + 500)
    {
        Beluga *b = (Beluga*)findCarrier(BLUE_FACTION);
        launchManta(b);

    }

    if (timer == starttime + 800)
    {
        Manta *m = (Manta*)findManta(BLUE_FACTION);

        m->goTo(Vec3f(100 kmf, 0, 100 kmf));
        m->enableAuto();

        char msg[256];
        Message mg;
        sprintf(msg, "TC55: Manta will be flying far away.");
        mg.faction = BOTH_FACTION;
        mg.msg = std::string(msg);
        messages.insert(messages.begin(), mg);

    }


    if (timer == starttime + 1300)
    {
        CommandCenter *c = (CommandCenter*)findIslandByName("Nemesis")->getCommandCenter();

        if (c)
        {
            // This is in case the missile do not destroy the command center.
            c->damage(10000);
        }
    }

    if (timer > starttime + 12500)
    {
        Manta *m = (Manta*)findManta(BLUE_FACTION);

        if (m)
        {
            printf("Test failed: Manta should be destroyed\n");
            endWorldModelling();
            exit(0);
        } else
        {
            printf("Test passed OK!\n");
            endWorldModelling();
            exit(1);
        }
    }

}

void test56()
{
    // Entities will be added later in time.
    Balaenidae *_b = new Balaenidae(GREEN_FACTION);
    _b->init();
    _b->embody(world,space);
    _b->setPos(0.0f,20.5f,-9000.0f);
    _b->stop();

    entities.push_back(_b, _b->getGeom());
}

void checktest56(unsigned long timer)
{
    static bool reached = false;


    if (timer == 100)
    {
        size_t idx = 0;
        spawnManta(space,world,entities[1],idx);
    }

    if (timer == 320)
    {
        // launch
        launchManta(entities[1]);
    }


    if (timer == 420)
    {
        Vehicle *_b = findManta(GREEN_FACTION,FlyingStatus::FLYING);
        SimplifiedDynamicManta *_manta1 = (SimplifiedDynamicManta*)_b;
        _manta1->inert = false;
        _manta1->enableAuto();
        _manta1->setStatus(FlyingStatus::FLYING);
        _manta1->elevator = +5;
        struct controlregister c;
        c.thrust = 400.0f/(10.0);
        c.pitch = 5;
        _manta1->setControlRegisters(c);
        _manta1->setThrottle(400.0f);
        _manta1->disableAuto();
    }


    if (timer == 650)
    {
        SimplifiedDynamicManta *_manta1 = (SimplifiedDynamicManta*)findManta(GREEN_FACTION,FlyingStatus::FLYING);

        Balaenidae *_b = (Balaenidae*)findCarrier(GREEN_FACTION);

        _manta1->goTo(_b->getPos()-_b->getForward().normalize()*(10 kmf));
        _manta1->setAttitude(_b->getForward());
        _manta1->enableAuto();
    }

    if (timer > 650)
    {
        SimplifiedDynamicManta *_manta1 = (SimplifiedDynamicManta*)findManta(GREEN_FACTION,FlyingStatus::FLYING);

        Balaenidae *_b = (Balaenidae*)findCarrier(GREEN_FACTION);

        if (_manta1)
           _manta1->setAttitude( _b->getForward());

    }


    if (timer > 700)
    {
        // Auto control
        SimplifiedDynamicManta *_manta1 = (SimplifiedDynamicManta*)findManta(GREEN_FACTION,FlyingStatus::HOLDING);

        Balaenidae *_b = (Balaenidae*)findCarrier(GREEN_FACTION);

        {
            runonce {
                _b->goTo(_b->getPos()+Vec3f(-1000,0,0));
                _b->enableAuto();
            }
        }

        if (_manta1)
        {
            {
                runonce {
                    _b->stop();
                    _manta1->land(_b->getPos(),_b->getForward());
                    _manta1->enableAuto();
                }
            }
        }
    }


    if (timer>1000)
    {
        SimplifiedDynamicManta *_manta1 = (SimplifiedDynamicManta*)findManta(GREEN_FACTION,FlyingStatus::ON_DECK);

        if (_manta1)
        {
            printf("Test passed OK!\n");
            endWorldModelling();
            exit(1);
        }
    }

    if (timer>8000)
    {
        // Timeout
        printf("Test failed.\n");
        endWorldModelling();
        exit(0);
    }
}

void test57()
{
    BoxIsland *nemesis = new BoxIsland(&entities);
    nemesis->setName("Nemesis");
    nemesis->setLocation(0.0f,-1.0,0.0f);
    nemesis->buildTerrainModel(space,"terrain/thermopilae.bmp");

    islands.push_back(nemesis);

    // Entities will be added later in time.
    Beluga *_b = new Beluga(BLUE_FACTION);
    _b->init();
    _b->embody(world,space);
    _b->setPos(0.0f,20.5f,-16000.0f);
    _b->stop();

    entities.push_back(_b, _b->getGeom());

    Structure *t1 = islands[0]->addStructure(new CommandCenter(GREEN_FACTION, FACTORY_ISLAND)    ,       800.0f,    -100.0f,0,world);
    //Structure *t2 = islands[0]->addStructure(new Runway(GREEN_FACTION)           ,         0.0f,    -650.0f,-PI/4,world);
    Structure *t3 = islands[0]->addStructure(new Warehouse(GREEN_FACTION)      ,         0.0f,    650.0f,0,world);
    Structure *t4 = islands[0]->addStructure(new Warehouse(GREEN_FACTION)        ,       100.0f,    -650.0f,0,world);
    Structure *t5 = islands[0]->addStructure(new Radar(GREEN_FACTION)        ,        20.0f,    80.0f,0,world);
    Structure *t6 = islands[0]->addStructure(new Dock(GREEN_FACTION)             ,         -0,    -1700,0,world);
    Structure *t7 = islands[0]->addStructure(new Factory(GREEN_FACTION)        ,         0.0f,    1000.0f,0,world);
    Structure *t8 = islands[0]->addStructure(new Antenna(GREEN_FACTION)        ,         -1000.0f,    230.0f,0,world);

    Vec3f pos(0.0,1.32, - 60);
    Camera.setPos(pos);

    aiplayer = BLUE_AI;
    controller.faction = BOTH_FACTION;
}

void checktest57(unsigned long timer)
{
    long unsigned starttime = 150;

    if (timer == starttime)
    {
        char msg[256];
        Message mg;
        sprintf(msg, "TC57: Check landing after the command center is destroyed.");
        mg.faction = BOTH_FACTION;
        mg.msg = std::string(msg);
        messages.insert(messages.begin(), mg);
    }

    if (timer > 2000)
    {
        Balaenidae *_b = (Balaenidae*)findCarrier(BLUE_FACTION);

        {
            runonce {
                _b->goTo(_b->getPos()+Vec3f(-1000,0,0));
                _b->enableAuto();
            }
        }

    }

    if (timer == starttime + 3300)
    {
        CommandCenter *c = (CommandCenter*)findIslandByName("Nemesis")->getCommandCenter();

        if (c)
        {
            // This is in case the missile do not destroy the command center.
            c->damage(10000);
        }
    }

    if (timer > starttime + 15400)
    {
        Manta *m = (Manta*)findManta(BLUE_FACTION);

        if (m)
        {
            printf("Test failed: Manta should have not been launched.\n");
            endWorldModelling();
            exit(0);
        } else
        {
            printf("Test passed OK!\n");
            endWorldModelling();
            exit(1);
        }
    }

}

void test58()
{
    srand (time(NULL));

    BoxIsland *nemesis = new BoxIsland(&entities);
    nemesis->setName("Tristan-Da-Cunha");
    nemesis->setLocation(0.0f,-1.0,0.0f);
    nemesis->buildTerrainModel(space,"terrain/tristan.bmp");

    islands.push_back(nemesis);

    // Entities will be added later in time.
    Beluga *_b = new Beluga(BLUE_FACTION);
    _b->init();
    _b->embody(world,space);
    _b->setPos(getRandomCircularSpot(Vec3f(0.0f,20.5f,0),16000.0f));
    _b->stop();

    entities.push_back(_b, _b->getGeom());

    Vec3f pos(0.0,1.32, - 60);
    Camera.setPos(pos);

    aiplayer = BLUE_AI;
    controller.faction = BOTH_FACTION;


}

void checktest58(unsigned int timer)
{
    long unsigned starttime = 150;


    if (timer > starttime + 8000)
    {
        Structure *t = islands[0]->getCommandCenter();
        if (t)
        {
            printf("Test failed: It seems like the island has not been conquered.\n");
            endWorldModelling();
            exit(0);
        } else {
            printf("Test passed OK!\n");
            endWorldModelling();
            exit(1);
        }
    }



    if (timer > starttime + 20000)
    {
        printf("Test failed: It seems like Walrus has been destroyed.\n");
        endWorldModelling();
        exit(0);
    }
}

void test81()
{
    srand (time(NULL));

    BoxIsland *nemesis = new BoxIsland(&entities);
    nemesis->setName("Baltimore");
    nemesis->setLocation(0.0f,-1.0,0.0f);
    nemesis->buildTerrainModel(space,"terrain/tristan.bmp");

    islands.push_back(nemesis);

    // Entities will be added later in time.
    Balaenidae *_b = new Balaenidae(GREEN_FACTION);
    _b->init();
    _b->embody(world,space);
    _b->setPos(getRandomCircularSpot(Vec3f(0.0f,20.5f,0),16000.0f));
    _b->stop();

    entities.push_back(_b, _b->getGeom());

    Vec3f pos(0.0,1.32, - 60);
    Camera.setPos(pos);

    aiplayer = GREEN_AI;
    controller.faction = BOTH_FACTION;


}

void checktest81(unsigned int timer)
{
    long unsigned starttime = 150;


    if (timer > starttime + 8000)
    {
        Structure *t = islands[0]->getCommandCenter();
        if (t)
        {
            Walrus *w = (Walrus*)findWalrus(GREEN_FACTION);

            if (w)
            {
                if (w->getHealth()<1000)
                {
                    printf("Test failed: It seems like Walrus is stumbled.\n");
                    endWorldModelling();
                    exit(0);
                } else {
                    printf("Test passed OK!\n");
                    endWorldModelling();
                    exit(1);
                }
            }
        }
    }



    if (timer > starttime + 20000)
    {
        printf("Test failed: It seems like Walrus has been destroyed.\n");
        endWorldModelling();
        exit(0);
    }
}


void test59()
{
    BoxIsland *nemesis = new BoxIsland(&entities);
    nemesis->setName("Nemesis");
    nemesis->setLocation(0.0f,-1.0,0.0f);
    nemesis->buildTerrainModel(space,"terrain/sentinel.bmp");

    islands.push_back(nemesis);

    // Entities will be added later in time.
    Balaenidae *_b = new Balaenidae(GREEN_FACTION);
    _b->init();
    _b->embody(world,space);
    _b->setPos(0.0f,20.5f,-4000.0f);
    _b->stop();

    entities.push_back(_b, _b->getGeom());

    Structure *t1 = islands[0]->addStructure(new CommandCenter(GREEN_FACTION, LOGISTICS_ISLAND)    ,       800.0f,    -100.0f,0,world);

    Vec3f pos(0.0,1.32, - 60);
    Camera.setPos(pos);

    aiplayer = BLUE_AI;
    controller.faction = BOTH_FACTION;
}

void checktest59(unsigned long timer)
{
    long unsigned starttime = 200;

    if (timer == starttime)
    {
        char msg[256];
        Message mg;
        sprintf(msg, "Check visually the structures (cannot do that from a unit test).");
        mg.faction = BOTH_FACTION;
        mg.msg = std::string(msg);
        messages.insert(messages.begin(), mg);
    }

    if (timer == starttime + 100)
    {
        Structure *t2 = islands[0]->addStructureAtDesiredHeight(new Dock(GREEN_FACTION), world, 0);
    }



    if (timer > starttime + 3000)
    {
        Structure* s = findStructureFromIsland(islands[0], VehicleSubTypes::DOCK);

        if (s)
        {
            if (s->getPos()[1]!=0)
            {
                printf("Test failed. The dock is not in position.\n");
                endWorldModelling();
                exit(0);

            } else {
                printf("Test passed OK!\n");
                endWorldModelling();
                exit(1);
            }
        }
    }
}

void test60()
{
    BoxIsland *nemesis = new BoxIsland(&entities);
    nemesis->setName("Atom");
    nemesis->setLocation(0.0f,-1.0,0.0f);
    nemesis->buildTerrainModel(space,"terrain/atom.bmp");

    islands.push_back(nemesis);

    // Entities will be added later in time.
    Beluga *_b = new Beluga(BLUE_FACTION);
    _b->init();
    _b->embody(world,space);
    _b->setPos(0.0f,20.5f,-4600.0f);
    _b->stop();

    entities.push_back(_b, _b->getGeom());

    Walrus *_walrus = new Walrus(BLUE_FACTION);

    _walrus->init();
    _walrus->embody(world, space);
    _walrus->setPos(0.0f,20.5f,-5000.0f);
    _walrus->setStatus(SailingStatus::SAILING);

    size_t idx = entities.push_back(_walrus, _walrus->getGeom());

    Vec3f pos(0.0,1.32, - 3500);
    Camera.setPos(pos);

    aiplayer = BLUE_AI;
    controller.faction = BOTH_FACTION;

    controller.controllingid = idx;
}

void checktest60(unsigned int timer)
{
    long unsigned starttime = 150;
    static long unsigned steptime1 = -1;


    if (timer > starttime + 1000 && steptime1==-1)
    {
        Structure *t = islands[0]->getCommandCenter();
        if (t)
        {
            steptime1 = timer;
        }
    }


    if (steptime1>0 && timer > steptime1 + 500)
    {
        Structure *t = islands[0]->getCommandCenter();
        if (t)
        {
            Walrus *w = (Walrus*)findWalrus(BLUE_FACTION);

            if (w)
            {
                if (w->getHealth()<1000)
                {
                    printf("Test failed: It seems like Walrus has stumbled.\n");
                    endWorldModelling();
                    exit(0);
                } else {
                    printf("Test passed OK!\n");
                    endWorldModelling();
                    exit(1);
                }
            }
        }
    }
}



void test61()
{
    BoxIsland *nemesis = new BoxIsland(&entities);
    nemesis->setName("North Sentinel");
    nemesis->setLocation(0.0f,-1.0,0.0f);
    nemesis->buildTerrainModel(space,"terrain/baltimore.bmp");  //sentinel

    islands.push_back(nemesis);

    // Entities will be added later in time.
    Beluga *_b = new Beluga(BLUE_FACTION);
    _b->init();
    _b->embody(world,space);
    _b->setPos(0.0f,20.5f,-4600.0f);
    _b->stop();

    entities.push_back(_b, _b->getGeom());

    Vec3f pos(0.0,1.32, - 3500);
    Camera.setPos(pos);

    aiplayer = BLUE_AI;
    controller.faction = BOTH_FACTION;

}

void checktest61(unsigned int timer)
{
    long unsigned starttime = 150;
    static long unsigned steptime1 = -1;


    if (timer > starttime + 1000 && steptime1==-1)
    {
        Structure *t = islands[0]->getCommandCenter();
        if (t)
        {
            steptime1 = timer;
        }
    }


    if (steptime1>0 && timer > steptime1 + 500)
    {
        Structure *t = islands[0]->getCommandCenter();
        if (t)
        {
            Cephalopod *c = (Cephalopod*)findMantaByOrder(BLUE_FACTION, CONQUEST_ISLAND);

            if (c)
            {
                printf("Test passed OK!\n");
                endWorldModelling();
                exit(1);
            }
        }
    }

    if (timer>20000)
    {
        printf("Test failed: For some reason, Cephalopod didn't return.\n");
        endWorldModelling();
        exit(0);
    }
}



void test62()
{
    BoxIsland *nemesis = new BoxIsland(&entities);
    nemesis->setName("North Sentinel");
    nemesis->setLocation(0.0f,-1.0,0.0f);
    nemesis->buildTerrainModel(space,"terrain/sentinel.bmp");

    islands.push_back(nemesis);

    // Entities will be added later in time.
    Beluga *_b = new Beluga(BLUE_FACTION);
    _b->init();
    _b->embody(world,space);
    _b->setPos(0.0f,20.5f,-4600.0f);
    _b->stop();
    _b->damage(-19000);

    entities.push_back(_b, _b->getGeom());




    // Entities will be added later in time.
    _b = new Beluga(BLUE_FACTION);
    _b->init();
    _b->embody(world,space);
    _b->setPos(1000.0f,20.5f,-4600.0f);
    _b->stop();
    _b->damage(-19000);

    entities.push_back(_b, _b->getGeom());


    // Entities will be added later in time.
    Balaenidae *_b2 = new Balaenidae(GREEN_FACTION);
    _b2->init();
    _b2->embody(world,space);
    _b2->setPos(-5000.0f,20.5,-4600.0f);
    _b2->stop();
    _b2->damage(-9000);

    entities.push_back(_b2, _b2->getGeom());

    Vec3f pos(0.0,1.32, - 3500);
    Camera.setPos(pos);

    aiplayer = BOTH_AI;
    controller.faction = BOTH_FACTION;

}

void checktest62(unsigned int timer)
{
    static std::ofstream fpsfile;
    if (timer == 1)
    {
        fpsfile.open ("fps.dat");
    }

    // Elapsedtime is the time taken for ODE to process one step
    fpsfile << entities.size() << "," <<  fps << "," << elapsedtime << std::endl;
    fpsfile.flush();

    long unsigned starttime = 150;
    static long unsigned steptime1 = -1;

    if (timer % 50 == 0 && timer > 200 && timer < 4000)
    {
        if (getRandomInteger(1,2)==1)
        {
            Vehicle *b = findCarrier(BLUE_FACTION);

            if (b)
            {
            SimplifiedDynamicManta *_manta1 = new AdvancedManta(GREEN_FACTION);

            _manta1->init();
            _manta1->embody(world, space);
            _manta1->setPos(getRandomInteger(1,9)*1000.0,2000.5f,-2000.0f);
            _manta1->setStatus(0);
            _manta1->inert = false;
            _manta1->setStatus(FlyingStatus::FLYING);
            _manta1->elevator = +12;
            struct controlregister c;
            c.thrust = 1000.0f/(10.0);
            c.pitch = 12;
            _manta1->setControlRegisters(c);
            _manta1->setThrottle(1000.0f);
            _manta1->enableAuto();
            _manta1->attack(b->getPos());
            _manta1->setOrder(ATTACK_ISLAND);
            //_manta1->setNameByNumber(findNextNumber(GREEN_FACTION,VehicleTypes::MANTA,VehicleSubTypes::SIMPLEMANTA));
            _manta1->setNameByNumber(1);

            entities.push_back(_manta1, _manta1->getGeom());
            }
        } else
        {
            Vehicle *b = findCarrier(GREEN_FACTION);

            if (b)
            {
            SimplifiedDynamicManta *_manta1 = new AdvancedManta(BLUE_FACTION);

            _manta1->init();
            _manta1->embody(world, space);
            _manta1->setPos(0.0f,2000.5f,getRandomInteger(1,9)*1000.0);
            _manta1->setStatus(0);
            _manta1->inert = false;
            _manta1->setStatus(FlyingStatus::FLYING);
            _manta1->elevator = +12;
            struct controlregister c;
            c.thrust = 1000.0f/(10.0);
            c.pitch = 12;
            _manta1->setControlRegisters(c);
            _manta1->setThrottle(1000.0f);
            _manta1->enableAuto();
            _manta1->attack(b->getPos());
            _manta1->setOrder(ATTACK_ISLAND);
            _manta1->setNameByNumber(1);

            entities.push_back(_manta1, _manta1->getGeom());

            }
        }
    }

    for(size_t i=entities.first();entities.hasMore(i);i=entities.next(i))
    {
        if (entities[i]->getOrder() == ATTACK_ISLAND)
        {
            Manta *m = (Manta*)entities[i];
            Vehicle *b = findNearestEnemyVehicle(entities[i]->getFaction(),entities[i]->getPos(),5000.0);
            if (b)
            {
                if (b->getStatus() == VehicleTypes::MANTA)
                    m->dogfight(b->getPos());
                else
                    m->attack(b->getPos());
            }
        }
    }

    if (timer > 10000)
    {
        printf("Test passed OK!\n");
        endWorldModelling();
        exit(1);
    }

}

void test63()
{
    // Entities will be added later in time.
    Balaenidae *_b = new Balaenidae(GREEN_FACTION);
    _b->init();
    _b->embody(world,space);
    _b->setPos(0.0f,20.5f,-17000.0f);
    _b->stop();

    entities.push_back(_b, _b->getGeom());

    Vec3f Pf = _b->getPos();
    Vec3f attitude = _b->getForward();
    Vec3f trans = attitude.rotateOnY(PI/2.0);

    Vec3f w[5];
    w[4] = (Pf);
    w[3] = (Pf - attitude.normalize()*(05 kmf)) ;
    w[2] = (Pf - attitude.normalize()*(10 kmf)) ;
    w[1] = (Pf - (attitude.normalize()*(20 kmf) + trans.normalize()*(5 kmf))) ;
    w[0] = (Pf - (attitude.normalize()*(10 kmf) + trans.normalize()*(10 kmf))) ;

    for(int i=0;i<5;i++)
    {
        BoxVehicle * _bo= new BoxVehicle();
        _bo->init();
        _bo->embody(world, space);
        w[i][1] = 250;
        _bo->setPos(w[i]);
        _bo->antigravity(_bo->getBodyID());
        _bo->stop();

        entities.push_back(_bo, _bo->getGeom());
    }


    BoxIsland *nemesis = new BoxIsland(&entities);
    nemesis->setName("Nemesis");
    nemesis->setLocation(0.0f,-1.0,-0.0f);
    nemesis->buildTerrainModel(space,"terrain/thermopilae.bmp");

    islands.push_back(nemesis);

    Structure *t1 = islands[0]->addStructure(new CommandCenter(GREEN_FACTION, LOGISTICS_ISLAND)    ,       800.0f,    -100.0f,0,world);
    Structure *t8 = islands[0]->addStructure(new Launcher(GREEN_FACTION)        ,         -230.0f,    230.0f,0,world);
    Structure *t9 = islands[0]->addStructure(new Warehouse(GREEN_FACTION)        ,           -330.0f,    230.0f,0,world);
    Structure *t2 = islands[0]->addStructure(new Warehouse(GREEN_FACTION),world);
    Structure *t10 = islands[0]->addStructure(new Warehouse(GREEN_FACTION),world);
    Structure *t11 = islands[0]->addStructure(new Warehouse(GREEN_FACTION),world);
    Structure *t12 = islands[0]->addStructure(new Warehouse(GREEN_FACTION),world);
    Structure *t13 = islands[0]->addStructure(new Warehouse(GREEN_FACTION),world);
    Structure *t14 = islands[0]->addStructure(new Warehouse(GREEN_FACTION),world);
    Structure *t15 = islands[0]->addStructure(new Warehouse(GREEN_FACTION),world);
    Structure *t16 = islands[0]->addStructure(new Warehouse(GREEN_FACTION),world);
    Structure *t17 = islands[0]->addStructure(new Warehouse(GREEN_FACTION),world);

    Vec3f pos(0.0,1.32, - 3500);
    Camera.setPos(pos);

    controller.faction = BOTH_FACTION;
}

void checktest63(unsigned int timer)
{
    Vehicle *b = findCarrier(GREEN_FACTION);

    long unsigned starttime = 200;
    static Vehicle *manta = NULL;

    if (timer == starttime)
    {
        char msg[256];
        Message mg;
        sprintf(msg, "TC63: Order Manta to follow a path using waypoints.");
        mg.faction = BOTH_FACTION;
        mg.msg = std::string(msg);
        messages.insert(messages.begin(), mg);
    }

    if (timer == 500)
    {
        size_t idx = 0;
        Vehicle *m = spawnManta(space,world,b,idx);

        manta = m;
    }

    if (timer == 800)
    {
        // launch
        launchManta(b);
    }

    if (timer == 1000)
    {
        Vec3f w1 = Vec3f(0,1000.0f, 15000.0f);
        Vec3f w2 = Vec3f(30000,10000.0,0.0f);
        Vec3f w3 = Vec3f(-30000.0f, 300.0f, 0.0f);

        manta->addWaypoint(w1);
        manta->addWaypoint(w2);
        manta->addWaypoint(w3);
        manta->goWaypoints();
        manta->enableAuto();

    }

    if (timer == 2000)
    {
        landManta(b,(Manta*)manta);
    }
}

void test64()
{
    // Entities will be added later in time.
    Balaenidae *_b = new Balaenidae(GREEN_FACTION);
    _b->init();
    _b->embody(world,space);
    _b->setPos(0.0f,20.5f,-17000.0f);
    _b->stop();

    entities.push_back(_b, _b->getGeom());

    BoxIsland *nemesis = new BoxIsland(&entities);
    nemesis->setName("Nemesis");
    nemesis->setLocation(0.0f,-1.0,-0.0f);
    nemesis->buildTerrainModel(space,"terrain/thermopilae.bmp");

    islands.push_back(nemesis);

    Structure *t1 = islands[0]->addStructure(new CommandCenter(BLUE_FACTION, LOGISTICS_ISLAND)    ,       800.0f,    -100.0f,0,world);
    Structure *t8 = islands[0]->addStructure(new Launcher(BLUE_FACTION)        ,         -230.0f,    230.0f,0,world);
    Structure *t9 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)        ,           -330.0f,    230.0f,0,world);
    Structure *t2 = islands[0]->addStructure(new Warehouse(BLUE_FACTION),world);
    Structure *t10 = islands[0]->addStructure(new Warehouse(BLUE_FACTION),world);
    Structure *t11 = islands[0]->addStructure(new Warehouse(BLUE_FACTION),world);
    Structure *t12 = islands[0]->addStructure(new Warehouse(BLUE_FACTION),world);
    Structure *t13 = islands[0]->addStructure(new Warehouse(BLUE_FACTION),world);
    Structure *t14 = islands[0]->addStructure(new Warehouse(BLUE_FACTION),world);
    Structure *t15 = islands[0]->addStructure(new Warehouse(BLUE_FACTION),world);
    Structure *t16 = islands[0]->addStructure(new Warehouse(BLUE_FACTION),world);
    Structure *t17 = islands[0]->addStructure(new Warehouse(BLUE_FACTION),world);

    Vec3f pos(0.0,1.32, - 3500);
    Camera.setPos(pos);

    aiplayer = BOTH_AI;
    controller.faction = BOTH_FACTION;
}


void checktest64(unsigned long timer)
{
    static bool isMantaPresent = false;

    long unsigned starttime = 200;

    if (timer == starttime)
    {
        char msg[256];
        Message mg;
        sprintf(msg, "TC64: Verifying launcher destroys Mantas by missiles.");
        mg.faction = BOTH_FACTION;
        mg.msg = std::string(msg);
        messages.insert(messages.begin(), mg);
    }

    if (timer > 1000 && !isMantaPresent)
    {
        Vehicle *v = findManta(GREEN_FACTION);

        if (v)
        {
            isMantaPresent = true;
        }
    }

    if (timer > 2000 && isMantaPresent)
    {
        Vehicle *v = findManta(GREEN_FACTION);

        if (!v)
        {
            printf("Test Passed\n");
            endWorldModelling();
            exit(1);
        }
    }

    if (timer > 6000)
    {
        printf("Test Failed:  Manta should have been hit by a Missile and got destroyed.\n");
        endWorldModelling();
        exit(0);
    }


}


void test65()
{
    // Entities will be added later in time.
    Balaenidae *_b = new Balaenidae(GREEN_FACTION);
    _b->init();
    _b->embody(world,space);
    _b->setPos(0.0f,20.5f,-17000.0f);
    _b->stop();

    entities.push_back(_b, _b->getGeom());

    static dJointID joint;

    Weapon * _bo= new Weapon(GREEN_FACTION);
    _bo->init();
    _bo->embody(world, space);
    _bo->attachTo(world,_b, 00.0f, 22.0f, 0.0f);
    _bo->stop();

    entities.push_back(_bo, _bo->getGeom());


    BoxIsland *nemesis = new BoxIsland(&entities);
    nemesis->setName("Nemesis");
    nemesis->setLocation(0.0f,-1.0,-0.0f);
    nemesis->buildTerrainModel(space,"terrain/thermopilae.bmp");

    islands.push_back(nemesis);

    Structure *t1 = islands[0]->addStructure(new CommandCenter(BLUE_FACTION, LOGISTICS_ISLAND)    ,       800.0f,    -100.0f,0,world);
    Structure *t8 = islands[0]->addStructure(new Launcher(BLUE_FACTION)        ,         -230.0f,    230.0f,0,world);
    Structure *t9 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)        ,           -330.0f,    230.0f,0,world);
    Structure *t2 = islands[0]->addStructure(new Warehouse(BLUE_FACTION),world);
    Structure *t10 = islands[0]->addStructure(new Warehouse(BLUE_FACTION),world);
    Structure *t11 = islands[0]->addStructure(new Warehouse(BLUE_FACTION),world);
    Structure *t12 = islands[0]->addStructure(new Warehouse(BLUE_FACTION),world);
    Structure *t13 = islands[0]->addStructure(new Warehouse(BLUE_FACTION),world);
    Structure *t14 = islands[0]->addStructure(new Warehouse(BLUE_FACTION),world);
    Structure *t15 = islands[0]->addStructure(new Warehouse(BLUE_FACTION),world);
    Structure *t16 = islands[0]->addStructure(new Warehouse(BLUE_FACTION),world);
    Structure *t17 = islands[0]->addStructure(new Warehouse(BLUE_FACTION),world);

    Vec3f pos(0.0,1.32, - 3500);
    Camera.setPos(pos);

    //aiplayer = BOTH_AI;
    controller.faction = BOTH_FACTION;
}

void checktest65(unsigned long timer)
{


}


void test66()
{
    // Entities will be added later in time.
    Balaenidae *_b = new Balaenidae(GREEN_FACTION);
    _b->init();
    _b->embody(world,space);
    _b->setPos(0.0f,20.5f,-17000.0f);
    _b->stop();

    entities.push_back(_b, _b->getGeom());

    CarrierTurret * _bo= new CarrierTurret(GREEN_FACTION);
    _bo->init();
    _bo->embody(world, space);
    _bo->attachTo(world,_b, -40.0f, 20.0f + 5, -210.0f);
    _bo->stop();

    entities.push_back(_bo, _bo->getGeom());


    CarrierArtillery * _w1= new CarrierArtillery(GREEN_FACTION);
    _w1->init();
    _w1->embody(world, space);
    _w1->attachTo(world,_b, -40.0, 27.0f, +210.0f);
    _w1->stop();

    entities.push_back(_w1, _w1->getGeom());


    BoxIsland *nemesis = new BoxIsland(&entities);
    nemesis->setName("Nemesis");
    nemesis->setLocation(0.0f,-1.0,-0.0f);
    nemesis->buildTerrainModel(space,"terrain/thermopilae.bmp");

    islands.push_back(nemesis);

    Structure *t1 = islands[0]->addStructure(new CommandCenter(BLUE_FACTION, LOGISTICS_ISLAND)    ,       800.0f,    -100.0f,0,world);
    Structure *t8 = islands[0]->addStructure(new Launcher(BLUE_FACTION)        ,         -230.0f,    230.0f,0,world);
    Structure *t9 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)        ,           -330.0f,    230.0f,0,world);
    Structure *t2 = islands[0]->addStructure(new Warehouse(BLUE_FACTION),world);
    Structure *t10 = islands[0]->addStructure(new Warehouse(BLUE_FACTION),world);
    Structure *t11 = islands[0]->addStructure(new Warehouse(BLUE_FACTION),world);
    Structure *t12 = islands[0]->addStructure(new Warehouse(BLUE_FACTION),world);
    Structure *t13 = islands[0]->addStructure(new Warehouse(BLUE_FACTION),world);
    Structure *t14 = islands[0]->addStructure(new Warehouse(BLUE_FACTION),world);
    Structure *t15 = islands[0]->addStructure(new Warehouse(BLUE_FACTION),world);
    Structure *t16 = islands[0]->addStructure(new Warehouse(BLUE_FACTION),world);
    Structure *t17 = islands[0]->addStructure(new Warehouse(BLUE_FACTION),world);

    Vec3f pos(0.0,1.32, - 3500);
    Camera.setPos(pos);

    //aiplayer = BOTH_AI;
    controller.faction = BOTH_FACTION;
}

void checktest66(unsigned long timer)
{
    long unsigned starttime = 200;

    if (timer == starttime)
    {
        char msg[256];
        Message mg;
        sprintf(msg, "TC66: Carrier approaches an enemy island and the carrier shoots the CC.");
        mg.faction = BOTH_FACTION;
        mg.msg = std::string(msg);
        messages.insert(messages.begin(), mg);
    }

    if (timer == 1000)
    {
        Vehicle *c = findCarrier(GREEN_FACTION);
        BoxIsland *b = findNearestIsland(c->getPos());

        c->goTo(b->getPos()+Vec3f(1900,0.0,0.0));
        c->enableAuto();
    }

    if (timer > 1400)
    {
        Vehicle *c = findCarrier(GREEN_FACTION);
        BoxIsland *b = findNearestIsland(c->getPos());

        if (c)

            for(size_t i=entities.first();entities.hasMore(i);i=entities.next(i))
            {
                Vehicle *v=entities[i];
                if (v->getType() == WEAPON && v->getSubType() == TURRET && v->getFaction() == GREEN_FACTION)
                {
                    CarrierTurret *w = (CarrierTurret*)v;
                    if (dAreConnected(c->getBodyID(),v->getBodyID()))
                    {
                        CommandCenter *cm = (CommandCenter*)b->getCommandCenter();
                        if (cm)
                        {

                            Vehicle *action = w->aimAndFire(world,space, cm->getPos());

                            if (action != NULL)
                            {
                                entities.push_back(action, action->getGeom());
                                gunshot();
                                //cm->setTtl(100);
                            }

                        } else {
                            printf("Test Passed\n");
                            endWorldModelling();
                            exit(1);
                        }
                    }
                }
            }

    }

    if (timer > 6000)
    {
        printf("Test Failed:  The carrier should have destroyed so far the Enemy Command Center.\n");
        endWorldModelling();
        exit(0);
    }
}

void test67()
{
    // Entities will be added later in time.
    Balaenidae *_b = new Balaenidae(GREEN_FACTION);
    _b->init();
    _b->embody(world,space);
    _b->setPos(0.0f,20.5f,-4000.0f);
    _b->stop();

    entities.push_back(_b, _b->getGeom());

    BoxIsland *nemesis = new BoxIsland(&entities);
    nemesis->setName("Nemesis");
    nemesis->setLocation(0.0f,-1.0,-0.0f);
    nemesis->buildTerrainModel(space,"terrain/atom.bmp");

    islands.push_back(nemesis);


    BoxIsland *thermopilae = new BoxIsland(&entities);
    thermopilae->setName("Thermopilae");
    thermopilae->setLocation(3100.0f,-1.0,-0.0f);
    thermopilae->buildTerrainModel(space,"terrain/gaijin.bmp");

    islands.push_back(thermopilae);


    Walrus *_walrus = new Walrus(GREEN_FACTION);

    _walrus->init();
    _walrus->embody(world, space);
    _walrus->setPos(0.0f,20.5f,2000);
    _walrus->setStatus(SailingStatus::SAILING);
    _walrus->setSignal(4);

    size_t idx = entities.push_back(_walrus, _walrus->getGeom());


    Vec3f pos(0.0,1.32, - 3500);
    Camera.setPos(pos);

    //aiplayer = BOTH_AI;
    controller.faction = BOTH_FACTION;


}

void checktest67(unsigned long timer)
{
    if (timer == 1000)
    {
        printf("Test Passed\n");
        endWorldModelling();
        exit(1);
    }
}

void test68()
{
    // Entities will be added later in time.
    Balaenidae *_b = new Balaenidae(GREEN_FACTION);
    _b->init();
    _b->embody(world,space);
    _b->setPos(0.0f,20.5f,-4000.0f);
    _b->stop();

    entities.push_back(_b, _b->getGeom());

    BoxIsland *nemesis = new BoxIsland(&entities);
    nemesis->setName("Nemesis");
    nemesis->setLocation(0.0f,-1.0,-0.0f);
    nemesis->buildTerrainModel(space,"terrain/nonsquareisland.bmp");

    islands.push_back(nemesis);


    BoxIsland *thermopilae = new BoxIsland(&entities);
    thermopilae->setName("Thermopilae");
    thermopilae->setLocation(3100.0f,-1.0,-0.0f);
    thermopilae->buildTerrainModel(space,"terrain/gaijin.bmp");

    islands.push_back(thermopilae);

    Structure *t1 = islands[0]->addStructure(new CommandCenter(GREEN_FACTION, LOGISTICS_ISLAND)    ,       800.0f,    -100.0f,0,world);
    Structure *t2 = islands[0]->addStructure(new Runway(GREEN_FACTION),                                    200.0f,     200.0f,33,world);

    Vec3f pos(0.0,1.32, - 3500);
    Camera.setPos(pos);

    //aiplayer = BOTH_AI;
    controller.faction = BOTH_FACTION;
}

void checktest68(unsigned long timer)
{
    if (timer == 10000)
    {
        printf("Test Passed\n");
        endWorldModelling();
        exit(1);
    }
}

void test69()
{
    // Entities will be added later in time.
    Balaenidae *_b = new Balaenidae(GREEN_FACTION);
    _b->init();
    _b->embody(world,space);
    _b->setPos(0.0f,20.5f,-4000.0f);
    _b->stop();

    entities.push_back(_b, _b->getGeom());

    BoxIsland *nemesis = new BoxIsland(&entities);
    nemesis->setName("Nemesis");
    nemesis->setLocation(0.0f,-1.0,-0.0f);
    nemesis->buildTerrainModel(space,"terrain/goku.bmp");

    islands.push_back(nemesis);


    BoxIsland *thermopilae = new BoxIsland(&entities);
    thermopilae->setName("Thermopilae");
    thermopilae->setLocation(3100.0f,-1.0,-0.0f);
    thermopilae->buildTerrainModel(space,"terrain/gaijin.bmp");

    islands.push_back(thermopilae);

    Structure *t1 = islands[0]->addStructure(new CommandCenter(GREEN_FACTION, LOGISTICS_ISLAND)    ,       800.0f,    -100.0f,0,world);
    Structure *t2 = islands[0]->addStructure(new Runway(GREEN_FACTION),                                    200.0f,     200.0f,0,world);


    // 6,3,12
    Otter *_otter = new Otter(GREEN_FACTION);
    _otter->init();
    dSpaceID car_space = _otter->embody_in_space(world, space);
    _otter->setPos(400.0f,70.0f,-4400.0f);
    _otter->setPos(0.0f,70.0f,-0.0f);
    _otter->stop();
    _otter->setSignal(4);
    _otter->setNameByNumber(1);
    _otter->setStatus(SailingStatus::SAILING);

    Vec3f dimensions(5.0f,4.0f,10.0f);

    entities.push_back(_otter, _otter->getGeom());



    Wheel * _fr= new Wheel(GREEN_FACTION, 0.001, 30.0);
    _fr->init();
    _fr->embody(world, car_space);
    _fr->attachTo(world,_otter,4.9f, -3.0, 5.8);
    _fr->stop();

    entities.push_back(_fr, _fr->getGeom());


    Wheel * _fl= new Wheel(GREEN_FACTION, 0.001, 30.0);
    _fl->init();
    _fl->embody(world, car_space);
    _fl->attachTo(world,_otter, -4.9f, -3.0, 5.8);
    _fl->stop();

    entities.push_back(_fl, _fl->getGeom());


    Wheel * _br= new Wheel(GREEN_FACTION, 0.001, 30.0);
    _br->init();
    _br->embody(world, car_space);
    _br->attachTo(world,_otter, 4.9f, -3.0, -5.8);
    _br->stop();

    entities.push_back(_br, _br->getGeom());


    Wheel * _bl= new Wheel(GREEN_FACTION, 0.001, 30.0);
    _bl->init();
    _bl->embody(world, car_space);
    _bl->attachTo(world,_otter, -4.9f, -3.0, -5.8);
    _bl->stop();

    entities.push_back(_bl, _bl->getGeom());

    _otter->addWheels(_fl, _fr, _bl, _br);

    _fl->setSteering(true);
    _fr->setSteering(true);


    Walrus *_walrus = new Walrus(GREEN_FACTION);

    _walrus->init();
    _walrus->embody(world, space);
    _walrus->setPos(0.0f,20.5f,2000);
    _walrus->setStatus(SailingStatus::SAILING);
    _walrus->setSignal(4);

    size_t idx = entities.push_back(_walrus, _walrus->getGeom());


    Vec3f pos(0.0,1.32, - 3500);
    Camera.setPos(pos);

    //aiplayer = BOTH_AI;
    controller.faction = BOTH_FACTION;
}

void checktest69(unsigned long timer)
{
    long unsigned starttime = 200;

    if (timer == starttime)
    {
        char msg[256];
        Message mg;
        sprintf(msg, "TC69: Testing the controllers of a wheeled Walrus.");
        mg.faction = BOTH_FACTION;
        mg.msg = std::string(msg);
        messages.insert(messages.begin(), mg);
    }

    if (timer == 10000)
    {
        printf("Test Passed\n");
        endWorldModelling();
        exit(1);
    }
}


void test70()
{
    // Entities will be added later in time.
    Balaenidae *_b = new Balaenidae(GREEN_FACTION);
    _b->init();
    _b->embody(world,space);
    _b->setPos(0.0f,20.5f,-4000.0f);
    _b->stop();

    entities.push_back(_b, _b->getGeom());

    BoxIsland *nemesis = new BoxIsland(&entities);
    nemesis->setName("Nemesis");
    nemesis->setLocation(0.0f,-1.0,-0.0f);
    nemesis->buildTerrainModel(space,"terrain/goku.bmp");

    islands.push_back(nemesis);


    BoxIsland *thermopilae = new BoxIsland(&entities);
    thermopilae->setName("Thermopilae");
    thermopilae->setLocation(3100.0f,-1.0,-0.0f);
    thermopilae->buildTerrainModel(space,"terrain/gaijin.bmp");

    islands.push_back(thermopilae);

    Structure *t1 = islands[0]->addStructure(new CommandCenter(GREEN_FACTION, LOGISTICS_ISLAND)    ,       800.0f,    -100.0f,0,world);
    Structure *t2 = islands[0]->addStructure(new Runway(GREEN_FACTION),                                    200.0f,     200.0f,127,world);


    Vec3f pos(0.0,1.32, - 3500);
    Camera.setPos(pos);

    //aiplayer = BOTH_AI;
    controller.faction = BOTH_FACTION;
}

void checktest70(unsigned long timer)
{
    long unsigned starttime = 200;

    if (timer == starttime)
    {
        char msg[256];
        Message mg;
        sprintf(msg, "TC70: Testing manually manta landing on islands");
        mg.faction = BOTH_FACTION;
        mg.msg = std::string(msg);
        messages.insert(messages.begin(), mg);
    }

    if (timer == 10000)
    {
        printf("Test Passed\n");
        endWorldModelling();
        exit(1);
    }
}

void test71()
{
    // Entities will be added later in time.
    Balaenidae *_b = new Balaenidae(GREEN_FACTION);
    _b->init();
    _b->embody(world,space);
    _b->setPos(0.0f,20.5f,-9000.0f);
    _b->stop();

    entities.push_back(_b, _b->getGeom());
}

void checktest71(unsigned long timer)
{
    long unsigned starttime = 200;
    static int step = 0;

    if (step==0 && timer == starttime)
    {
        char msg[256];
        Message mg;
        sprintf(msg, "TC71: Landing Manta on a moving Carrier.");
        mg.faction = BOTH_FACTION;
        mg.msg = std::string(msg);
        messages.insert(messages.begin(), mg);
    }

    if (step == 0 && timer == starttime + 100)
    {
        size_t idx = 0;
        spawnManta(space,world,entities[1],idx);
    }

    if (step==0 && timer == starttime + 320)
    {
        // launch
        launchManta(entities[1]);
    }



    if (step==0 && timer == starttime + 420)
    {
        Vehicle *_b = findManta(GREEN_FACTION,FlyingStatus::FLYING);
        SimplifiedDynamicManta *_manta1 = (SimplifiedDynamicManta*)_b;
        _manta1->inert = false;
        _manta1->enableAuto();
        _manta1->setStatus(FlyingStatus::FLYING);
        _manta1->elevator = +5;
        struct controlregister c;
        c.thrust = 400.0f/(10.0);
        c.pitch = 5;
        _manta1->setControlRegisters(c);
        _manta1->setThrottle(400.0f);
        _manta1->disableAuto();
    }

    if (step ==0 && timer == starttime + 500)
    {
        Balaenidae *_b = (Balaenidae*)findCarrier(GREEN_FACTION);
        Vec3f pos = _b->getPos();
        pos = Vec3f(pos[0]+(rand() % 10 -5 +1)*200,pos[1],pos[2]+(rand() % 10 -5 +1)*200);

        _b->setDestination(pos);
        _b->enableAuto();

    }

    if (step ==0 && timer > starttime + 500)
    {
        Balaenidae *_b = (Balaenidae*)findCarrier(GREEN_FACTION);
        if (!_b->isAuto())
        {
            step = 1;
            Vehicle *_b = findManta(GREEN_FACTION,FlyingStatus::FLYING);
            SimplifiedDynamicManta *_manta1 = (SimplifiedDynamicManta*)_b;
            Balaenidae *_c = (Balaenidae*)findCarrier(GREEN_FACTION);
            _c->stop();
            _manta1->land(_c->getPos(), _c->getForward());
        }
    }


    if (timer == 15000)
    {
        Vehicle *_b = findManta(GREEN_FACTION, FlyingStatus::ON_DECK);
        if (_b)
        {
            printf("Test Passed\n");
            endWorldModelling();
            exit(1);
        }
        else
        {
            printf("Test Failed.  Manta did not landed.\n");
            endWorldModelling();
            exit(1);
        }
    }

}

void test72()
{
    // Entities will be added later in time.
    Balaenidae *_b = new Balaenidae(GREEN_FACTION);
    _b->init();
    _b->embody(world,space);
    _b->setPos(0.0f,20.5f,-4000.0f);
    _b->stop();

    entities.push_back(_b, _b->getGeom());

    BoxIsland *nemesis = new BoxIsland(&entities);
    nemesis->setName("Nemesis");
    nemesis->setLocation(0.0f,-1.0,-0.0f);
    nemesis->buildTerrainModel(space,"terrain/goku.bmp");

    islands.push_back(nemesis);


    Structure *t1 = islands[0]->addStructure(new CommandCenter(GREEN_FACTION, LOGISTICS_ISLAND)    ,       800.0f,    -100.0f,0,world);
    Structure *t2 = islands[0]->addStructure(new Runway(GREEN_FACTION),                                    200.0f,     200.0f,127,world);


    Vec3f pos(0.0,1.32, - 3500);
    Camera.setPos(pos);

    //aiplayer = BOTH_AI;
    controller.faction = BOTH_FACTION;
}

void checktest72(unsigned long timer)
{
    if (timer == 50)
    {
        char msg[256];
        Message mg;
        sprintf(msg, "TC72: Visually Checking explosions.");
        mg.faction = BOTH_FACTION;
        mg.msg = std::string(msg);
        messages.insert(messages.begin(), mg);
    }

    if (timer == 100)
    {
        controller.controllingid = CONTROLLING_NONE;
        Vec3f pos(10.0f,10.0f,-30.0f);
        Camera.setPos(pos);
        Camera.fw = Vec3f(0.0f,0.0f,1.0f);
    }

    if (timer == 300)
    {
        Vec3f loc(10.0f, 10.0f, 10.0f);

        Explosion* b1 = new Explosion();
        b1->init();
        b1->setTexture(textures["land"]);
        b1->embody(world, space);
        b1->setPos(loc[0],loc[1],loc[2]);
        b1->stop();

        entities.push_back(b1, b1->getGeom());

        b1->expand(10,10,10,2,world, space);
    }

    if (timer == 3000)
    {
        printf("Test Passed\n");
        endWorldModelling();
        exit(1);
    }
}

void test73()
{

    Torpedo *t = new Torpedo(GREEN_FACTION);
    t->init();
    t->embody(world, space);
    t->setPos(0.0,20.0f,0.0f);
    t->stop();

    entities.push_back(t, t->getGeom());

    // Entities will be added later in time.
    Balaenidae *_b = new Balaenidae(GREEN_FACTION);
    _b->init();
    _b->embody(world,space);
    _b->setPos(0.0f,20.5f,-4000.0f);
    _b->stop();

    entities.push_back(_b, _b->getGeom());

}

void checktest73(unsigned long timer)
{
    if (timer == 50)
    {
        char msg[256];
        Message mg;
        sprintf(msg, "TC73: Testing torpedos chasing Carrier.");
        mg.faction = BOTH_FACTION;
        mg.msg = std::string(msg);
        messages.insert(messages.begin(), mg);
    }

    if (timer == 100)
    {
        Vehicle *b = findCarrier(GREEN_FACTION);
        Torpedo *t = (Torpedo*) entities[1];

        t->goTo(b->getPos());
        t->enableAuto();
    }

    if (timer == 3000)
    {
        Vehicle *t = findCarrier(GREEN_FACTION);

        if (!t)
        {
            printf("Test Passed\n");
            endWorldModelling();
            exit(1);
        }
        else
        {
            printf("Test Failed.  Carrier still around.\n");
            endWorldModelling();
            exit(1);
        }

    }
}


void test74()
{

    Torpedo *t = new Torpedo(GREEN_FACTION);
    t->init();
    t->embody(world, space);
    t->setPos(0.0,20.0f,0.0f);
    t->stop();

    entities.push_back(t, t->getGeom());

    // Entities will be added later in time.
    AdvancedWalrus *_b = new AdvancedWalrus(GREEN_FACTION);
    _b->init();
    _b->embody(world,space);
    _b->setPos(0.0f,20.5f,-4000.0f);
    _b->setSignal(4);
    _b->setNameByNumber(1);
    _b->stop();

    entities.push_back(_b, _b->getGeom());

    BoxIsland *nemesis = new BoxIsland(&entities);
    nemesis->setName("Nemesis");
    nemesis->setLocation(-8000,-1.0,-8000);
    nemesis->buildTerrainModel(space,"terrain/goku.bmp");

    islands.push_back(nemesis);

}

void checktest74(unsigned long timer)
{
    if (timer == 50)
    {
        char msg[256];
        Message mg;
        sprintf(msg, "TC73: Testing torpedos chasing Walruses.");
        mg.faction = BOTH_FACTION;
        mg.msg = std::string(msg);
        messages.insert(messages.begin(), mg);
    }

    if (timer == 80)
    {
        Vehicle *b = findWalrus(GREEN_FACTION);

        b->goTo(Vec3f(0, 0.0, -9000.0));
        b->enableAuto();
    }

    if (timer == 100)
    {
        Vehicle *b = findWalrus(GREEN_FACTION);

        if (entities.isValid(0))
        {
            Torpedo *t = (Torpedo*) entities[1];

            t->goTo(b->getPos());
            t->enableAuto();
        }
    }

    if (timer > 100)
    {
        Vehicle *b = findWalrus(GREEN_FACTION);

        if (b)
            if (entities.isValid(0))
            {
                Torpedo *t = (Torpedo*) entities[1];

                t->goTo(b->getPos());
                t->enableAuto();
            }
    }




    if (timer == 3000)
    {
        Vehicle *t = findWalrus(GREEN_FACTION);

        if (!t)
        {
            printf("Test Passed\n");
            endWorldModelling();
            exit(1);
        }
        else
        {
            printf("Test Failed.  Walrus still around.\n");
            endWorldModelling();
            exit(1);
        }

    }
}

void test75()
{

    BoxIsland *nemesis = new BoxIsland(&entities);
    nemesis->setName("Nemesis");
    nemesis->setLocation(0,-1.0,0);
    nemesis->buildTerrainModel(space,"terrain/goku.bmp");

    islands.push_back(nemesis);

    Structure *t1 = islands[0]->addStructure(new CommandCenter(GREEN_FACTION, LOGISTICS_ISLAND)    ,       20.0f,      -20.0f,  0,world);
    Structure *t2 = islands[0]->addStructure(new Runway(GREEN_FACTION),                                    200.0f,     200.0f,127,world);


    Bomb *t = new Bomb(GREEN_FACTION);
    t->init();
    t->embody(world, space);
    t->setPos(0.0,1000.0f,0.0f);
    t->stop();

    entities.push_back(t, t->getGeom());

}

void checktest75(unsigned long timer)
{

    if (timer == 50)
    {
        char msg[256];
        Message mg;
        sprintf(msg, "TC75: Testing bombs.");
        mg.faction = BOTH_FACTION;
        mg.msg = std::string(msg);
        messages.insert(messages.begin(), mg);
    }


    if (timer == 3000)
    {
        Structure *t = islands[0]->getCommandCenter();

        if (!t)
        {
            printf("Test Passed\n");
            endWorldModelling();
            exit(1);
        }
        else
        {
            printf("Test Failed.  Walrus still around.\n");
            endWorldModelling();
            exit(1);
        }

    }
}

void test76()
{
    BoxVehicle * ao= new BoxVehicle(BLUE_FACTION);
    ao->init();
    ao->embody(world, space);
    ao->setPos(0,100,0);
    ao->setSignal(4);
    ao->stop();

    entities.push_back(ao, ao->getGeom());

}

void checktest76(unsigned long timer)
{
    if (timer == 50)
    {
        char msg[256];
        Message mg;
        sprintf(msg, "TC76: Playground on radar HUD. Move and check position.");
        mg.faction = BOTH_FACTION;
        mg.msg = std::string(msg);
        messages.insert(messages.begin(), mg);
    }

}


void test77()
{
    BoxVehicle * ao= new BoxVehicle(BLUE_FACTION);
    ao->init();
    ao->embody(world, space);
    ao->setPos(0,100,0);
    ao->setSignal(4);
    ao->stop();

    entities.push_back(ao, ao->getGeom());

    Balaenidae *_b = new Balaenidae(GREEN_FACTION);
    _b->init();
    _b->embody(world,space);
    _b->setPos(0.0f,20.5f,-4000.0f);
    _b->stop();

    entities.push_back(_b, _b->getGeom());

    CarrierTurret * _bo= new CarrierTurret(GREEN_FACTION);
    _bo->init();
    _bo->embody(world, space);
    _bo->attachTo(world,_b, -40.0f, 20.0f + 5, -210.0f);
    _bo->stop();

    entities.push_back(_bo, _bo->getGeom());


    CarrierArtillery * _w1= new CarrierArtillery(GREEN_FACTION);
    _w1->init();
    _w1->embody(world, space);
    _w1->attachTo(world,_b, -40.0, 27.0f, +210.0f);
    _w1->stop();

    entities.push_back(_w1, _w1->getGeom());
}

void checktest77(unsigned long timer)
{
    if (timer == 50)
    {
        char msg[256];
        Message mg;
        sprintf(msg, "TC75: Testing radar hud with enemy units.");
        mg.faction = BOTH_FACTION;
        mg.msg = std::string(msg);
        messages.insert(messages.begin(), mg);
    }

    if (timer == 200)
    {
        char msg[256];
        Message mg;
        sprintf(msg, "TC75: Switch to the carrier or the turret and verify if there is a pink cross on the target");
        mg.faction = BOTH_FACTION;
        mg.msg = std::string(msg);
        messages.insert(messages.begin(), mg);
    }

}

void test78()
{

    BoxIsland *nemesis = new BoxIsland(&entities);
    nemesis->setName("Nemesis");
    nemesis->setLocation(0,-1.0,0);
    nemesis->buildTerrainModel(space,"terrain/goku.bmp");

    islands.push_back(nemesis);

    Structure *t1 = islands[0]->addStructure(new CommandCenter(GREEN_FACTION, LOGISTICS_ISLAND)    ,       0.0f,      -0.0f,  0,world);
    Structure *t2 = islands[0]->addStructure(new Runway(GREEN_FACTION),                                    200.0f,     200.0f,127,world);

    Balaenidae *_b = new Balaenidae(GREEN_FACTION);
    _b->init();
    _b->embody(world,space);
    _b->setPos(0.0f,20.5f,-8000.0f);
    _b->stop();

    entities.push_back(_b, _b->getGeom());

    CarrierTurret * _bo= new CarrierTurret(GREEN_FACTION);
    _bo->init();
    _bo->embody(world, space);
    _bo->attachTo(world,_b, -40.0f, 20.0f + 5, -210.0f);
    _bo->stop();

    entities.push_back(_bo, _bo->getGeom());


    CarrierArtillery * _w1= new CarrierArtillery(GREEN_FACTION);
    _w1->init();
    _w1->embody(world, space);
    _w1->attachTo(world,_b, -40.0, 27.0f, +210.0f);
    _w1->stop();

    entities.push_back(_w1, _w1->getGeom());
}

void checktest78(unsigned long timer)
{

    static Vehicle* manta = NULL;
    static bool bombed = false;
    Vehicle *b = findCarrier(GREEN_FACTION);

    if (timer == 50)
    {
        char msg[256];
        Message mg;
        sprintf(msg, "TC75: Testing Mantas bombing an island.");
        mg.faction = BOTH_FACTION;
        mg.msg = std::string(msg);
        messages.insert(messages.begin(), mg);
    }

    if (timer == 500)
    {
        size_t idx = 0;
        Vehicle *m = spawnManta(space,world,b,idx);

        manta = m;
    }

    if (timer == 800)
    {
        // launch
        launchManta(b);
    }

    if (timer == 900)
    {
        // @FIXME: Automatic AI bombing from Mantas should calculate the bias of the bomb based on the height where it is released, the speed of the airplane
        // and the location of the real target.  For instance, under this configuration the offset is 360 on positive z.
        manta->goTo(Vec3f(0,1000,360));
        manta->enableAuto();
    }

    if (manta && manta->arrived() && !bombed)
    {
        Vehicle *action = manta->fire(2,world, space);
        if (action != NULL)
        {
            entities.push_back(action, action->getGeom());
            soaring();
        }
        bombed = true;
    }


    if (timer == 5000)
    {
        Structure *t = islands[0]->getCommandCenter();

        if (!t)
        {
            printf("Test Passed\n");
            endWorldModelling();
            exit(1);
        }
        else
        {
            printf("Test Failed.  Somehow the commandcenter survived.\n");
            endWorldModelling();
            exit(1);
        }

    }
}

void test79()
{
    Missile *t = new Missile(GREEN_FACTION);
    t->init();
    t->embody(world, space);
    t->setPos(-5000,1000.0f,-5000);
    t->stop();

    /**
    dMatrix3 R;
    dRSetIdentity(R);
    dQuaternion q;
    dRFromAxisAndAngle(R,0,1,0,PI/2);
    dQfromR(q,R);
    dBodySetQuaternion(t->getBodyID(), q);
    **/

    t->setTheOrientation(Vec3f(7,8,9));

    entities.push_back(t, t->getGeom());

    BoxIsland *nemesis = new BoxIsland(&entities);
    nemesis->setName("Nemesis");
    nemesis->setLocation(0,-1.0,0);
    nemesis->buildTerrainModel(space,"terrain/goku.bmp");

    islands.push_back(nemesis);

    Structure *t1 = islands[0]->addStructure(new CommandCenter(GREEN_FACTION, LOGISTICS_ISLAND)    ,       20.0f,      -20.0f,  0,world);
    Structure *t2 = islands[0]->addStructure(new Runway(GREEN_FACTION),                                    200.0f,     200.0f,127,world);

    Balaenidae *_b = new Balaenidae(GREEN_FACTION);
    _b->init();
    _b->embody(world,space);
    _b->setPos(0.0f,20.5f,-8000.0f);
    _b->stop();

    entities.push_back(_b, _b->getGeom());

    CarrierTurret * _bo= new CarrierTurret(GREEN_FACTION);
    _bo->init();
    _bo->embody(world, space);
    _bo->attachTo(world,_b, -40.0f, 20.0f + 5, -210.0f);
    _bo->stop();

    entities.push_back(_bo, _bo->getGeom());


    CarrierArtillery * _w1= new CarrierArtillery(GREEN_FACTION);
    _w1->init();
    _w1->embody(world, space);
    _w1->attachTo(world,_b, -40.0, 27.0f, +210.0f);
    _w1->stop();

    entities.push_back(_w1, _w1->getGeom());


}

void checktest79(unsigned long timer)
{

    if (timer == 50)
    {
        char msg[256];
        Message mg;
        sprintf(msg, "TC79: Visually testing smoke.");
        mg.faction = BOTH_FACTION;
        mg.msg = std::string(msg);
        messages.insert(messages.begin(), mg);
    }

    if (timer == 200)
    {
        char msg[256];
        Message mg;
        sprintf(msg, "TC79: Check if the missile has a smoke tail");
        mg.faction = BOTH_FACTION;
        mg.msg = std::string(msg);
        messages.insert(messages.begin(), mg);
    }

    if (timer == 100)
    {
        controller.controllingid = CONTROLLING_NONE;
        Vec3f pos(-5000,1000.0f,-5500);
        Camera.setPos(pos);
        Camera.fw = Vec3f(0.0f,0.0f,1.0f);
    }

    if (timer == 3000)
    {
        printf("Test Passed\n");
        endWorldModelling();
        exit(1);

    }
}

void test80()
{
    BoxIsland *nemesis = new BoxIsland(&entities);
    nemesis->setName("Nemesis");
    nemesis->setLocation(0,-1.0,0);
    nemesis->buildTerrainModel(space,"terrain/goku.bmp");

    islands.push_back(nemesis);


}

void checktest80(unsigned long timer)
{

    CommandOrder co;

    co.command = Command::SpawnOrder;
    co.parameters.spawnid = VehicleSubTypes::CEPHALOPOD;

    Controller cont;
    cont.push(co);


    CommandOrder cnew;
    cnew = cont.pop();

    printf("Expected: %d\n", cnew.command);
    printf("Parameter: %d\n", cnew.parameters.spawnid);


    CommandOrder clast;
    clast = cont.pop();


    struct controlregister cr;

    cr.pitch = 9.2;

    crc val = crcSlow((uint8_t *) &cr,  sizeof(struct controlregister));

    printf("Crc: %d\n", val);

    cr.precesion = 34.2;

    val = crcSlow((uint8_t *) &cr,  sizeof(struct controlregister));

    printf("Crc: %d\n", val);

    std::string uuid = generate_hex(10);
    printf("UUID: %s\n", uuid.c_str());


    if (clast.command == Command::None)
    {
        printf("Test Passed\n");
        endWorldModelling();
        exit(1);
    } else {
        printf("Test Not passed \n");
        endWorldModelling();
        exit(0);
    }

}

void test82()
{
    BoxIsland *nemesis = new BoxIsland(&entities);
    nemesis->setName("Nemesis");
    nemesis->setLocation(0.0f,-1.0,0.0f);
    nemesis->buildTerrainModel(space,"terrain/thermopilae.bmp");

    islands.push_back(nemesis);

    // Entities will be added later in time.
    Balaenidae *_b = new Balaenidae(GREEN_FACTION);
    _b->init();
    _b->embody(world,space);
    _b->setPos(0.0f,20.5f,-16000.0f);
    _b->stop();

    entities.push_back(_b, _b->getGeom());

    Structure *t1 = islands[0]->addStructure(new CommandCenter(GREEN_FACTION, DEFENSE_ISLAND)    ,       800.0f,    -100.0f,0,world);
    Structure *t2 = islands[0]->addStructure(new Runway(GREEN_FACTION)           ,         0.0f,    -650.0f,-PI/4,world);
    Structure *t3 = islands[0]->addStructure(new Warehouse(GREEN_FACTION)      ,         0.0f,    650.0f,0,world);
    Structure *t4 = islands[0]->addStructure(new Warehouse(GREEN_FACTION)        ,       100.0f,    -650.0f,0,world);
    Structure *t5 = islands[0]->addStructure(new Warehouse(GREEN_FACTION)        ,        20.0f,    80.0f,0,world);
    Structure *t6 = islands[0]->addStructure(new Warehouse(GREEN_FACTION)        ,         -60.0f,    -80.0f,0,world);
    Structure *t7 = islands[0]->addStructure(new Warehouse(GREEN_FACTION)        ,         0.0f,    120.0f,0,world);
    Structure *t8 = islands[0]->addStructure(new Warehouse(GREEN_FACTION)        ,         -230.0f,    230.0f,0,world);

    Vec3f pos(0.0,1.32, - 60);
    Camera.setPos(pos);

    aiplayer = BLUE_AI;
    controller.faction = BOTH_FACTION;
}

void checktest82(unsigned long timer)
{

}



static int testing=-1;

void initWorldModelling()
{
    initWorldModelling(-1);
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
    dWorldSetAutoDisableFlag(world, 1);

    dWorldSetAutoDisableLinearThreshold(world, 0.01);
    dWorldSetAutoDisableAngularThreshold(world, 0.01);
    dWorldSetAutoDisableTime(world, 20);

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

void initWorldModelling(int testcase)
{
    switch(testcase)
    {
    case 1:initIslands();test1();break;             // Carrier stability
    case 2:initIslands();test1();test2();break;     // Manta landing on island.
    case 3:initIslands();test1();test2();test3();break;// Manta crashing on structure
    case 4:initIslands();test1();test2();test3();test4();break;// Manta landing on runway
    case 5:initIslands();test1();test2();break;     // Manta landing on aircraft
    case 6:initIslands();test1();test6();break;     // Carrier stranded on island
    case 7:test1();test2();test7();break;           // Manta crashing on water.
    case 8:initIslands();test1();test8();break;     // Walrus reaching island.
    case 9:test1();test9();break;                   // Walrus stability.
    case 10:initIslands();test1();test10();break;   // Walrus arrive to island and build the command center.
    case 11:initIslands();test11();break;           // Carrier stability far away.
    case 12:initIslands();test1();test12();break;   // Turret firing to Carrier.  Gunshot stability.
    case 13:initIslands();test13();break;           // Laser firing and hitting carrier.
    case 14:initIslands();;test14();break;          // Spawn Manta from Carrier, launch it and direct it towards some coordinate.
    case 15:initIslands();test15();break;           // Spawn Manta from Carrier, launch it, send it back behind the carrier and make it land.
    case 16:initIslands();test1();test12();break;   // Turret firing to the other Turret.
    case 17:initIslands();test1();test12();break;   // Turret automatically aiming at Carrier.
    case 18:initIslands();test1();test12();break;   // The other Turret aims automatically to the Carrier.
    case 19:initIslands();test1();test12();break;   // Both turrets aim to the carrier and start to fire.
    case 20:test20();test12();break;                // Both turrets aim to the carrier at a far away island.
    case 21:test21();break;                         // Check arriving at Complex island like Nemesis and verify if Walrus can land on island.
    case 22:test22();break;                         // This test has been cancelled because it was superseeded by the next one.
    case 23:test23();break;                         // Set walrus to reach the shore and the turret to fire to it
    case 24:test24();break;                         // Check azimuth and declination calculation based on a forward vector.
    case 25:test25();break;                         // Spawn many walruses and check FPS
    case 26:test26();break;                         // Initialize structures on all islands and check FPS
    case 27:test27();break;                         // Launch Manta from a drifted Carrier, check orientation.
    case 28:test28();break;                         // Add artillery and fire it !
    case 29:test29();break;                         // Turrets open fire to coming Manta
    case 30:test30();break;                         // Laser Turret opens fire on a static vehicle.
    case 31:test31();test10();break;                // Walrus arrive to a weird island, tumbles and is destroyed.
    case 32:test32();break;                         // Check continuous azimuth
    case 33:test33();break;                         // PID Manta
    case 34:test34();break;                         // Test advanced Walrus.
    case 35:test35();break;                         // Test Missiles fired from Carrier
    case 36:test36();break;                         // Test Missile Launcher
    case 37:test37();break;                         // Test Missile Launcher against Manta flying.
    case 38:test38();break;                         // Carrier attacks an island and tries to conquer it.
    case 39:test39();break;                         // Walrus attack enemy carrier trying to destroy it.
    case 40:test40();break;                         // Different walrus attack enemy carrier trying to destroy it.
    case 41:test41();break;                         // Carrier detects enemy automatically, stops what it is doing and attacks it.
    case 42:test42();break;                         // Carrier is attacked by Manta and activates defenses.
    case 43:test43();break;                         // Basic Dogfight.  Manta is flying and is attacked by an enemy manta.
    case 44:test44();break;                         // Manta attacks incoming walruses.
    case 45:test45();break;                         // Introducing Medusa.  Airplanes defending the islands.  They attack enemy carrier.
    case 46:test46();break;                         // Test FPS
    case 47:test47();break;                         // Heavy fighting while attackng an island.
    case 48:test48();break;                         // Medusas fighters land after a failed attack from a walrus.
    case 49:test49();break;                         // Check structure orientation (Runways).
    case 50:test50();break;                         // Check new structures.
    case 51:test51();break;                         // Check savegame
    case 52:test52();break;                         // Test Cephalopod aircraft stability, flying and basic destination.
    case 53:test53();break;                         // Test Cephalopod attacking command center.
    case 54:test54();break;                         // Stop aircraft from departing if the island is already free.
    case 55:test55();break;                         // Comm Link interrupted for a Manta when the Command Center is destroyed.
    case 56:test56();break;                         // Check a more complex manta landing.
    case 57:test57();break;                         // Check landing after successfully attacking an island
    case 58:test58();break;                         // Walrus landing on bumpy islands.
    case 59:test59();break;                         // Check placement of dock
    case 60:test60();break;                         // Walrus evades Carrier using potential fields.
    case 61:test61();break;                         // Using Cephalopod to build the command center.
    case 62:test62();break;                         // Performance measurement (several carriers, Mantas and walruses)
    case 63:test63();break;                         // Manta travels through different waypoints.
    case 64:test64();break;                         // Manta attacks island and it is defended by missile launchers.
    case 65:test65();break;                         // Carrier weapons.  Add a template weapon, attached to the carrier.
    case 66:test66();break;                         // Carrier Turret Weapon firing to Command Center as the carrier approaches the island.
    case 67:test67();break;                         // Test buggy behaviour on the islands.
    case 68:test68();break;                         // Test a wheeled version of Manta which will be interesting to test.
    case 69:test69();break;                         // Test a wheeled version of Walrus which is mandatory from now on.
    case 70:test70();break;                         // Manta lands on island's runway. Check slippage.
    case 71:test71();break;                         // Manta landing on a moving carrier.
    case 72:test72();break;                         // Testing explosions with ODE.
    case 73:test73();break;                         // Introducing Torpedos.
    case 74:test74();break;                         // Torpedos chasing Walruses.
    case 75:test75();break;                         // Testing Bombs !
    case 76:test76();break;                         // Playground Radar HUD
    case 77:test77();break;                         // Visually Testing Radar HUD with enemy units
    case 78:test78();break;                         // Testing Manta bombing an island.
    case 79:test79();break;                         // Visually checking smoke coming out of a missile thruster.
    case 80:test80();break;                         // Check multiple controllers and the command order.
    case 81:test81();break;                         // Test AdvancedWalrus landing on a bumpy island.
    case 82:test82();break;                         // Test island boundary and dock position.
    default:initIslands();test1();break;
    }

    testing = testcase;

    controller.faction = BOTH_FACTION;

}

void update(int value);

void worldStep(int value)
{
    timer++;
    update(value);

    switch(testing)
    {
    case 1:checktest1(timer);break;
    case 2:checktest2(timer);break;
    case 3:checktest3(timer);break;
    case 4:checktest4(timer);break;
    case 5:checktest5(timer);break;
    case 6:checktest6(timer);break;
    case 7:checktest7(timer);break;
    case 8:checktest8(timer);break;
    case 9:checktest9(timer);break;
    case 10:checktest10(timer);break;
    case 11:checktest11(timer);break;
    case 12:checktest12(timer);break;
    case 13:checktest13(timer);break;
    case 14:checktest14(timer);break;
    case 15:checktest15(timer);break;
    case 16:checktest16(timer);break;
    case 17:checktest17(timer);break;
    case 18:checktest18(timer);break;
    case 19:checktest19(timer);break;
    case 20:checktest20(timer);break;
    case 21:checktest21(timer);break;
    case 22:checktest22(timer);break;
    case 23:checktest23(timer);break;
    case 24:checktest24(timer);break;
    case 25:checktest25(timer);break;
    case 26:checktest26(timer);break;
    case 27:checktest27(timer);break;
    case 28:checktest28(timer);break;
    case 29:checktest29(timer);break;
    case 30:checktest30(timer);break;
    case 31:checktest31(timer);break;
    case 32:checktest32(timer);break;
    case 33:checktest33(timer);break;
    case 34:checktest34(timer);break;
    case 35:checktest35(timer);break;
    case 36:checktest36(timer);break;
    case 37:checktest37(timer);break;
    case 38:checktest38(timer);break;
    case 39:checktest39(timer);break;
    case 40:checktest40(timer);break;
    case 41:checktest41(timer);break;
    case 42:checktest42(timer);break;
    case 43:checktest43(timer);break;
    case 44:checktest44(timer);break;
    case 45:checktest45(timer);break;
    case 46:checktest46(timer);break;
    case 47:checktest47(timer);break;
    case 48:checktest48(timer);break;
    case 49:checktest49(timer);break;
    case 50:checktest50(timer);break;
    case 51:checktest51(timer);break;
    case 52:checktest52(timer);break;
    case 53:checktest53(timer);break;
    case 54:checktest54(timer);break;
    case 55:checktest55(timer);break;
    case 56:checktest56(timer);break;
    case 57:checktest57(timer);break;
    case 58:checktest58(timer);break;
    case 59:checktest59(timer);break;
    case 60:checktest60(timer);break;
    case 61:checktest61(timer);break;
    case 62:checktest62(timer);break;
    case 63:checktest63(timer);break;
    case 64:checktest64(timer);break;
    case 65:checktest65(timer);break;
    case 66:checktest66(timer);break;
    case 67:checktest67(timer);break;
    case 68:checktest68(timer);break;
    case 69:checktest69(timer);break;
    case 70:checktest70(timer);break;
    case 71:checktest71(timer);break;
    case 72:checktest72(timer);break;
    case 73:checktest73(timer);break;
    case 74:checktest74(timer);break;
    case 75:checktest75(timer);break;
    case 76:checktest76(timer);break;
    case 77:checktest77(timer);break;
    case 78:checktest78(timer);break;
    case 79:checktest79(timer);break;
    case 80:checktest80(timer);break;
    case 81:checktest81(timer);break;
    default: break;
    }

}

void endWorldModelling()
{
    dJointGroupDestroy (contactgroup);
    dSpaceDestroy (space);// Destroy spaces on islands.
    dWorldDestroy (world);
    dCloseODE();

    printf("God knows his rules and he has determined that this world must be terminated.\n");
    printf("The World has been terminated.\n");
}



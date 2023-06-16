//
//  tester.cpp
//  Wakuseibokan
//
//  Created by Rodrigo Ramele on 22/05/20.
//

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

#include "../ThreeMaxLoader.h"

#include "../font/DrawFonts.h"

#include "../math/yamathutil.h"

#include "../container.h"

#include "../usercontrols.h"
#include "../camera.h"

#include "../openglutils.h"
#include "../odeutils.h"

#include "../imageloader.h"
#include "../terrain/Terrain.h"

#include "../sounds/sounds.h"

#include "../engine.h"
#include "../keplerivworld.h"
#include "../usercontrols.h"
#include "../savegame.h"

#include "../units/Vehicle.h"
#include "../units/BoxVehicle.h"
#include "../units/Manta.h"
#include "../units/Walrus.h"
#include "../units/AdvancedWalrus.h"
#include "../units/Balaenidae.h"
#include "../units/SimplifiedDynamicManta.h"
#include "../units/Medusa.h"
#include "../units/Cephalopod.h"
#include "../units/AdvancedManta.h"

#include "../actions/Gunshot.h"
#include "../actions/Missile.h"

#include "../structures/Structure.h"
#include "../structures/Warehouse.h"
#include "../structures/Hangar.h"
#include "../structures/Runway.h"
#include "../structures/Laserturret.h"
#include "../structures/CommandCenter.h"
#include "../structures/Turret.h"
#include "../structures/Artillery.h"
#include "../structures/Launcher.h"
#include "../structures/Antenna.h"
#include "../structures/Dock.h"
#include "../structures/Radar.h"
#include "../structures/Factory.h"

#include "../weapons/Weapon.h"
#include "../weapons/CarrierTurret.h"
#include "../weapons/CarrierArtillery.h"

#include "../units/Wheel.h"
#include "../units/WheeledManta.h"
#include "../units/Otter.h"

#include "../map.h"

#include "testcase.h"

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

static int testing=-1;


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



void endWorldModelling()
{
    dJointGroupDestroy (contactgroup);
    dSpaceDestroy (space);// Destroy spaces on islands.
    dWorldDestroy (world);
    dCloseODE();

    printf("God knows his rules and he has determined that this world must be terminated.\n");
    printf("The World has been terminated.\n");
}


// General code that executes the testcase.

TestCase *t = NULL;

void update(int value);

void initWorldModelling(int testcase)
{

    testing = testcase;

    t = pickTestCase(testcase);

    t->init();

    controller.faction = BOTH_FACTION;

}

void worldStep(int value)
{
    timer++;
    update(value);

    long unsigned starttime = 200;

    if (timer == starttime)
    {
        char msg[256];
        Message mg;
        sprintf(msg, "TC%03d: %s", t->number(), t->title().c_str());
        mg.faction = BOTH_FACTION;
        mg.msg = std::string(msg);
        messages.insert(messages.begin(), mg);
    }

    t->check(timer);

    if (t->done())
    {
        if (t->passed())
        {
            printf("Test Passed\n");
            endWorldModelling();
            exit(1);
        }
        else
        {
            char msg[256];
            sprintf(msg, "Test Failed: %s\n", t->failedMessage().c_str());
            printf(msg);
            endWorldModelling();
            exit(0);
        }
    }
}



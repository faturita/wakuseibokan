//
//  testbox.cpp
//  mycarrier
//
//  Created by Rodrigo Ramele on 22/05/14.
//  Copyright (c) 2014 Baufest. All rights reserved.
//

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <stdarg.h>
#include <math.h>

#include <GLUT/glut.h>

#include <ode/ode.h>

#include <vector>
#include <mutex>

#include "ThreeMaxLoader.h"

#include "font/DrawFonts.h"

#include "math/yamathutil.h"

#include "container.h"

#include "usercontrols.h"
#include "camera.h"

#include "openglutils.h"

#include "sounds/sounds.h"

#include "keplerivworld.h"

#include "units/Vehicle.h"
#include "units/BoxVehicle.h"
#include "units/Manta.h"
#include "units/Walrus.h"
#include "units/Balaenidae.h"
#include "units/SimplifiedDynamicManta.h"

#include "actions/Gunshot.h"

#include "odeutils.h"

#include "imageloader.h"
#include "terrain/Terrain.h"

#include "structures/Structure.h"
#include "structures/Warehouse.h"
#include "structures/Hangar.h"
#include "structures/Runway.h"
#include "structures/Laserturret.h"
#include "structures/CommandCenter.h"
#include "structures/Turret.h"

#include "map.h"


extern  Controller controller;

/* dynamics and collision objects */

static dGeomID platform, ground;

dWorldID world;
dSpaceID space;
dBodyID body[NUM];
dJointGroupID contactgroup;

container<Vehicle*> entities;

std::vector<BoxIsland*> islands;

std::vector<std::string> messages;


// SYNC
Vehicle* gVehicle(dBodyID body)
{
    for(size_t i=entities.first();entities.exists(i);i=entities.next(i))
    {
        Vehicle *vehicle = entities[i];
        if (vehicle->getBodyID() == body)
        {
            return vehicle;
        }
    }
    return NULL;
}

void gVehicle(Vehicle* &v1, Vehicle* &v2, dBodyID b1, dBodyID b2, Structure* &s1, Structure*s2, dGeomID g1, dGeomID g2)
{
    for(size_t i=entities.first();entities.exists(i);i=entities.next(i))
    {
        Vehicle *vehicle = entities[i];
        if (vehicle->getBodyID() == b1)
        {
            v1 = vehicle;
        }
        if (vehicle->getBodyID() == b2)
        {
            v2 = vehicle;
        }
        if (vehicle->getBodyID()==NULL)
        {
            Structure *s= (Structure*)entities[i];
            if (s->getGeom() == g1)
            {
                s1 = s;
            }
            if (s->getGeom() == g2)
            {
                s2 = s;
            }
        }

    }
}

// SYNC
bool stranded(Vehicle *carrier, Island *island)
{
    if (island && carrier && carrier->getType() == 4 && carrier->getStatus() != Balaenidae::OFFSHORING)
    {
        Balaenidae *b = (Balaenidae*)carrier;

        b->offshore();
        controller.reset();
        b->doControl(controller);
        b->setStatus(Balaenidae::OFFSHORING);
        char str[256];
        sprintf(str, "Carrier has stranded on %s.", island->getName().c_str());
        messages.insert(messages.begin(), str);
    }
}

bool departed(Vehicle *walrus)
{
    if (walrus && walrus->getType() == WALRUS && walrus->getStatus() == Walrus::ROLLING)
    {
        Walrus *w = (Walrus*)walrus;

        w->setStatus(Walrus::SAILING);
        BoxIsland *island = w->getIsland();
        w->setIsland(NULL);
        char str[256];
        sprintf(str, "Walrus has departed from %s.", island->getName().c_str());
        messages.insert(messages.begin(), str);
    }
    return true;
}

// SYNC
bool arrived(Vehicle *walrus, Island *island)
{
    if (island && walrus && walrus->getType() == WALRUS && walrus->getStatus() != Walrus::ROLLING)
    {
        Walrus *w = (Walrus*)walrus;

        w->setStatus(Walrus::ROLLING);
        w->setIsland((BoxIsland*)island);
        char str[256];
        sprintf(str, "Walrus has arrived to %s.", island->getName().c_str());
        messages.insert(messages.begin(), str);
    }
    return true;
}

// SYNC
bool landed(Vehicle *manta, Island *island)
{
    if (manta && island && manta->getType() == 3)
    {
        if (manta->getStatus() != 0)
        {
            char str[256];
            sprintf(str, "Manta has landed on Island %s.", island->getName().c_str());
            messages.insert(messages.begin(), str);
            manta->setStatus(Manta::LANDED);
            manta->setThrottle(0.0f);
            controller.reset();
            manta->doControl(controller);
        }
    }
    return true;
}

// SYNC
bool isMineFire(Vehicle* vehicle, Gunshot *g)
{
    return (g->getOrigin() == vehicle->getBodyID());
}

// SYNC
bool hit(Vehicle *vehicle, Gunshot *g)
{
    /**   Sending too many messages hurts FPS
    if (vehicle && vehicle->getType() == CARRIER)
    {
        messages.insert(messages.begin(), std::string("Carrier is under fire!"));
    }
    if (vehicle && vehicle->getType() == MANTA)
    {
        messages.insert(messages.begin(), std::string("Manta is under fire!"));
    }
    if (vehicle && vehicle->getType() == WALRUS)
    {
        messages.insert(messages.begin(), std::string("Walrus is under fire!"));
    }
    **/

    // Dont hit me
    if (g->getOrigin() != vehicle->getBodyID())
    {
        vehicle->damage(2);
        return false;
    }
    return true;
}

bool hit(Structure* structure)
{
    if (structure)
    {
        //char str[256];
        //sprintf(str, "Island %s under attack!", structure->island->getName().c_str());
        //messages.insert(messages.begin(), str);
    }
    structure->damage(2);
}

// SYNC
bool releasecontrol(Vehicle* vehicle)
{
    if (vehicle && vehicle->getType() == MANTA)
    {
        if (vehicle->getStatus() != 0 && vehicle->getStatus() != 1)
        {
            vehicle->inert = true;
            controller.reset();
            vehicle->setStatus(0);
            vehicle->doControl(controller);
            vehicle->setThrottle(0.0f);
            messages.insert(messages.begin(), std::string("Manta has re landed on Aircraft."));
        }
    }
    return true;
}

void inline releasecontrol(dBodyID body)
{
    synchronized(entities.m_mutex)
    {
        Vehicle* vehicle = gVehicle(body);
        releasecontrol(vehicle);
    }
}


// SYNC
bool inline isType(Vehicle *vehicle, int type)
{
    bool result = false;
    if (vehicle)
    {
        if (vehicle->getType() == type)
        {
            result = true;
        }
    }
    return result;
}


bool inline isType(dBodyID body, int type)
{
    bool result = false;
    synchronized(entities.m_mutex)
    {
        Vehicle *vehicle = gVehicle(body);
        result = isType(vehicle,type);
    }
    return result;
}

bool inline isManta(dBodyID body)
{
    return isType(body,3);
}

bool inline isManta(Vehicle* vehicle)
{
    return isType(vehicle,3);
}

bool inline isCarrier(dBodyID body)
{
    return isType(body, 4);
}

bool inline isCarrier(Vehicle* vehicle)
{
    return isType(vehicle,4);
}

bool inline isWalrus(Vehicle* vehicle)
{
    return isType(vehicle, WALRUS);
}

bool inline isAction(dBodyID body)
{
    return isType(body, 5);
}

bool inline isAction(Vehicle* vehicle)
{
    return isType(vehicle, 5);
}


bool inline isRunway(Structure* s)
{
    if (s && s->getType()==LANDINGABLE)
    {
        return true;
    }
    return false;
}

Island* getIsland(dGeomID candidate)
{
    for (int j=0;j<islands.size();j++)
    {
        if (candidate == islands[j]->getGeom())
            return islands[j];
    }
    return NULL;
}

bool inline isIsland(dGeomID candidate)
{
    for (int j=0;j<islands.size();j++)
    {
        if (candidate == islands[j]->getGeom())
        {
            return true;
        }
    }
    return false;
}

// SYNC
bool inline groundcollisions(Vehicle *vehicle)
{
    if (vehicle)
    {
        if (vehicle->getSpeed()>100 and vehicle->getType() == MANTA)
        {
            explosion();
            controller.reset();
            vehicle->doControl(controller);
            vehicle->setThrottle(0.0f);
            vehicle->damage(200);
        }
    }
    return true;
}

void inline groundcollisions(dBodyID body)
{
    synchronized(entities.m_mutex)
    {
        Vehicle *vehicle = gVehicle(body);
        groundcollisions(vehicle);
    }
}


void nearCallback (void *data, dGeomID o1, dGeomID o2)
{
    int i,n;

    dBodyID b1,b2;

    // only collide things with the ground
    int g1 = (o1 == ground );
    int g2 = (o2 == ground );
    if (!(g1 ^ g2))
    {
        printf ("Ground colliding..\n");

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

            Vehicle *v1=NULL,*v2=NULL;
            Structure *s1=NULL, *s2=NULL;
            gVehicle(v1,v2,dGeomGetBody(contact[i].geom.g1), dGeomGetBody(contact[i].geom.g2),s1,s2,contact[i].geom.g1,contact[i].geom.g2);


            if (isIsland(contact[i].geom.g1) || isIsland(contact[i].geom.g2))
            {
                 // Island reaction
                 contact[i].surface.mode = dContactBounce |
                 dContactApprox1;

                 contact[i].surface.mu = 0;
                 contact[i].surface.bounce = 0.2f;
                 contact[i].surface.slip1 = 0.1f;
                 contact[i].surface.slip2 = 0.1f;

                 contact[i].surface.soft_erp = 0;   // 0 in both will force the surface to be tight.
                 contact[i].surface.soft_cfm = 0;


                 // Carrier stranded and Walrus arrived on island.
                 if (isIsland(contact[i].geom.g1) && isCarrier(v2) && stranded(v2,getIsland(contact[i].geom.g1))) {}
                 if (isIsland(contact[i].geom.g2) && isCarrier(v1) && stranded(v1,getIsland(contact[i].geom.g2))) {}
                 if (isIsland(contact[i].geom.g1) && isWalrus(v2)  && arrived(v2,getIsland(contact[i].geom.g1))) {}
                 if (isIsland(contact[i].geom.g2) && isWalrus(v1)  && arrived(v1,getIsland(contact[i].geom.g2))) {}


            } else
            if (ground == contact[i].geom.g1 || ground == contact[i].geom.g2 ) {

                 // Water buyoncy reaction
                 contact[i].surface.mode = dContactSlip1 | dContactSlip2 |
                 dContactSoftERP | dContactSoftCFM | dContactApprox1;

                 contact[i].surface.mu = 0.0f;
                 contact[i].surface.slip1 = 0.1f;
                 contact[i].surface.slip2 = 0.1f;
                 contact[i].surface.soft_erp = .5f;   // 0 in both will force the surface to be tight.
                 contact[i].surface.soft_cfm = .3f;

                 // Walrus reaching shore.
                 //if (ground == contact[i].geom.g1 && isWalrus(v2) && departed(v2)) {}
                 //if (ground == contact[i].geom.g2 && isWalrus(v1) && departed(v1)) {}

            } else {
                /**
                // Water buyoncy reaction
                contact[i].surface.mode = dContactSlip1 | dContactSlip2 |
                dContactSoftERP | dContactSoftCFM | dContactApprox1;

                contact[i].surface.mu = 0.0f;
                contact[i].surface.slip1 = 0.1f;
                contact[i].surface.slip2 = 0.1f;
                contact[i].surface.soft_erp = .5f;   // 0 in both will force the surface to be tight.
                contact[i].surface.soft_cfm = .3f;
                **/
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
    BoxIsland *thermopilae = new BoxIsland();
    thermopilae->setName("Thermopilae");
    thermopilae->setLocation(0.0f,-1.0,0.0f);
    thermopilae->buildTerrainModel(space,"terrain/thermopilae.bmp");

    BoxIsland *nonsquareisland = new BoxIsland();
    nonsquareisland->setName("Atolon");
    nonsquareisland->setLocation(0.0f,-1.0f,-100 kmf);
    nonsquareisland->buildTerrainModel(space,"terrain/nonsquareisland.bmp");

    BoxIsland *vulcano = new BoxIsland();
    vulcano->setName("Vulcano");
    vulcano->setLocation(145 kmf, -1.0f, 89 kmf);
    vulcano->buildTerrainModel(space,"terrain/vulcano.bmp");

    BoxIsland *nemesis = new BoxIsland();
    nemesis->setName("Nemesis");
    nemesis->setLocation(-450 kmf, -1.0, 300 kmf);
    nemesis->buildTerrainModel(space,"terrain/nemesis.bmp");

    BoxIsland *atom = new BoxIsland();
    atom->setName("Atom");
    atom->setLocation( 500 kmf, -1.0, -100 kmf);
    atom->buildTerrainModel(space,"terrain/atom.bmp");

    BoxIsland *island = new BoxIsland();
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
    Balaenidae *_b = new Balaenidae();
    _b->init();
    _b->setPos(0.0f,20.5f,-4000.0f);
    _b->embody(world,space);
    _b->stop();

    entities.push_back(_b);
}

void test2()
{
    SimplifiedDynamicManta *_manta1 = new SimplifiedDynamicManta();

    _manta1->init();
    _manta1->setPos(0.0f,20.5f,-6000.0f);
    _manta1->embody(world, space);
    _manta1->setStatus(0);
    _manta1->inert = true;
    _manta1->setStatus(2);
    _manta1->elevator = +12;
    struct controlregister c;
    c.thrust = 1500.0f/(-10.0);
    c.pitch = 12;
    _manta1->setControlRegisters(c);
    _manta1->setThrottle(1500.0f);

    entities.push_back(_manta1);
}


void checktest1(unsigned long timer)
{
    if (timer>500)
    {
        Vehicle *_b = entities[0];
        Vec3f val = _b->getPos();

        dReal *v = (dReal *)dBodyGetLinearVel(_b->getBodyID());
        Vec3f vec3fV;
        vec3fV[0]= v[0];vec3fV[1] = v[1]; vec3fV[2] = v[2];


        dReal *av = (dReal *)dBodyGetLinearVel(_b->getBodyID());
        Vec3f vav;
        vav[0]= av[0];vav[1] = av[1]; vav[2] = av[2];

        if (val.magnitude()>100)
        {
            printf("Test failed.\n");
            endWorldModelling();
            exit(-1);
        } else if (vav.magnitude()>5)
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
            exit(-1);
        }
    }
}
void checktest2(unsigned long timer)
{
    if (timer>2500)
    {
        Vehicle *_b = entities[1];
        _b->stop();
    }
    if (timer>4500)
    {
        Vehicle *_b = entities[1];
        Vec3f val = _b->getPos();

        dReal *v = (dReal *)dBodyGetLinearVel(_b->getBodyID());
        Vec3f vec3fV;
        vec3fV[0]= v[0];vec3fV[1] = v[1]; vec3fV[2] = v[2];


        dReal *av = (dReal *)dBodyGetLinearVel(_b->getBodyID());
        Vec3f vav;
        vav[0]= av[0];vav[1] = av[1]; vav[2] = av[2];

        if (val.magnitude()>100)
        {
            printf("Test failed.\n");
            endWorldModelling();
            exit(-1);
        } else if (vav.magnitude()>5)
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
            exit(-1);
        }
    }
}

void initWorldModelling()
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

    initIslands();

    test1();
    test2();
}

void update(int value);

void endWorldModelling()
{
    dJointGroupDestroy (contactgroup);
    dSpaceDestroy (space);
    dWorldDestroy (world);
    dCloseODE();

    printf("God knows his rules and he has determined that this world must be terminated.\n");
    printf("The World has been terminated.\n");
}

unsigned long timer=0;

void worldStep(int value)
{
    timer++;
    update(value);

    //checktest1(timer);
    //checktest2(timer);

}

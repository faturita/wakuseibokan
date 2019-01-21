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

void gVehicle(Vehicle* &v1, Vehicle* &v2, dBodyID b1, dBodyID b2, Structure* &s1, Structure* &s2, dGeomID g1, dGeomID g2)
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
        if (vehicle->getGeom() == g1 && vehicle->getType() > COLLISIONABLE)
        {
            s1 = (Structure*)vehicle;
        }
        if (vehicle->getGeom() == g2 && vehicle->getType() > COLLISIONABLE)
        {
            s2 = (Structure*)vehicle;
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

        w->setStatus(Walrus::OFFSHORING);
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
    if (island && walrus && walrus->getType() == WALRUS && walrus->getStatus() == Walrus::SAILING)
    {
        Walrus *w = (Walrus*)walrus;

        w->setStatus(Walrus::INSHORING);
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
    if (manta && island && manta->getType() == MANTA)
    {
        if (manta->getStatus() == Manta::FLYING)
        {
            char str[256];
            sprintf(str, "Manta has landed on Island %s.", island->getName().c_str());
            messages.insert(messages.begin(), str);

            controller.reset();
            SimplifiedDynamicManta *s = (SimplifiedDynamicManta*)manta;
            struct controlregister c;
            c.thrust = 0.0f;
            c.pitch = 0.0f;
            s->setControlRegisters(c);
            s->setThrottle(0.0f);
            s->setStatus(Manta::LANDED);
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
        if (vehicle->getStatus() != Manta::ON_DECK && vehicle->getStatus() != Manta::TACKINGOFF)
        {
            controller.reset();

            SimplifiedDynamicManta *s = (SimplifiedDynamicManta*)vehicle;
            struct controlregister c;
            c.thrust = 0.0f;
            c.pitch = 0.0f;
            s->setControlRegisters(c);
            s->setThrottle(0.0f);
            s->setStatus(Manta::ON_DECK);
            s->inert = true;
            messages.insert(messages.begin(), std::string("Manta has landed on Aircraft."));
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

bool inline isRunway(dGeomID candidate)
{
    for(size_t i=entities.first();entities.exists(i);i=entities.next(i))
    {
        Vehicle *vehicle = entities[i];
        if (vehicle && vehicle->getGeom() == candidate && vehicle->getType()==LANDINGABLE)
        {
            return true;
        }
    }
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


// @FIXME Check the island !
CommandCenter* findCommandCenter()
{
    for(size_t i=entities.first();entities.exists(i);i=entities.next(i))
    {
        Vehicle *v=entities[i];
        if (v->getType() == CONTROL)
        {
            return (CommandCenter*)v;
        }
    }
    return NULL;
}

// SYNC
bool inline groundcollisions(Vehicle *vehicle)
{
    if (vehicle)
    {
        if (vehicle->getSpeed()>70 and vehicle->getType() == MANTA)
        {
            explosion();
            SimplifiedDynamicManta *s = (SimplifiedDynamicManta*)vehicle;
            struct controlregister c;
            c.thrust = 0.0f;
            c.pitch = 0.0f;
            s->setControlRegisters(c);
            s->setThrottle(0.0f);
            s->damage(1);
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

            Vehicle *v1=NULL,*v2=NULL;
            Structure *s1=NULL, *s2=NULL;
            gVehicle(v1,v2,dGeomGetBody(contact[i].geom.g1), dGeomGetBody(contact[i].geom.g2),s1,s2,contact[i].geom.g1,contact[i].geom.g2);


            if ( ( isManta(v1) && isCarrier(v2) && releasecontrol(v1) ) ||
                 ( isManta(v2) && isCarrier(v1) && releasecontrol(v2) ) )
                {
                // Manta landing on Carrier
                contact[i].surface.mode = dContactBounce |
                dContactApprox1;

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


                contact[i].surface.mu = 0.99f;
                contact[i].surface.slip1 = 0.9f;
                contact[i].surface.slip2 = 0.9f;
                contact[i].surface.bounce = 0.2f;

                if ( isRunway(s1) && isManta(v2) && landed(v2, s1->island)) {}
                if ( isRunway(s2) && isManta(v1) && landed(v1, s2->island)) {}

            } else
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
                 if (ground == contact[i].geom.g1 && isWalrus(v2) && departed(v2)) {}
                 if (ground == contact[i].geom.g2 && isWalrus(v1) && departed(v1)) {}

                 if (ground == contact[i].geom.g1 && v2 && isManta(v2) && groundcollisions(v2)) {}
                 if (ground == contact[i].geom.g2 && v1 && isManta(v1) && groundcollisions(v1)) {}

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
                if (v1 && isManta(v1) && groundcollisions(v1)) {}
                if (v2 && isManta(v2) && groundcollisions(v2)) {}
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
    _b->embody(world,space);
    _b->setPos(0.0f,20.5f,-4000.0f);
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
    _manta1->setStatus(Manta::FLYING);
    _manta1->elevator = +12;
    struct controlregister c;
    c.thrust = 1500.0f/(-10.0);
    c.pitch = 12;
    _manta1->setControlRegisters(c);
    _manta1->setThrottle(1500.0f);

    entities.push_back(_manta1);
}

void test3()
{
    entities.push_back(islands[0]->addStructure(new Structure()  ,           0.0f,-1000.0f,space,world));
}
void test4()
{
    entities.push_back(islands[0]->addStructure(new Runway()     ,           0.0f,    0.0f,space,world));
    entities.push_back(islands[0]->addStructure(new Hangar()     ,        -550.0f,    0.0f,space,world));
}



void test6()
{
    Vec3f pos(0.0f,10.0f,-4400.0f);
    Camera.setPos(pos);

    Balaenidae *b = (Balaenidae*)entities[0];

    b->setPos(0.0f,20.5f,-3000.0f);

}

void test7()
{
    Vec3f pos(0.0f,10.0f,-3700.0f);
    Camera.setPos(pos);
}

void test8()
{
    Walrus *_walrus = new Walrus();

    _walrus->init();
    _walrus->setPos(200.0f,1.32f,-6000.0f);
    _walrus->embody(world, space);
    _walrus->setStatus(Walrus::SAILING);
    struct controlregister c;
    c.thrust = 200.0f;
    c.roll = 0;
    _walrus->setControlRegisters(c);
    _walrus->setThrottle(200.0f);

    entities.push_back(_walrus);
}

void test9()
{
    Walrus *_walrus = new Walrus();
    _walrus->init();
    _walrus->setPos(200.0f,1.32f,-6000.0f);
    _walrus->embody(world, space);
    _walrus->setStatus(Walrus::SAILING);
    _walrus->stop();

    entities.push_back(_walrus);
}

void test10()
{
    Vehicle *walrus = (entities[0])->spawn(world,space,WALRUS);
    if (walrus != NULL)
    {
        size_t id = entities.push_back(walrus);
        messages.insert(messages.begin(), std::string("Walrus has been deployed."));
    }
}


void checktest1(unsigned long timer)
{
    if (timer>500)
    {
        Vehicle *_b = entities[0];
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
void checktest2(unsigned long timer)
{
    if (timer==850)
    {
        SimplifiedDynamicManta *_manta1 = (SimplifiedDynamicManta*)entities[1];
        _manta1->elevator = -4;
        struct controlregister c;
        c.thrust = 0.0f/(-10.0);
        c.pitch = -4;
        _manta1->setControlRegisters(c);
        _manta1->setThrottle(0.0f);
    }
    if (timer==1200)
    {
        Vehicle *_b = entities[1];
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


void checktest3(unsigned long timer)
{
    if (timer==620)
    {
        SimplifiedDynamicManta *_manta1 = (SimplifiedDynamicManta*)entities[1];
        _manta1->elevator = -4;
        struct controlregister c;
        c.thrust = 400.0f/(-10.0);
        c.pitch = -4;
        _manta1->setControlRegisters(c);
        _manta1->setThrottle(400.0f);
    }
    if (timer==1000)
    {
        SimplifiedDynamicManta *_manta1 = (SimplifiedDynamicManta*)entities[1];

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

void checktest4(unsigned long  timer)
{
    if (timer==720)
    {
        SimplifiedDynamicManta *_manta1 = (SimplifiedDynamicManta*)entities[1];
        _manta1->elevator = -4;
        struct controlregister c;
        c.thrust = 400.0f/(-10.0);
        c.pitch = -4;
        _manta1->setControlRegisters(c);
        _manta1->setThrottle(400.0f);
        _manta1->setStatus(Manta::FLYING);
    }
    if (timer==1000)
    {
        SimplifiedDynamicManta *_manta1 = (SimplifiedDynamicManta*)entities[1];

        if (_manta1->getStatus()!=Manta::LANDED)
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


void checktest5(unsigned long  timer)
{
    if (timer==240)
    {
        SimplifiedDynamicManta *_manta1 = (SimplifiedDynamicManta*)entities[1];
        _manta1->elevator = -4;
        struct controlregister c;
        c.thrust = 400.0f/(-10.0);
        c.pitch = -4;
        _manta1->setControlRegisters(c);
        _manta1->setThrottle(400.0f);
        _manta1->setStatus(Manta::FLYING);
    }
    if (timer==265)
    {
        SimplifiedDynamicManta *_manta1 = (SimplifiedDynamicManta*)entities[1];
        _manta1->elevator = -4;
        struct controlregister c;
        c.thrust = 0.0f/(-10.0);
        c.pitch = 0;
        _manta1->setControlRegisters(c);
        _manta1->setThrottle(0.0f);
        _manta1->stop();
        _manta1->setStatus(Manta::FLYING);
    }
    if (timer==1000)
    {
        SimplifiedDynamicManta *_manta1 = (SimplifiedDynamicManta*)entities[1];

        if (_manta1->getStatus()!=Manta::ON_DECK)
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

void checktest6(unsigned long timer)
{
    static bool isOffshoring = false;

    if (timer==100)
    {
        Balaenidae *b = (Balaenidae*)entities[0];
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
        Balaenidae *b = (Balaenidae*)entities[0];

        if (b->getStatus()==Balaenidae::OFFSHORING)
        {
            isOffshoring = true;
        }
    }
    if (timer==3800)
    {
        Balaenidae *b = (Balaenidae*)entities[0];

        Vehicle *_b = entities[0];
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


void checktest7(unsigned long  timer)
{

    if (timer==380)
    {
        SimplifiedDynamicManta *_manta1 = (SimplifiedDynamicManta*)entities[1];
        _manta1->elevator = +14;
        struct controlregister c;
        c.thrust = 3500.0f/(-10.0);
        c.pitch = 0;
        _manta1->setControlRegisters(c);
        _manta1->setThrottle(350.0f);
        _manta1->setStatus(Manta::FLYING);
    }
    if (timer==900)
    {
        SimplifiedDynamicManta *_manta1 = (SimplifiedDynamicManta*)entities[1];

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


void checktest8(unsigned long  timer)
{
    static bool isWalrusInIsland = false;
    static bool didWalrusLeftIsland = false;

    if (timer>100)
    {
        Walrus *_walrus = (Walrus*)entities[1];

        if (_walrus->getIsland() != NULL)
        {
            isWalrusInIsland = true;
        }

        if (_walrus->getStatus() == Walrus::OFFSHORING)
        {
            didWalrusLeftIsland = true;
        }
    }

    if (timer>=3500)
    {
        Walrus *_walrus = (Walrus*)entities[1];

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



void checktest9(unsigned long timer)
{
    if (timer>500)
    {
        Vehicle *_b = entities[1];
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

void checktest10(unsigned long timer)
{
    static unsigned long timerstep = 0;
    Walrus *_walrus = (Walrus*)entities[1];

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

    if (stateMachine == 0 && _walrus->getStatus()==Walrus::ROLLING)
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

        Structure *s = island->addStructure(new CommandCenter(),x,z,space,world);
        entities.push_back(s);

        timerstep = timer;
        stateMachine = 3;
    }

    if (stateMachine == 3 && timerstep>0 && timer == (timerstep + 100))
    {
        CommandCenter *c = findCommandCenter();

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

static int testing=-1;

void initWorldModelling(int testcase)
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



    switch(testcase)
    {
    case 1:initIslands();test1();break;// Carrier stability
    case 2:initIslands();test1();test2();break;// Manta landing on island.
    case 3:initIslands();test1();test2();test3();break;// Manta crashing on structure
    case 4:initIslands();test1();test2();test3();test4();break;// Manta landing on runway
    case 5:initIslands();test1();test2();break;// Manta landing on aircraft
    case 6:initIslands();test1();test6();break;// Carrier stranded on island
    case 7:test1();test2();test7();break; //Manta crashing on water.
    case 8:initIslands();test1();test8();break; // Walrus reaching island.
    case 9:test1();test9();break; // Walrus stability.
    case 10:initIslands();test1();test10();break; // Walrus arrive to island and build the command center.
    case 11:initIslands();test1();test10();break;
    default:break;
    }

    testing = testcase;

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
    default: break;
    }

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



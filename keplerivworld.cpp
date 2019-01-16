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

#define kmf *1000.0f

#include <vector>
#include <mutex>
#include <unordered_map>

#include "container.h"

#include "odeutils.h"

#include "keplerivworld.h"

// Add a new interface to an enbodied object.
#include "units/BoxVehicle.h"
#include "units/Walrus.h"
#include "units/Manta.h"
#include "units/SimplifiedDynamicManta.h"
#include "units/Buggy.h"
#include "units/MultiBodyVehicle.h"
#include "units/Balaenidae.h"
#include "units/Beluga.h"

#include "terrain/Terrain.h"

#include "structures/Structure.h"
#include "structures/Runway.h"
#include "structures/Hangar.h"
#include "structures/Turret.h"
#include "structures/Warehouse.h"
#include "structures/Laserturret.h"
#include "structures/CommandCenter.h"

extern  Controller controller;

/* dynamics and collision objects */

static dGeomID platform, ground;

dWorldID world;
dSpaceID space;
dBodyID body[NUM];
dJointGroupID contactgroup;

container<Vehicle*> vehicles;

std::vector<BoxIsland*> islands;
std::vector<Structure*> structures;

std::vector<std::string> messages;

std::vector<Vehicle*> controlables;

// SYNC
Vehicle* gVehicle(dBodyID body)
{
    for(size_t i=vehicles.first();vehicles.exists(i);i=vehicles.next(i))
    {
        Vehicle *vehicle = vehicles[i];
        if (vehicle->getBodyID() == body)
        {
            return vehicle;
        }
    }
    return NULL;
}

void gVehicle(Vehicle* &v1, Vehicle* &v2, dBodyID b1, dBodyID b2)
{
    for(size_t i=vehicles.first();vehicles.exists(i);i=vehicles.next(i))
    {
        Vehicle *vehicle = vehicles[i];
        if (vehicle->getBodyID() == b1)
        {
            v1 = vehicle;
        }
        if (vehicle->getBodyID() == b2)
        {
            v2 = vehicle;
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
bool hit(Vehicle *vehicle)
{
    if (vehicle && vehicle->getType() == 4)
    {
        messages.insert(messages.begin(), std::string("Carrier is under fire!"));
        vehicle->damage(2);
    }
    if (vehicle && vehicle->getType() == 3)
    {
        messages.insert(messages.begin(), std::string("Manta is under fire!"));
        vehicle->damage(2);
    }
    return true;
}

// SYNC
bool releasecontrol(Vehicle* vehicle)
{
    if (vehicle && vehicle->getType() == 3)
    {
        if (vehicle->getStatus() != 0 && vehicle->getStatus() != 1)
        {
            vehicle->inert = true;
            controller.reset();
            vehicle->setStatus(0);
            vehicle->doControl(controller);
            vehicle->setThrottle(0.0f);
            messages.insert(messages.begin(), std::string("Manta has landed on Aircraft."));
        }
    }
    return true;
}

void inline releasecontrol(dBodyID body)
{
    synchronized(vehicles.m_mutex)
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
    synchronized(vehicles.m_mutex)
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


bool inline isStructure(dGeomID candidate)
{
    for (int i=0; i<structures.size(); i++)
    {
        if (candidate == structures[i]->getGeom())
        {
            return true;
        }
    }
    return false;
}

bool inline isRunway(dGeomID candidate)
{
    for (int i=0; i<structures.size(); i++)
    {
        if (candidate == structures[i]->getGeom() && structures[i]->getType()==LANDINGABLE)
        {
            return true;
        }
    }
    return false;
}

Structure* getRunway(dGeomID candidate)
{
    for (int i=0; i<structures.size(); i++)
    {
        if (candidate == structures[i]->getGeom() && structures[i]->getType()==LANDINGABLE)
        {
            return structures[i];
        }
    }
    return NULL;
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
        if (vehicle->getSpeed()>100 and vehicle->getType() == 3)
        {
            explosion();
            controller.reset();
            vehicle->doControl(controller);
            vehicle->setThrottle(0.0f);
        }
    }
    return true;
}

void inline groundcollisions(dBodyID body)
{
    synchronized(vehicles.m_mutex)
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
           synchronized(vehicles.m_mutex)
           {
               Vehicle *v1=NULL,*v2=NULL;
               gVehicle(v1,v2,dGeomGetBody(contact[i].geom.g1), dGeomGetBody(contact[i].geom.g2));


               if ( (ground == contact[i].geom.g1  &&  groundcollisions(v2)) ||
                    (ground == contact[i].geom.g2  &&  groundcollisions(v1)) )
               {
                   // Nothing to do here....
               }


               if ( (isRunway(contact[i].geom.g1) && isManta(v2) && landed(v2,getRunway(contact[i].geom.g1)->island)) ||
                    (isRunway(contact[i].geom.g2) && isManta(v1) && landed(v1,getRunway(contact[i].geom.g2)->island)) )
               {
                   contact[i].surface.mode = dContactBounce |
                   dContactApprox1;

                   contact[i].surface.mu = 0.99;dInfinity;
                   contact[i].surface.slip1 = 0.9;
                   contact[i].surface.slip2 = 0.9;
                   contact[i].surface.bounce = 0.2;

               } else
               if ( (isRunway(contact[i].geom.g1) || isRunway(contact[i].geom.g2))  )
               {
                   contact[i].surface.mode = dContactBounce |
                   dContactApprox1;

                   contact[i].surface.mu = 0.99;dInfinity;
                   contact[i].surface.slip1 = 0.9;
                   contact[i].surface.slip2 = 0.9;
                   contact[i].surface.bounce = 0.2;

                   //releasecontrol(dGeomGetBody(contact[i].geom.g1));
               } else
               if ( (isStructure(contact[i].geom.g1) && groundcollisions(v2)) ||
                    (isStructure(contact[i].geom.g2) && groundcollisions(v1))  )
               {
                   printf("Structure Collision\n");
               } else

               if (isIsland(contact[i].geom.g1) || isIsland(contact[i].geom.g2))
               {
                   contact[i].surface.mode = dContactBounce |
                   dContactApprox1;

                   contact[i].surface.mu = 0;
                   contact[i].surface.bounce = 0.2;
                   contact[i].surface.slip1 = 0.1;
                   contact[i].surface.slip2 = 0.1;

                   if (isIsland(contact[i].geom.g1) && isCarrier(v2) && stranded(v2,getIsland(contact[i].geom.g1))) {}
                   if (isIsland(contact[i].geom.g2) && isCarrier(v1) && stranded(v1,getIsland(contact[i].geom.g2))) {}
                   if (isIsland(contact[i].geom.g1) && isWalrus(v2)  && arrived(v2,getIsland(contact[i].geom.g1))) {}
                   if (isIsland(contact[i].geom.g2) && isWalrus(v1)  && arrived(v1,getIsland(contact[i].geom.g2))) {}
               } else
               if ( ( isManta(v1) && isCarrier(v2) && releasecontrol(v1) ) ||
                    ( isManta(v2) && isCarrier(v1) && releasecontrol(v2) ) )
                   {
                   contact[i].surface.mode = dContactBounce |
                   dContactApprox1;

                   contact[i].surface.mu = dInfinity;
                   contact[i].surface.slip1 = 0;
                   contact[i].surface.slip2 = 0;
                   contact[i].surface.bounce = 0.2;

               } else
               if (  (isCarrier(v1) && isAction(v2) && hit(v1)) ||
                     (isCarrier(v2) && isAction(v1) && hit(v2)) )
               {

                   contact[i].surface.mode = dContactSlip1 | dContactSlip2 |
                   dContactSoftERP | dContactSoftCFM | dContactApprox1;
                   contact[i].surface.mu = 0;dInfinity;
                   contact[i].surface.slip1 = 0.1;
                   contact[i].surface.slip2 = 0.1;

                   printf("Carrier hit\n");
               } else
               if (  (isManta(v1) && isAction(v2) && hit(v1)) ||
                     (isManta(v2) && isAction(v1) && hit(v2)) )
               {

                   contact[i].surface.mode = dContactSlip1 | dContactSlip2 |
                   dContactSoftERP | dContactSoftCFM | dContactApprox1;
                   contact[i].surface.mu = 0;dInfinity;
                   contact[i].surface.slip1 = 0.1;
                   contact[i].surface.slip2 = 0.1;

                   printf("Manta hit\n");
               }  else {
                   contact[i].surface.mode = dContactSlip1 | dContactSlip2 |
                   dContactSoftERP | dContactSoftCFM | dContactApprox1;

                   contact[i].surface.mu = 0;dInfinity;
                   contact[i].surface.slip1 = 0.1;
                   contact[i].surface.slip2 = 0.1;


                   //if (ground == contact[i].geom.g1 && isWalrus(v2) && departed(v2)) {}
                   //if (ground == contact[i].geom.g2 && isWalrus(v1) && departed(v1)) {}
                }
            }

           contact[i].surface.soft_erp = .5;   // 0 in both will force the surface to be tight.
           contact[i].surface.soft_cfm = .3;


           dJointID c = dJointCreateContact (world,contactgroup,&contact[i]);
           dJointAttach (c,
                         dGeomGetBody(contact[i].geom.g1),
                         dGeomGetBody(contact[i].geom.g2));
       }
   }
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
            if ((ground == contact[i].geom.g1) || (ground == contact[i].geom.g2))
            {
                groundcollisions(dGeomGetBody(contact[i].geom.g1));
                groundcollisions(dGeomGetBody(contact[i].geom.g2));
            }

            if ( (isRunway(contact[i].geom.g1) || isRunway(contact[i].geom.g2))  )
            {
                contact[i].surface.mode = dContactBounce |
                dContactApprox1;

                contact[i].surface.mu = 0.99;dInfinity;
                contact[i].surface.slip1 = 0.9;
                contact[i].surface.slip2 = 0.9;
                contact[i].surface.bounce = 0.2;

                //releasecontrol(dGeomGetBody(contact[i].geom.g1));
            } else
            if ( (isStructure(contact[i].geom.g1) || isStructure(contact[i].geom.g2))  )
            {
                //printf("Structure Collision\n");
                groundcollisions(dGeomGetBody(contact[i].geom.g1));
                groundcollisions(dGeomGetBody(contact[i].geom.g2));
            } else

            if (isIsland(contact[i].geom.g1) || isIsland(contact[i].geom.g2))
            {
                contact[i].surface.mode = dContactBounce |
                dContactApprox1;

                contact[i].surface.mu = 0;
                contact[i].surface.bounce = 0.2;
                contact[i].surface.slip1 = 0.1;
                contact[i].surface.slip2 = 0.1;
            } else
            if ( ((isManta(dGeomGetBody(contact[i].geom.g1)) && isCarrier(dGeomGetBody(contact[i].geom.g2))) ) ||
                (isManta(dGeomGetBody(contact[i].geom.g2)) && isCarrier(dGeomGetBody(contact[i].geom.g1))) )
                {
                contact[i].surface.mode = dContactBounce |
                dContactApprox1;

                contact[i].surface.mu = dInfinity;
                contact[i].surface.slip1 = 0;
                contact[i].surface.slip2 = 0;
                contact[i].surface.bounce = 0.2;

                releasecontrol(dGeomGetBody(contact[i].geom.g1));
            } else
            if (  isAction(dGeomGetBody(contact[i].geom.g1)) || isAction(dGeomGetBody(contact[i].geom.g2)) )
            {

                contact[i].surface.mode = dContactSlip1 | dContactSlip2 |
                dContactSoftERP | dContactSoftCFM | dContactApprox1;
                contact[i].surface.mu = 0;dInfinity;
                contact[i].surface.slip1 = 0.1;
                contact[i].surface.slip2 = 0.1;


                //messages.insert(messages.begin(), std::string("Carrier is under fire!"));

            }  else {
                contact[i].surface.mode = dContactSlip1 | dContactSlip2 |
                dContactSoftERP | dContactSoftCFM | dContactApprox1;
                contact[i].surface.mu = 0;dInfinity;
                contact[i].surface.slip1 = 0.1;
                contact[i].surface.slip2 = 0.1;
            }


            contact[i].surface.soft_erp = 0.5;
            contact[i].surface.soft_cfm = 0.3;
            
            
            dJointID c = dJointCreateContact (world,contactgroup,&contact[i]);
            dJointAttach (c,
                          dGeomGetBody(contact[i].geom.g1),
                          dGeomGetBody(contact[i].geom.g2));
        }
    }
}


void initWorldPopulation()
{
    Balaenidae *_b = new Balaenidae();
    _b->init();
    _b->setPos(0.0f,50.0f,-2200.0f);
    _b->embody(world,space);


    // This is the enemy carrier.
    /**
    Beluga *_bel = new Beluga();
    _bel->init();
    _bel->setPos(-450 kmf, 50.0f, 300 kmf + -2200.0f);
    _bel->embody(world,space);
    **/

    //dBodySetData(_boxVehicle1->getBodyID(),(void*)(new size_t(vehicles.push_back(_boxVehicle1))));

    vehicles.push_back(_b);


    for(int i=vehicles.first();vehicles.exists(i);i=vehicles.next(i))
    {
        controlables.push_back(vehicles[i]);
    }
    
}

void initWorldModelling()
{
	dReal k;
	dMass m;
    
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
    dWorldSetCFM (world,1e-5);
    
    // Set Damping
    dWorldSetLinearDamping(world, 0.01);  // 0.00001
    dWorldSetAngularDamping(world, 0.005);     // 0.005
    dWorldSetMaxAngularSpeed(world, 200);
    
    // Set and get the depth of the surface layer around all geometry objects. Contacts are allowed to sink into the surface layer up to the given depth before coming to rest. The default value is zero. Increasing this to some small value (e.g. 0.001) can help prevent jittering problems due to contacts being repeatedly made and broken.
    dWorldSetContactSurfaceLayer (world,0.001);
    
	ground = dCreatePlane (space,0,1,0,0);
    
    
    /**
    BoxIsland *baltimore = new BoxIsland();
    baltimore->setLocation(800.0,0.3,800.0);
    baltimore->buildTerrainModel(space,"terrain/baltimore.bmp"); //,1000,10);

   
    BoxIsland *runway = new BoxIsland();
    runway->setLocation(1800.0,0.3,1800.0);
    runway->buildTerrainModel(space,"terrain/runway.bmp"); //,1000,10);
    

    
    
    BoxIsland *vulcano = new BoxIsland();
    vulcano->setLocation(-1300.0,1.0,-2300.0);
    vulcano->buildTerrainModel(space,"terrain/vulcano.bmp"); //,1000,10);
    
    
    BoxIsland *vulcrum = new BoxIsland();
    vulcrum->setLocation(-2300.0,1.0,-1300.0);
    vulcrum->buildTerrainModel(space,"terrain/vulcrum.bmp"); //,1000,10);
    
    BoxIsland *heightmap = new BoxIsland();
    heightmap->setLocation(-23000.0,1.0,0.0);
    heightmap->buildTerrainModel(space,"terrain/heightmap.bmp"); //,1000,10);
    

    

    
**/

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

    /**structures.push_back(thermopilae->addStructure(new Turret()     ,         100.0f, -100.0f,space,world));
    structures.push_back(thermopilae->addStructure(new LaserTurret(),        -100.0f,  100.0f,space,world));
    structures.push_back(thermopilae->addStructure(new Structure()  ,           0.0f,-1000.0f,space,world));
    structures.push_back(thermopilae->addStructure(new Runway()     ,           0.0f,    0.0f,space,world));
    structures.push_back(thermopilae->addStructure(new Hangar()     ,        -550.0f,    0.0f,space,world));
    structures.push_back(thermopilae->addStructure(new Warehouse()  ,       -1000.0f,    0.0f,space,world));
    structures.push_back(thermopilae->addStructure(new CommandCenter()     ,  400.0f, -500.0f,space,world));**/


    /**

    structures.push_back(nonsquareisland->addStructure(new Runway(),       0.0f,    0.0f,space,world));
    structures.push_back(nonsquareisland->addStructure(new Hangar()   , -550.0f,    0.0f,space,world));
    structures.push_back(nonsquareisland->addStructure(new Turret()   ,  100.0f, -100.0f,space,world));


    structures.push_back(vulcano->addStructure(new Runway(),       0.0f,    0.0f,space,world));
    structures.push_back(vulcano->addStructure(new Hangar()   , -550.0f,    0.0f,space,world));
    structures.push_back(vulcano->addStructure(new Turret()   ,  100.0f, -100.0f,space,world));


    structures.push_back(nemesis->addStructure(new Runway(),       0.0f,    0.0f,space,world));
    structures.push_back(nemesis->addStructure(new Hangar()   , -550.0f,    0.0f,space,world));
    structures.push_back(nemesis->addStructure(new Turret()   ,  100.0f, -100.0f,space,world));
    **/

    initWorldPopulation();

    for(int i=0;i<structures.size();i++)
    {
        controlables.push_back(structures[i]);
    }

    for(int i=0;i<controlables.size();i++)
    {
        printf("[%d] - Controlables type %d\n",i,controlables[i]->getType());
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



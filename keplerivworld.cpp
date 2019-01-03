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
#include <unordered_map>

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

#include "terrain/Terrain.h"


extern  Controller controller;

/* dynamics and collision objects */

static dGeomID platform, ground;

dWorldID world;
dSpaceID space;
dBodyID body[NUM];
dJointGroupID contactgroup;

std::vector<Vehicle*> vehicles;

std::vector<BoxIsland*> islands;


std::vector<std::string> messages;


std::unordered_map<dBodyID, Vehicle*> vehiclesInWorld;


// this is called by dSpaceCollide when two objects in space are
// potentially colliding.

void inline releasecontrol(dBodyID body)
{
    for (int i=0; i<vehicles.size(); i++)
    {
        Vehicle *vehicle = vehicles[i];
        if (vehicle->getBodyID() == body)
        {
            if (vehicle->getType() == 3)
            {
                vehicle->letMeGo = true;
            }
        }
    }
}
bool inline isManta(dBodyID body)
{
    for (int i=0; i<vehicles.size(); i++)
    {
        Vehicle *vehicle = vehicles[i];
        if (vehicle->getBodyID() == body)
        {
            if (vehicle->getType() == 3)
            {
                return true;
            }
        }
    }
    return false;
}

bool inline isCarrier(dBodyID body)
{
    for (int i=0; i<vehicles.size(); i++)
    {
        Vehicle *vehicle = vehicles[i];
        if (vehicle->getBodyID() == body)
        {
            if (vehicle->getType() == 4)
            {
                return true;
            }
        }
    }
    return false;
}

void inline groundcollisions(dBodyID body)
{
    for (int i=0; i<vehicles.size(); i++)
    {
        Vehicle *vehicle = vehicles[i];
        if (vehicle->getBodyID() == body)
        {
            if (vehicle->getSpeed()>100 and vehicle->getType() == 3)
            {
                explosion();
                controller.reset();
                vehicle->setThrottle(0.0f);
            }
        }
    }

}


 void nearCallback (void *data, dGeomID o1, dGeomID o2)
{
    int i,n;
    
    dBodyID b1,b2;

    bool isWater = false;
    bool isIsland = false;
    
    // only collide things with the ground
    int g1 = (o1 == ground );
    int g2 = (o2 == ground );
    if (!(g1 ^ g2))
    {
        //printf ("Ground colliding..\n");
        isWater = true;
        
        //return;
    }
    

    if (o1 == ground)
    {
        groundcollisions(dGeomGetBody(o2));
    } else if (o2 == ground) {
        groundcollisions(dGeomGetBody(o1));
    } else {

        for (int j=0;j<islands.size();j++)
        {
            if (o1 == islands[j]->getGeom())
            {
                groundcollisions(dGeomGetBody(o2));
                isIsland = true;
            }
            if (o2 == islands[j]->getGeom())
            {
                groundcollisions(dGeomGetBody(o1));
                isIsland == true;
            }

        }
    }

    if (!isIsland && !isWater)
    {
        b1 = dGeomGetBody(o1);
        b2 = dGeomGetBody(o2);
        if (b1 && b2 && dAreConnected (b1,b2)) return;
    }







    /*for (int i=0; i<vehicles.size(); i++) {
        Vehicle *vehicle = vehicles[i];
        
        if (b1 == vehicle->getBodyID() || b2 == vehicle->getBodyID())
        {
            //printf ("Vehicle %d collisioned\n", i);
            //vehicles[0].stop();
            
            
            if (vehicles[i]->getType()==2 && vehicles[i]->getSpeed()>300)
            {
                printf("Walrus has been destroyed.\n");
                explosion();
                
            }

            if (vehicles[i]->getType()==3 && vehicles[i]->getSpeed()>100)
            {
                printf("Sorry your plane has crashed. You're dead.\n");
                if (o1 == ground)
                {
                    printf("It has collided with the ground.\n");
                    printf("Height:\n",vehicles[i]->getPos()[1]);
                }
                if (o2 == ground)
                {
                    printf("It has collided with the ground.\n");
                    printf("Height:\n",vehicles[i]->getPos()[1]);
                }
                for (int j=0;j<islands.size();j++)
                {
                    if ((o2 == islands[j]->getGeom()) || (o1 == islands[j]->getGeom()))
                    {
                        printf("It has collided with the island.\n");
                        //printf("Height:\n",vehicles[i]->getPos()[1]);
                        int a;
                        //std::cin >> a;
                        ((Manta*)vehicle)->setThrottle(0.0f);
                        controller.reset();
                        explosion();
                    }
                }
                //exit(2);
            }
        }

    }**/
    
    
    
    const int N = 10;
    dContact contact[N];
    n = dCollide (o1,o2,N,&contact[0].geom,sizeof(dContact));
    if (n > 0) {
        for (i=0; i<n; i++) {


            if ( ((isManta(dGeomGetBody(contact[i].geom.g1)) && isCarrier(dGeomGetBody(contact[i].geom.g2))) ) ||
                (isManta(dGeomGetBody(contact[i].geom.g2)) && isCarrier(dGeomGetBody(contact[i].geom.g1))) )
                {
                contact[i].surface.mode =
                dContactSoftERP | dContactSoftCFM | dContactApprox1;

                contact[i].surface.mu = dInfinity;
                printf("Contact between units...\n",n);
                contact[i].surface.slip1 = 0;
                contact[i].surface.slip2 = 0;

                releasecontrol(dGeomGetBody(contact[i].geom.g1));
            } else {
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

//
//  NearCallback is the collision handler callback function.
//
//
void nearCadllback (void *data, dGeomID o1, dGeomID o2)
{
    /* exit without doing anything if the two bodies are connected by a joint */
    dBodyID b1,b2;
    dContact contact;

    
    b1 = dGeomGetBody(o1);
    b2 = dGeomGetBody(o2);
    if (b1 && b2 && dAreConnected (b1,b2)) return;
    
    
    for (int i=0; i<vehicles.size(); i++) {
        Vehicle *vehicle = vehicles[i];
        
        if (b1 == vehicle->getBodyID() || b2 == vehicle->getBodyID())
        {
            printf ("Vehicle %d collisioned\n", i);
            //vehicles[0].stop();
            
            if (vehicles[i]->getType()==3 && vehicles[i]->getSpeed()>300)
            {
                printf("Sorry your plane has crashed. You're dead.\n");
                exit(2);
            }
        }
    }
    
    
    /**contact.surface.mode |=
    dContactMotionN |                   // velocity along normal
    dContactMotion1 |
    dContactMotion2 | // and along the contact plane
    dContactFDir1;                      // don't forget to set the direction 1
    **/
    
    //contact.surface.mu = 0.1;
    //contact.surface.mu2 = 0.1;
    //contact.surface.bounce = 0.001;

    //contact.surface.mode = dContactBounce ;
    
    //contact.surface.mode |=
    //dContactMotionN;
    

    
    if (dCollide (o1,o2,1,&contact.geom,sizeof(dContactGeom))) {
        dJointID c = dJointCreateContact (world,contactgroup,&contact);
        dJointAttach (c,b1,b2);
    }

}

dGeomID gheight;

// Heightfield dimensions

#define HFIELD_WSTEP			15			// Vertex count along edge >= 2
#define HFIELD_DSTEP			31

#define HFIELD_WIDTH			REAL( 1000.0 )
#define HFIELD_DEPTH			REAL( 1000.0 )

#define HFIELD_WSAMP			( HFIELD_WIDTH / ( HFIELD_WSTEP-1 ) )
#define HFIELD_DSAMP			( HFIELD_DEPTH / ( HFIELD_DSTEP-1 ) )


// NO SE USA
dReal heightfield_caldlback( void* pUserData, int x, int z )
{
    dReal h = 100;
    
    return h;
}

void initWorldPopulation()
{
    //Init lands, fixed objects, and objects
    BoxVehicle *_boxVehicle1 = new BoxVehicle();
    _boxVehicle1->init();
    _boxVehicle1->setPos(-10.0f,40.0f,300.0f);
    
    
    BoxVehicle *_boxVehicle2 = new BoxVehicle();
    _boxVehicle2->init();
    _boxVehicle2->setPos(0.0f,40.0f,-300.0f);
    
    
    // Start modelling BoxVehicles
    _boxVehicle1->embody(world, space);
    _boxVehicle2->embody(world, space);
    
    
    
    // Add vehicles
    SimplifiedDynamicManta *_manta1 = new SimplifiedDynamicManta();
    _manta1->init();
    _manta1->setPos(+130.0f, 10.0f, -1010.0f);
    
    _manta1->embody(world, space);
    
    
    Walrus *_walrus2 = new Walrus();
    _walrus2->init();
    _walrus2->setPos(0.0f, 0.0f, -900.0f);
    
    _walrus2->embody(world, space);
    
    
    Walrus *_walrus3 = new Walrus();
    _walrus3->init();
    _walrus3->setPos(600.0f, 0.0f, -200.0f);
    
    _walrus3->embody(world, space);
    
    
    Buggy *_buggy = new Buggy();
    _buggy->init();
    _buggy->setPos(1300.0f, 10, 10);
    
    _buggy->embody(world, space);


    MultiBodyVehicle *_mb = new MultiBodyVehicle();
    _mb->init();
    _mb->setPos(100.0f,0.0f,+2000.0f);
    _mb->embody(world,space);

    Balaenidae *_b = new Balaenidae();
    _b->init();
    _b->setPos(0.0f,50.0f,+2000.0f);
    _b->embody(world,space);
    

    vehicles.push_back(_boxVehicle1);
    //vehicles.push_back(_boxVehicle2);
    vehicles.push_back(_manta1);
    vehicles.push_back(_walrus2);
    vehicles.push_back(_walrus3);
    vehicles.push_back(_mb);
    //vehicles.push_back(_buggy);
    vehicles.push_back(_b);

    for(int i=0;i<vehicles.size();i++)
        vehiclesInWorld[vehicles[i]->getBodyID()] = vehicles[i];
    
}

void initWorldModelling()
{
	dReal k;
	dMass m;
    
	/* create world */
	dInitODE();
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
    
    
    // create lift platform
    //platform = dCreateBox(space, 100, 10, 100);
    //dGeomSetPosition(platform, 0.0, 5, -300);
    
    
    //drawBoxIsland(300,5,300,1000,10);
    
    //dHeightfieldDataID heightid = dGeomHeightfieldDataCreate();
    
    // Create an finite heightfield.
    //dGeomHeightfieldDataBuildCallback( heightid, NULL, heightfield_callback,
    //                                  REAL( 1000.0 ), REAL (1000.0), HFIELD_WSTEP, HFIELD_DSTEP,
    //                                  REAL( 1.0 ), REAL( 0.0 ), REAL( 0.0 ), 0 );
    
    //dGeomHeightfieldDataSetBounds( heightid, REAL( -4.0 ), REAL( +6.0 ) );
    
    //gheight = dCreateHeightfield( space, heightid, 1 );
    
    //dGeomSetPosition( gheight, 300, 5, 300 );
    
    // Add this to destroy
    // dGeomHeightfieldDataDestroy( heightid );
    
    //_boxIsland.buildTerrainModel(space);
    
    /**
    BoxIsland *baltimore = new BoxIsland();
    baltimore->setLocation(800.0,0.3,800.0);
    baltimore->buildTerrainModel(space,"terrain/baltimore.bmp"); //,1000,10);

   
    BoxIsland *runway = new BoxIsland();
    runway->setLocation(1800.0,0.3,1800.0);
    runway->buildTerrainModel(space,"terrain/runway.bmp"); //,1000,10);
    
    BoxIsland *nemesis = new BoxIsland();
    nemesis->setLocation(-1300.0,1.0,-1300.0);
    nemesis->buildTerrainModel(space,"terrain/nemesis.bmp"); //,1000,10);
    
    
    BoxIsland *vulcano = new BoxIsland();
    vulcano->setLocation(-1300.0,1.0,-2300.0);
    vulcano->buildTerrainModel(space,"terrain/vulcano.bmp"); //,1000,10);
    
    
    BoxIsland *vulcrum = new BoxIsland();
    vulcrum->setLocation(-2300.0,1.0,-1300.0);
    vulcrum->buildTerrainModel(space,"terrain/vulcrum.bmp"); //,1000,10);
    
    BoxIsland *heightmap = new BoxIsland();
    heightmap->setLocation(-23000.0,1.0,0.0);
    heightmap->buildTerrainModel(space,"terrain/heightmap.bmp"); //,1000,10);
    
    BoxIsland *island = new BoxIsland();
    island->setLocation(-23000.0,1.0,1000.0);
    island->buildTerrainModel(space,"terrain/island.bmp"); //,1000,10);
    
    BoxIsland *thermopilae = new BoxIsland();
    thermopilae->setLocation(6000.0f,1.0,1000.0);
    thermopilae->buildTerrainModel(space,"terrain/thermopilae.bmp"); //,1000,10);
    
    
    BoxIsland *nonsquareisland = new BoxIsland();
    nonsquareisland->setLocation(6000.0f,-0.3,-1000.0);
    nonsquareisland->buildTerrainModel(space,"terrain/nonsquareisland.bmp"); //,1000,10);
    

    BoxIsland *atom = new BoxIsland();
    atom->setLocation(1500.0f,-0.3,+10000.0);
    atom->buildTerrainModel(space,"terrain/atom.bmp"); //,1000,10);
**/

    BoxIsland *baltandmore = new BoxIsland();
    baltandmore->setLocation(-1600.0f,-1.0,+9000.0);
    baltandmore->buildTerrainModel(space,"terrain/runway.bmp"); //,1000,10);

    /**islands.push_back(baltimore);
    islands.push_back(runway);
    islands.push_back(nemesis);
    islands.push_back(vulcrum);
    islands.push_back(vulcano);
    islands.push_back(island);
    islands.push_back(heightmap);
    islands.push_back(thermopilae);
    islands.push_back(nonsquareisland);
    islands.push_back(atom);
    **/
    islands.push_back(baltandmore);
    
    initWorldPopulation();
    
    //MultiBodyVehicle *_mbody = new MultiBodyVehicle();
    //_mbody->init();
    //_mbody->setPos(30,57.0,-100.0);
    //_mbody->embody(world, space);
    
    //MultiBodyVehicle *_abody = new MultiBodyVehicle();
    //_abody->init();
    //_abody->setPos(60,5.0,-1000);
    //_abody->embody(world, space);
    
    //vehicles.push_back(_abody);
    //vehicles.push_back(_mbody);
    
    //buildTerrainModel(space,_vulcano,600.0f,-1340.0f, 0.0f, -1740.0f);
    
    //buildTerrainModel(space, _terrain,600.0f, 640.0f, 0.0f, 340.0f );
    
    //buildTerrainModel(space, _terrain,600.0f, -940.0f, 0.0f, -740.0f );
    
    //buildTerrainModel(space, _vulcano,600.0f, -1340.0f, 0.0f, -1740.0f );
    
    //buildTerrainModel(space, _baltimore,600.0f, 0.0f, 0.0f, 0.0f);
    
    //buildTerrainModel(space, _nemesis,600.0f, 2340.0f, 0.0f, 1220.0f );
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


void buildTerrainModel(dSpaceID space, Terrain *_landmass, float fscale,float xx,float yy,float zz)
{
    float slopeData[_landmass->width()*_landmass->length()];
    float scale = fscale / fmax(_landmass->width() - 1, _landmass->length() - 1);
    fscale = 10.0f;
    for(int z = 0; z < _landmass->length() - 1; z++) {
        
        for(int x = 0; x < _landmass->width(); x++) {
            slopeData[z*_landmass->width() +x] = 0; /**_landmass->getHeight(x, z)*fscale+**/;
        }
    }
    
    float xsamples = _landmass->width(),zsamples = _landmass->width(), xdelta = 10, zdelta =10;
    
    dHeightfieldDataID slopeHeightData = dGeomHeightfieldDataCreate (); // data geom
    
    float width = xsamples*xdelta; // 5 samples at delta of 1 unit
    float depth = zsamples*zdelta; // 5 samples at delta of 1 unit
    
    dGeomHeightfieldDataBuildSingle(slopeHeightData,slopeData,
                                    0,width,depth, xsamples, zsamples, 1.0f, 5.0f,10.0f, 0); // last 4
    
    //dGeomHeightfieldDataSetBounds (slopeHeightData, 0.0f, 100.0f); // sort
    
    dGeomID slopeHeightID = dCreateHeightfield(space, slopeHeightData, 1); // fff
    
    dGeomSetPosition(slopeHeightID,xx,yy,zz);
    
}


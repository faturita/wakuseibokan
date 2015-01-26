//
//  keplerivworld.cpp
//  mycarrier
//
//  Created by Rodrigo Ramele on 24/05/14.
//  Copyright (c) 2014 Baufest. All rights reserved.
//

#include <vector>

#include "odeutils.h"

#include "keplerivworld.h"

// Add a new interface to an enbodied object.
#include "units/BoxVehicle.h"
#include "units/Walrus.h"
#include "units/Manta.h"

#include "terrain/Terrain.h"

/* dynamics and collision objects */

static dGeomID platform, ground;

dWorldID world;
dSpaceID space;
dBodyID body[NUM];
dJointID joint[NUM-1];
dJointGroupID contactgroup;
dGeomID geoms[NUM];

std::vector<Vehicle*> vehicles;


void nearCallback (void *data, dGeomID o1, dGeomID o2)
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
        }
    }
    
    
    contact.surface.mode |=
    dContactMotionN |                   // velocity along normal
    dContactMotion1 | dContactMotion2 | // and along the contact plane
    dContactFDir1;                      // don't forget to set the direction 1

    
    //contact.surface.mode = dContactBounce ;
    //contact.surface.mu = 0.1;
    //contact.surface.mu2 = 0.1;
    //contact.surface.bounce = 0.001;
    
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



dReal heightfield_caldlback( void* pUserData, int x, int z )
{
    dReal h = 5;
    
    return h;
}

BoxIsland _boxIsland;

void initWorldModelling()
{
	dReal k;
	dMass m;
    
	/* create world */
	dInitODE();
	world = dWorldCreate();
	space = dHashSpaceCreate (0);
    
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
    //platform = dCreateBox(space, 1000, 10, 1000);
    //dGeomSetPosition(platform, 300, 5, 300);
    
    
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
    
    gheight = _boxIsland.buildTerrainModel(space);
    
    
    //Init lands, fixed objects, and objects
    BoxVehicle *_boxVehicle1 = new BoxVehicle();
    _boxVehicle1->init();
    _boxVehicle1->setPos(0.0f,40.0f,300.0f);
    
    BoxVehicle *_boxVehicle2 = new BoxVehicle();
    _boxVehicle2->init();
    _boxVehicle2->setPos(0.0f,40.0f,-300.0f);

    
    body[0] = dBodyCreate(world);
    _boxVehicle1->embody(body[0]);
    geoms[0] = dCreateSphere( space, 2.64f);
    //geoms[0] = dCreateBox (space, 7, 7, 7);
	dGeomSetBody(geoms[0], body[0]);
    
	body[1] = dBodyCreate(world);
    _boxVehicle2->embody(body[1]);
    geoms[1] = dCreateSphere( space, 2.64f);
    //geoms[1] = dCreateBox (space, 7, 7, 7);
	dGeomSetBody(geoms[1], body[1]);
    
    Walrus *_walrus1 = new Walrus();
    _walrus1->init();
    _walrus1->setPos(0.0f, 40.0f, -600.0f);
    
    body[2] = dBodyCreate(world);
    _walrus1->embody(body[2]);
    geoms[2] = dCreateSphere( space, 2.64f );
    dGeomSetBody(geoms[2], body[2]);
    
    Manta *_manta1 = new Manta();
    _manta1->init();
    _manta1->setPos(30.0f, 5.0f, -650.0f);
    
    body[3] = dBodyCreate(world);
    _manta1->embody(body[3]);
    geoms[3] = dCreateSphere( space, 2.64f );
    dGeomSetBody(geoms[3], body[3]);
    

    vehicles.push_back(_boxVehicle1);
    vehicles.push_back(_boxVehicle2);
    vehicles.push_back(_walrus1);
    vehicles.push_back(_manta1);
    
    
    
    /**
    Vec3f pos2;
	pos2 = _walrus.getPos();
	body[1] = dBodyCreate(world);
	//dBodySetPosition(body[1],100.0f,20.0f,0.0f);
    
	dBodySetPosition(body[1], pos2[0], pos2[1], pos2[2]);
	dMassSetBox(&m,1,SIDE,SIDE,SIDE);
	dMassAdjust(&m, MASS*3.0f);
	dBodySetMass(body[1],&m);
	sphere[1] = dCreateSphere( space, RADIUS);
	dGeomSetBody(sphere[1], body[1]);
     **/
    
    
    
    
    
    /**
	Vec3f pos;
	pos = _manta.getPos();
	body[0] = dBodyCreate(world);
	printf ("%10.8f, %10.8f, %10.8f\n", pos[0],pos[1],pos[2]);
	dBodySetPosition(body[0],pos[0],pos[1],pos[2]);
	dMassSetBox(&m,1,SIDE,SIDE,SIDE);
	dMassAdjust(&m, MASS*1);
	dBodySetMass(body[0],&m);
	sphere[0] = dCreateSphere( space, RADIUS);
	dGeomSetBody(sphere[0], body[0]);
    
    
	Vec3f pos2;
	pos2 = _walrus.getPos();
	body[1] = dBodyCreate(world);
	//dBodySetPosition(body[1],100.0f,20.0f,0.0f);
    
	dBodySetPosition(body[1], pos2[0], pos2[1], pos2[2]);
	dMassSetBox(&m,1,SIDE,SIDE,SIDE);
	dMassAdjust(&m, MASS*3.0f);
	dBodySetMass(body[1],&m);
	sphere[1] = dCreateSphere( space, RADIUS);
	dGeomSetBody(sphere[1], body[1]);
    **/
    
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
    
    printf("The World has been terminated.\n");
}


void buildTerrainModel(dSpaceID space, Terrain *_landmass, float fscale,float xx,float yy,float zz)
{
    float slopeData[_landmass->width()*_landmass->length()];
    float scale = fscale / fmax(_landmass->width() - 1, _landmass->length() - 1);
    fscale = 10.0f;
    for(int z = 0; z < _landmass->length() - 1; z++) {
        
        for(int x = 0; x < _landmass->width(); x++) {
            slopeData[z*_landmass->width() +x] = _landmass->getHeight(x, z)*fscale;
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


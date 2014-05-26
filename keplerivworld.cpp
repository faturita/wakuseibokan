//
//  keplerivworld.cpp
//  mycarrier
//
//  Created by Rodrigo Ramele on 24/05/14.
//  Copyright (c) 2014 Baufest. All rights reserved.
//

#include "odeutils.h"

#include "keplerivworld.h"

// Add a new interface to an enbodied object.
#include "units/BoxVehicle.h"

/* dynamics and collision objects */

dWorldID world;
dSpaceID space;
dBodyID body[NUM];
dJointID joint[NUM-1];
dJointGroupID contactgroup;
dGeomID sphere[NUM];


BoxVehicle _boxVehicle1;
BoxVehicle _boxVehicle2;


void nearCallback (void *data, dGeomID o1, dGeomID o2)
{
    /* exit without doing anything if the two bodies are connected by a joint */
    dBodyID b1,b2;
    dContact contact;
    
    printf ("Collision near\n");
    
    b1 = dGeomGetBody(o1);
    b2 = dGeomGetBody(o2);
    if (b1 && b2 && dAreConnected (b1,b2)) return;
    
    if (b1 == _boxVehicle1.getBodyID())
    {
        _boxVehicle1.stop();
    }
    
    if (b2 == _boxVehicle2.getBodyID())
    {
        _boxVehicle2.stop();
    }
    
    contact.surface.mode = dContactBounce ;
    contact.surface.mu = 0;
    contact.surface.mu2 = 1;
    contact.surface.bounce = 0;
    
    if (dCollide (o1,o2,1,&contact.geom,sizeof(dContactGeom))) {
        dJointID c = dJointCreateContact (world,contactgroup,&contact);
        dJointAttach (c,b1,b2);
    }
}

void initWorldModelling()
{
	dReal k;
	dMass m;
    
	/* create world */
	dInitODE2(0);
	world = dWorldCreate();
	space = dHashSpaceCreate (0);
	contactgroup = dJointGroupCreate (1000000);
	dWorldSetGravity (world,0,-9.81f,0);
	dCreatePlane (space,0,1,0,0);

    
    body[0] = dBodyCreate(world);
    _boxVehicle1.embody(body[0]);
    sphere[0] = dCreateSphere( space, 1.0);
	dGeomSetBody(sphere[0], body[0]);
    
	body[1] = dBodyCreate(world);
    _boxVehicle2.embody(body[1]);
    sphere[1] = dCreateSphere( space, 1.0);
	dGeomSetBody(sphere[1], body[1]);
    
	//dBodySetPosition(body[1],100.0f,20.0f,0.0f);
    
    
    
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
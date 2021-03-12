//
//  Buggy.cpp
//  wakuseibokan
//
//  Created by Rodrigo Ramele on 6/26/15.
//  Copyright (c) 2015 Baufest. All rights reserved.
//

#include "Buggy.h"
#include "../md2model.h"


void Buggy::init()
{
    _model = MD2Model::loadModel("walrus.md2");
    if (_model != NULL)
        _model->setAnimation("run");
    else
        printf ("Model has been initialized");
    
    setForward(0,0,1);
    
}

int Buggy::getType()
{
    return 4;
}

void Buggy::doMaterial()
{
    GLfloat specref[] = { 1.0f, 1.0f, 1.0f, 1.0f};
    
    glEnable(GL_COLOR_MATERIAL);
    
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    
    glMaterialfv(GL_FRONT, GL_SPECULAR, specref);
    glMateriali(GL_FRONT, GL_SHININESS,128);
}


void Buggy::drawModel(float yRot, float xRot, float x, float y, float z)
{
    float f[3];
    f[0] = 0; f[1] = 0; f[2] = 0;
    
    //Draw the saved model
    if (_model != NULL)
    {
        glPushMatrix();
        glTranslatef(x, y, z);
        
        doTransform(f, R);
        
        drawArrow();
        
       	glRotatef(-180.0f, 1.0f, 0.0f, 0.0f);
        //glRotatef(yRot, 0.0f, 1.0f, 0.0f);
        
        //glRotatef(xRot, 1.0f, 0.0f, 0.0f);
        
        //doMaterial();
        
        _model->draw();
        
        glPopMatrix();
    }
    else
    {
        printf ("model is null\n");
    }
}

void Buggy::drawModel()
{
    drawModel(0,0,pos[0],pos[1],pos[2]);
}

void Buggy::doControl(Controller controller)
{
    //engine[0] = -controller.roll;
    //engine[1] = controller.yaw;
    //engine[2] = -controller.pitch;
    //steering = -controller.precesion;
    
    
    setThrottle(-controller.registers.pitch);
    
    xRotAngle = controller.registers.precesion;
    
    yRotAngle = controller.registers.roll;
    
}


void Buggy::drawDirectModel()
{
    float modX=0.0f, modY=15.0f, modZ=0.0f;
    
    modX = pos[0];
    modY = pos[1];
    modZ = pos[2];
    
    Vec3f forward = toVectorInFixedSystem(0, 0, -0.1,-xRotAngle,yRotAngle);
    
    modX+=speed*forward[0]; modY+=speed*forward[1];modZ+=speed*forward[2];
    
    pos += speed * forward;
    
    drawModel(xRotAngle, yRotAngle, modX, modY, modZ);
}



void Buggy::doDynamics()
{
    doDynamics(getBodyID());
}


#define CMASS 1		// chassis mass
#define WMASS 0.2	// wheel mass


// dynamics and collision objects (chassis, 3 wheels, environment)
static dBodyID carbody[5];
static dJointID carjoint[4];	// joint[0] is the front wheel
static dSpaceID car_space;
static dGeomID box[1];
static dGeomID sphere[4];
static dGeomID ground_box;


void Buggy::doDynamics(dBodyID body)
{
    //dReal *v = (dReal *)dBodyGetLinearVel(body);

    //dJointSetHinge2Param (carjoint[0],dParamVel,getThrottle());
    //dJointSetHinge2Param (carjoint[0],dParamFMax,1000);

    dJointSetHinge2Param (carjoint[0],dParamVel2,yRotAngle);
    dJointSetHinge2Param (carjoint[0],dParamFMax2,1000);
    
    dJointSetHinge2Param (carjoint[1],dParamVel2,yRotAngle);
    dJointSetHinge2Param (carjoint[1],dParamFMax2,1000);
    
    // steering
    dReal v = dJointGetHinge2Angle1 (carjoint[0]);
    if (v > 0.1) v = 0.1;
    if (v < -0.1) v = -0.1;
    v *= 10.0;
    //dJointSetHinge2Param (carjoint[0],dParamVel,0);
    //dJointSetHinge2Param (carjoint[0],dParamFMax,1000);
    //dJointSetHinge2Param (carjoint[0],dParamLoStop,-0.75);
    //dJointSetHinge2Param (carjoint[0],dParamHiStop,0.75);
    //dJointSetHinge2Param (carjoint[0],dParamFudgeFactor,0.1);
    
    //dBodyAddRelForce (body,0, 0,getThrottle());

    
    // This should be after the world step
    /// stuff
    dVector3 result;
    
    dBodyVectorToWorld(body, 0,0,1,result);
    setForward(result[0],result[1],result[2]);
    
    const dReal *dBodyPosition = dBodyGetPosition(body);
    const dReal *dBodyRotation = dBodyGetRotation(body);
    
    setPos(dBodyPosition[0],dBodyPosition[1],dBodyPosition[2]);
    setLocation((float *)dBodyPosition, (float *)dBodyRotation);
    
}




void Buggy::embody(dBodyID myBodySelf)
{
    
}

static const dVector3 yunit = { 0, 1, 0 }, zunit = { 0, 0, 1 };

void Buggy::embody(dWorldID world, dSpaceID space)
{
    dMass m;
    
    float LENGTH = 10;
    float WIDTH = 5;
    float HEIGHT = 5;
    
    float RADIUS=4;	// wheel radius
    float STARTZ=1;	// starting height of chassis
    
    me = dBodyCreate(world);
    
    dBodySetPosition (me,pos[0],pos[1],pos[2]);
    dMassSetBox (&m,1,WIDTH,HEIGHT,LENGTH);
    dMassAdjust (&m,CMASS);
    dBodySetMass (me,&m);
    box[0] = dCreateBox (0,WIDTH,HEIGHT,LENGTH);
    dGeomSetBody(box[0], me);
    
    
    // wheel bodies
    for (int i=1; i<=4; i++) {
        carbody[i] = dBodyCreate (world);
        dQuaternion q;
        dQFromAxisAndAngle (q,1,0,0,0);
        dBodySetQuaternion (carbody[i],q);
        dMassSetSphere (&m,1,RADIUS);
        dMassAdjust (&m,WMASS);
        dBodySetMass (carbody[i],&m);
        sphere[i-1] = dCreateSphere (0,RADIUS);
        dGeomSetBody (sphere[i-1],carbody[i]);
    }
//
    dBodySetPosition (carbody[1],WIDTH*0.5 ,STARTZ/**-HEIGHT*0.5**/,+0.5*LENGTH);
    dBodySetPosition (carbody[2],-WIDTH*0.5 ,STARTZ/**-HEIGHT*0.5**/,+0.5*LENGTH);
    
    dBodySetPosition (carbody[3],WIDTH*0.5 ,STARTZ/**-HEIGHT*0.5**/,-0.5*LENGTH);
    dBodySetPosition (carbody[4],-WIDTH*0.5 ,STARTZ/**-HEIGHT*0.5**/,-0.5*LENGTH);
//
//    
//    // front and back wheel hinges
    for (int i=0; i<4; i++) {
        carjoint[i] = dJointCreateHinge2 (world,0);
        dJointAttach (carjoint[i],me,carbody[i+1]);
        const dReal *a = dBodyGetPosition (carbody[i+1]);
        dJointSetHinge2Anchor (carjoint[i],a[0],a[1],a[2]);
        dJointSetHinge2Axes (carjoint[i], zunit, yunit);
        //dJointSetHinge2Axis1 (carjoint[i],0,1,0);  // Axis 1 that comes from the structure
        //dJointSetHinge2Axis2 (carjoint[i],0,0,1);  // Axis 2 where the wheels spin
    }
//
//    // set joint suspension
    for (int i=0; i<4; i++) {
        dJointSetHinge2Param (carjoint[i],dParamSuspensionERP,0.4);
        dJointSetHinge2Param (carjoint[i],dParamSuspensionCFM,0.8);
    }

    // lock back wheels along the steering axis
    for (int i=1; i<4; i++) {
        // set stops to make sure wheels always stay in alignment
        dJointSetHinge2Param (carjoint[i],dParamLoStop,0);
        dJointSetHinge2Param (carjoint[i],dParamHiStop,0);
//        // the following alternative method is no good as the wheels may get out
//        // of alignment:
//        //   dJointSetHinge2Param (joint[i],dParamVel,0);
//        //   dJointSetHinge2Param (joint[i],dParamFMax,dInfinity);
    }

    // create car space and add it to the top level space
    car_space = dSimpleSpaceCreate (space);
    dSpaceSetCleanup (car_space,0);
    dSpaceAdd (car_space,box[0]);
    dSpaceAdd (car_space,sphere[0]);
    dSpaceAdd (car_space,sphere[1]);
    dSpaceAdd (car_space,sphere[2]);
    dSpaceAdd (car_space,sphere[3]);
    
    
    
}


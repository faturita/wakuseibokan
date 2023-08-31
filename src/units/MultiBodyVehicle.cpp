//
//  MultiBodyVehicle.cpp
//  wakuseibokan
//
//  Created by Rodrigo Ramele on 1/12/17.
//

#include "MultiBodyVehicle.h"



void MultiBodyVehicle::init()
{
    Image* image = loadBMP("terrain/vtr.bmp");
    _textureBox = loadTexture(image);
    delete image;
    
    
    setForward(0,0,1);
    
}

int MultiBodyVehicle::getType()
{
    return 4;
}

EntityTypeId MultiBodyVehicle::getTypeId()
{
    return EntityTypeId::TMultiBodyVehicle;
}

void MultiBodyVehicle::doMaterial()
{
    GLfloat specref[] = { 1.0f, 1.0f, 1.0f, 1.0f};
    
    glEnable(GL_COLOR_MATERIAL);
    
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    
    glMaterialfv(GL_FRONT, GL_SPECULAR, specref);
    glMateriali(GL_FRONT, GL_SHININESS,128);
}




void MultiBodyVehicle::drawModel()
{
    drawModel(0,0,pos[0],pos[1],pos[2]);
}

void MultiBodyVehicle::doControl(Controller controller)
{
    //engine[0] = -controller.roll;
    //engine[1] = controller.yaw;
    //engine[2] = -controller.pitch;
    //steering = -controller.precesion;
    
    
    setThrottle(-controller.registers.pitch);
    
    xRotAngle = controller.registers.precesion;
    
    yRotAngle = controller.registers.roll;
    
}


void MultiBodyVehicle::drawDirectModel()
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



void MultiBodyVehicle::doDynamics()
{
    doDynamics(getBodyID());
}



void MultiBodyVehicle::doDynamics(dBodyID body)
{

//    dJointSetHinge2Param (carjoint[2],dParamVel,0);
//    dJointSetHinge2Param (carjoint[2],dParamFMax,1000);
//    dJointSetHinge2Param (carjoint[2],dParamLoStop,-0.75);
//    dJointSetHinge2Param (carjoint[2],dParamHiStop,0.75);
//    dJointSetHinge2Param (carjoint[2],dParamFudgeFactor,0.1);

//    dJointSetHinge2Param (carjoint[3],dParamVel,0);
//    dJointSetHinge2Param (carjoint[3],dParamFMax,1000);
//    dJointSetHinge2Param (carjoint[3],dParamLoStop,-0.75);
//    dJointSetHinge2Param (carjoint[3],dParamHiStop,0.75);
//    dJointSetHinge2Param (carjoint[3],dParamFudgeFactor,0.1);

//    dJointSetHinge2Param (carjoint[2],dParamVel2,getThrottle());
//    dJointSetHinge2Param (carjoint[2],dParamFMax2,1000);

//    dJointSetHinge2Param (carjoint[3],dParamVel2,getThrottle());
//    dJointSetHinge2Param (carjoint[3],dParamFMax2,1000);
    
    //dBodyAddRelForce (body,0, 0,getThrottle());
    dBodyAddForce(me, 0,9.81f,0);
    // This should be after the world step
    /// stuff
    dVector3 result;
    
    //dBodyAddRelForce(body, xRotAngle, 0, 0);
    
    dBodyVectorToWorld(body, 0,0,1,result);
    setForward(result[0],result[1],result[2]);
    
    const dReal *dBodyPosition = dBodyGetPosition(body);
    const dReal *dBodyRotation = dBodyGetRotation(body);
    
    setPos(dBodyPosition[0],dBodyPosition[1],dBodyPosition[2]);
    setLocation((float *)dBodyPosition, (float *)dBodyRotation);
    
}


void wheel(float x, float y, float z,float xx, float yy, float zz, float R[12])
{
    glPushMatrix();
    //glTranslatef(x, y, z);
    glTranslatef(xx, yy, zz);
    doTransform(R);
    //glRotatef(-180.0f, 1.0f, 0.0f, 0.0f);
    //glutSolidSphere(2.0, 20, 50);
    drawRectangularBox(0.5, 2.0f, 2.0f);
    glPopMatrix();
}

void MultiBodyVehicle::drawModel(float yRot, float xRot, float x, float y, float z)
{
    float f[3];
    f[0] = 0; f[1] = 0; f[2] = 0;
    
    
    glPushMatrix();
    glTranslatef(x, y, z);
    
    doTransform(f, R);
    
    drawArrow();
    
    glRotatef(-180.0f, 1.0f, 0.0f, 0.0f);
    //glRotatef(yRot, 0.0f, 1.0f, 0.0f);
    
    //glRotatef(xRot, 1.0f, 0.0f, 0.0f);
    
    doMaterial();
    //glutSolidSphere(4.0, 20, 50);
    drawRectangularBox(10.0, 2.0f, 30.0f);
    //_model->draw();
    
    
    
    float WIDTH = 10.0f;
    float LENGTH = 30.0f;

    const dReal *dBodyPosition1 = dBodyGetPosition(carbody[1]);
    const dReal *dBodyRotation1 = dBodyGetRotation(carbody[1]);
    wheel (x,y,z,dBodyPosition1[0],dBodyPosition1[1],dBodyPosition1[2], (float *)dBodyRotation1);
    const dReal *dBodyPosition2 = dBodyGetPosition(carbody[2]);
    const dReal *dBodyRotation2 = dBodyGetRotation(carbody[2]);
    wheel (x,y,z,dBodyPosition2[0],dBodyPosition2[1],dBodyPosition2[2], (float *)dBodyRotation2);
    const dReal *dBodyPosition3 = dBodyGetPosition(carbody[3]);
    const dReal *dBodyRotation3 = dBodyGetRotation(carbody[3]);
    wheel (x,y,z,dBodyPosition3[0],dBodyPosition3[1],dBodyPosition3[2], (float *)dBodyRotation3);
    const dReal *dBodyPosition4 = dBodyGetPosition(carbody[4]);
    const dReal *dBodyRotation4 = dBodyGetRotation(carbody[4]);
    wheel (x,y,z,dBodyPosition4[0],dBodyPosition4[1],dBodyPosition4[2], (float *)dBodyRotation4);
    
    
    //printf("%10.8f, %10.8f, %10.8f\n",dBodyPosition1[0],dBodyPosition1[1],dBodyPosition1[2]);
    
    glPopMatrix();
    

    
}

void MultiBodyVehicle::embody(dBodyID myBodySelf)
{
    
}

void MultiBodyVehicle::embody(dWorldID world, dSpaceID space)
{
    me = dBodyCreate(world);
    embody(me);
    //geom = dCreateSphere( space, 4.0f);
    //geom = dCreateBox( space, 10.0, 2.0f, 30.0f);
    dBodySetPosition (me,pos[0],pos[1],pos[2]);
    box[0] = dCreateBox (0,10.0f,2.0f,30.0f);
    dGeomSetBody(box[0] , me);

    dMass m;
    
    float LENGTH = 10;
    float WIDTH = 5;
    float HEIGHT = 5;
    
    float RADIUS=2;	// wheel radius
    float STARTZ=1;	// starting height of chassis
 
    
    
    dMassSetBox (&m,1,10.0, 2.0f, 30.0f);
    //dMassSetSphere (&m,1,RADIUS);
    dMassAdjust (&m,CMASS);
    dBodySetMass (me,&m);
    //box[0] = dCreateBox (0,WIDTH,HEIGHT,LENGTH);
    //dGeomSetBody(box[0], me);
    
    
    // wheel bodies
    for (int i=1; i<=4; i++) {
        carbody[i] = dBodyCreate (world);
        dQuaternion q;
        dQFromAxisAndAngle (q,1,0,0,0);
        dBodySetQuaternion (carbody[i],q);
        dMassSetSphere (&m,1,RADIUS);//dMassSetCylinder(&m,1,1,RADIUS, 3.0f);
        dMassAdjust (&m,WMASS);
        dBodySetMass (carbody[i],&m);
        sphere[i-1] = dCreateSphere (0,RADIUS);//dCreateCylinder(0,RADIUS,3.0f); //
        dGeomSetBody (sphere[i-1],carbody[i]);
    }

    WIDTH = 10.0f;
    LENGTH = 30.0f;
    
    dBodySetPosition (carbody[1], WIDTH*0.5+RADIUS  ,-1,+0.3*LENGTH);
    dBodySetPosition (carbody[2],-WIDTH*0.5-RADIUS  ,-1,+0.3*LENGTH);
    
    dBodySetPosition (carbody[3], WIDTH*0.5+RADIUS  ,-1,-0.3*LENGTH);
    dBodySetPosition (carbody[4],-WIDTH*0.5-RADIUS  ,-1,-0.3*LENGTH);

    //    //
//    //
    //    // front and back wheel hinges
//    for (int i=0; i<4; i++) {
//        carjoint[i] = dJointCreateHinge2 (world,0);
//        dJointAttach (carjoint[i],me,carbody[i+1]);
//        const dReal *a = dBodyGetPosition (carbody[i+1]);
//        dJointSetHinge2Anchor (carjoint[i],a[0],a[1],a[2]);
//        dJointSetHinge2Axis1 (carjoint[i],0,1,0);  // Axis 1 that comes from the structure
//        dJointSetHinge2Axis2 (carjoint[i],0,0,1);  // Axis 2 where the wheels spin
//    }
////    //
////    //    // set joint suspension
//    for (int i=0; i<4; i++) {
//        dJointSetHinge2Param (carjoint[i],dParamSuspensionERP,0.4);
//        dJointSetHinge2Param (carjoint[i],dParamSuspensionCFM,0.8);
//    }
//
//    // lock back wheels along the steering axis
//    for (int i=0; i<4; i++) {
//        // set stops to make sure wheels always stay in alignment
//        dJointSetHinge2Param (carjoint[i],dParamLoStop,0);
//        dJointSetHinge2Param (carjoint[i],dParamHiStop,0);
//        //        // the following alternative method is no good as the wheels may get out
//        //        // of alignment:
//        //        //   dJointSetHinge2Param (joint[i],dParamVel,0);
//        //        //   dJointSetHinge2Param (joint[i],dParamFMax,dInfinity);
//    }
//
//    // create car space and add it to the top level space
    car_space = dSimpleSpaceCreate (space);
    dSpaceSetCleanup (car_space,0);
    dSpaceAdd (car_space,box[0]);
    dSpaceAdd (car_space,sphere[0]);
    dSpaceAdd (car_space,sphere[1]);
    dSpaceAdd (car_space,sphere[2]);
    dSpaceAdd (car_space,sphere[3]);

    
    
    
}

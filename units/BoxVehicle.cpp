//
//  BoxVehicle.cpp
//  mycarrier
//
//  Created by Rodrigo Ramele on 24/05/14.
//  Copyright (c) 2014 Baufest. All rights reserved.
//

#include "BoxVehicle.h"


void BoxVehicle::init()
{

    Image* image = loadBMP("vtr.bmp");
    _textureBox = loadTexture(image);
    delete image;
    
    boxRotatingAngle = 0;
    
    setForward(0,0,1);
    

}

void BoxVehicle::embody(dBodyID myBodySelf)
{
	dMass m;
    
    float myMass = 1.0f;
    float radius = 1.0f;
    float length = 1.0f;

	dBodySetPosition(myBodySelf, pos[0], pos[1], pos[2]);
	dMassSetBox(&m,1,length,length,length);
	dMassAdjust(&m, myMass*1.0f);
	dBodySetMass(myBodySelf,&m);
    
    me = myBodySelf;
    
}

dBodyID BoxVehicle::getBodyID()
{
    return me;
}

void BoxVehicle::doMaterial()
{
    /**
     GLfloat specref[] = { 1.0f, 1.0f, 1.0f, 1.0f};
     
     glEnable(GL_COLOR_MATERIAL);
     
     glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
     
     glMaterialfv(GL_FRONT, GL_SPECULAR, specref);
     glMateriali(GL_FRONT, GL_SHININESS,128);
     **/
}

void BoxVehicle::doControl(Controller controller)
{
    engine[0] = controller.pitch;
    engine[1] = controller.yaw;
    engine[2] = controller.roll;
}

void BoxVehicle::drawModel(float yRot, float xRot, float x, float y, float z)
{

}

void BoxVehicle::drawModel()
{
    int x=0, y=0, z=0;
    
    float xx = pos[0];
    float yy = pos[1];
    float zz = pos[2];
    
    const float BOX_SIZE = 7.0f; //The length of each side of the cube
    
    //glLoadIdentity();
    glPushMatrix();
    glTranslatef(xx,yy,zz);
    glRotatef(boxRotatingAngle, 0.0f, 0.0f, 1.0f);
    glBegin(GL_QUADS);
    
    //Top face
    glColor3f(1.0f, 1.0f, 0.0f);
    glNormal3f(0.0, 1.0f, 0.0f);
    glVertex3f(-BOX_SIZE / 2 + x, BOX_SIZE / 2 + y, -BOX_SIZE / 2 + z);
    glVertex3f(-BOX_SIZE / 2 + x, BOX_SIZE / 2 + y, BOX_SIZE / 2 + z);
    glVertex3f(BOX_SIZE / 2 + x, BOX_SIZE / 2 + y, BOX_SIZE / 2 + z);
    glVertex3f(BOX_SIZE / 2 + x, BOX_SIZE / 2 + y, -BOX_SIZE / 2 + z);
    
    //Bottom face
    glColor3f(1.0f, 0.0f, 1.0f);
    glNormal3f(0.0, -1.0f, 0.0f);
    glVertex3f(-BOX_SIZE / 2 + x, -BOX_SIZE / 2 + y, -BOX_SIZE / 2 + z);
    glVertex3f(BOX_SIZE / 2 + x, -BOX_SIZE / 2 + y, -BOX_SIZE / 2 + z);
    glVertex3f(BOX_SIZE / 2 + x, -BOX_SIZE / 2 + y, BOX_SIZE /2 + z);
    glVertex3f(-BOX_SIZE / 2 + x, -BOX_SIZE / 2 + y, BOX_SIZE / 2 + z);
    
    //Left face
    glNormal3f(-1.0, 0.0f, 0.0f);
    glColor3f(0.0f, 1.0f, 1.0f);
    glVertex3f(-BOX_SIZE / 2 + x, -BOX_SIZE / 2 + y, -BOX_SIZE / 2 + z);
    glVertex3f(-BOX_SIZE / 2 + x, -BOX_SIZE / 2 + y, BOX_SIZE / 2 + z);
    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(-BOX_SIZE / 2 + x, BOX_SIZE / 2 + y, BOX_SIZE / 2 + z);
    glVertex3f(-BOX_SIZE / 2 + x, BOX_SIZE / 2 + y, -BOX_SIZE / 2 + z);
    
    //Right face
    glNormal3f(1.0, 0.0f, 0.0f);
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(BOX_SIZE / 2 + x, -BOX_SIZE / 2 + y, -BOX_SIZE / 2 + z);
    glVertex3f(BOX_SIZE / 2 + x, BOX_SIZE / 2 + y, -BOX_SIZE / 2 + z);
    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3f(BOX_SIZE / 2 + x, BOX_SIZE / 2 + y, BOX_SIZE / 2 + z);
    glVertex3f(BOX_SIZE / 2 + x, -BOX_SIZE / 2 + y, BOX_SIZE / 2 + z);
    
    glEnd();
    
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, _textureBox);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glColor3f(1.0f, 1.0f, 1.0f);
    
    glBegin(GL_QUADS);
    
    //Front face
    glNormal3f(0.0, 0.0f, 1.0f);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-BOX_SIZE / 2 + x, -BOX_SIZE / 2 + y, BOX_SIZE / 2 + z);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(BOX_SIZE / 2 + x, -BOX_SIZE / 2 + y, BOX_SIZE / 2 + z);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(BOX_SIZE / 2 + x, BOX_SIZE / 2 + y, BOX_SIZE / 2 + z);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(-BOX_SIZE / 2 + x, BOX_SIZE / 2 + y, BOX_SIZE / 2 + z);
    
    //Back face
    glNormal3f(0.0, 0.0f, -1.0f);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-BOX_SIZE / 2+ x, -BOX_SIZE / 2+ y, -BOX_SIZE / 2+ z);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(-BOX_SIZE / 2+ x, BOX_SIZE / 2+ y, -BOX_SIZE / 2+ z);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(BOX_SIZE / 2+ x, BOX_SIZE / 2+ y, -BOX_SIZE / 2+ z);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(BOX_SIZE / 2+ x, -BOX_SIZE / 2+ y, -BOX_SIZE / 2+ z);
    
    glEnd();
    glPopMatrix();
    
    //boxangle += 0.5f;
    
    //if (boxangle >= 360.f)
      //  boxangle = 0.0f;
}

void BoxVehicle::drawDirectModel()
{

}

void BoxVehicle::doDynamics(dBodyID body)
{
    me = body;
    doDynamics();
}

void BoxVehicle::stop()
{
    engine[0]=engine[1]=engine[2]=0;
}

void BoxVehicle::doDynamics()
{
    
	// This should be after the world step
	/// stuff
    //dVector3 result;
    
    //dBodyVectorToWorld(me, 0,0,1,result);
    //setForward(result[0],result[1],result[2]);
    if (fabs(engine[0])>0 || fabs(engine[1])>0 || fabs(engine[2])>0 )
    {
        //printf("Applying force %10.8f %10.8f %10.8f \n", engine[0],engine[1],engine[2]);
        dBodyAddForce(me, engine[0],engine[1], engine[2]);
    }
    
    if (tweakOde) {dBodyAddForce(me, 0, 0, 0.3);tweakOde=(!tweakOde);}
    
    dBodyAddForce(me, 0,9.81f,0);
    
	const dReal *dBodyPosition = dBodyGetPosition(me);
	const dReal *dBodyRotation = dBodyGetRotation(me);
    
	setPos(dBodyPosition[0],dBodyPosition[1],dBodyPosition[2]);
	setLocation((float *)dBodyPosition, (float *)dBodyRotation);
    
}

//
//  carrier.cpp
//  mycarrier
//
//  Created by Rodrigo Ramele on 22/05/14.
//  Copyright (c) 2014 Baufest. All rights reserved.
//

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <stdarg.h>
#include <math.h>

#include <GLUT/glut.h>

#include <ode/ode.h>

/**
#include "FractalNoise.h"
#include "terrain/imageloader.h"
#include "md2model.h"
#include "terrain/Terrain.h"


#include "carrier/yamathutil.h"




#include "odeutils.h"
#include "units/Manta.h"
#include "units/Walrus.h"



#include "carrier.h"
**/

#include "font/DrawFonts.h"

#include "carrier/yamathutil.h"



#include "usercontrols.h"
#include "camera.h"

#include "openglutils.h"

#include "keplerivworld.h"

#include "units/BoxVehicle.h"


extern  Controller controller;
extern  Camera Camera;


/* dynamics and collision objects */

extern dWorldID world;
extern dSpaceID space;
extern dBodyID body[NUM];
extern dJointID joint[NUM-1];
extern dJointGroupID contactgroup;
extern dGeomID sphere[NUM];

extern BoxVehicle _boxVehicle1;
extern BoxVehicle _boxVehicle2;

void drawHUD()
{
    // This will make things dark.
    
	//glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, 1200, 800, 0, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glColor4f(1.0f, 1.0f, 1.0f, 1);
	glDisable(GL_DEPTH_TEST);
	glRotatef(180.0f,0,0,1);
	glRotatef(180.0f,0,1,0);
    
    char str[256];

    
    if (isnan(Camera.pos[0])) exit(1);
    
    float fps = getFPS();
    
    sprintf (str, "fps %4.2f (%10.8f,%10.8f,%10.8f)\n", fps, Camera.pos[0],Camera.pos[1],Camera.pos[2]);
	// width, height, 0 0 upper left
	drawString(0,-30,1,str,0.2f);
    
	//glPrint(1,10,10,"HUD");
    
	//glRectf(400.0f,400.0f,450.0f,400.0f);
    
	sprintf (str, "Speed:%10.8f - X,Y,Z,P (%10.8f,%10.6f,%10.6f,%10.6f)\n", Camera.dx, controller.roll,controller.pitch,controller.yaw,controller.precesion);
	drawString(0,-60,1,str,0.2f);
    
	sprintf (str, "Vehicle:%d  - Thrust:%10.8f\n", controller.controlling,controller.thrust);
	drawString(0,-90,1,str,0.2f);
    
    
    glMatrixMode(GL_MODELVIEW);
	glPushMatrix(); {
		glTranslatef(0, -400, 1);
        
        glLineWidth(2.5);
        //glColor3f(1.0, 0.0, 0.0);
        glBegin(GL_LINES);
        glVertex3f(590, 0.0, 0.0);
        glVertex3f(690, 0, 0);
        glEnd();
        
        
        glBegin(GL_LINES);
        glVertex3f(590, Camera.yAngle, 0.0);
        glVertex3f(690, + Camera.yAngle, 0);
        glEnd();
        
	} glPopMatrix();
    
    

  
    
	glEnable(GL_DEPTH_TEST);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
}


void drawScene() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
    
    
    drawLightning();
    
    Vec3f up,pos,forward;
    
    //Camera.getViewPort(up,pos,forward);
    
    _boxVehicle1.getViewPort(up,pos,forward);
    
    //drawSky(pos[0],pos[1],pos[2]);
    
    Camera.lookAtFrom(up, pos, forward);
    
    Camera.setPos(pos);

    drawFloor(Camera.pos[0],0.0f,Camera.pos[2]);


    drawArrow(100.0);
    
    drawBox(10,10,1400);
    drawBox(10,10,1300);
    drawBox(10,10,1200);
    drawBox(10,10,1100);
    drawBox(10,10,1000);
    drawBox(10,10,900);
    drawBox(10,10,800);
    drawBox(10,10,700);
    drawBox(10,10,600);
    drawBox(10,10,500);
    drawBox(10,10,400);
    drawBox(10,10,300);
    drawBox(10,20,-80);
    
    drawBox(10,10,10);
    drawBox(-10,-10,-10);
    
    
    // Draw lands
    
    // Draw objects
    _boxVehicle1.drawModel();
    _boxVehicle2.drawModel();
    
    
    // GO with the HUD
    drawHUD();
    
	glDisable(GL_TEXTURE_2D);
	
	glutSwapBuffers();
}



void initRendering() {
	// Lightning
    
	glEnable(GL_LIGHTING);
    
	glEnable(GL_LIGHT0);

    
	// Normalize the normals (this is very expensive).
	glEnable(GL_NORMALIZE);
    
    
	// Do not show hidden faces.
	glEnable(GL_DEPTH_TEST);
    
	glEnable(GL_COLOR_MATERIAL);
    
	// Do not show the interior faces....
	//glEnable(GL_CULL_FACE);
    
	// Blue sky !!!
	glClearColor(0.7f, 0.9f, 1.0f, 1.0f);
    
    // Initialize scene textures.
    initTextures();
    
    //Init lands, fixed objects, and objects
    _boxVehicle1.init();
    _boxVehicle1.setPos(0.0f,40.0f,300.0f);
    
    _boxVehicle2.init();
    _boxVehicle2.setPos(300.0f,40.0f,300.0f);
    
}



void handleResize(int w, int h) {
    printf("Handling Resize: %d, %d \n", w, h);
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, (float)w / (float)h, 1.0, Camera.pos[2]+ 10450.0f  /**+ yyy**/);
}


void update(int value)
{
	// Derive the control to the correct object
    
    if (!controller.pause)
	{
		//_manta.doDynamics(body[0]);
		//_walrus.doDynamics(body[1]);
        switch (controller.controlling) {
            case 1:
                _boxVehicle1.doControl(controller);
                break;
            case 2:
                _boxVehicle2.doControl(controller);
                break;
                
            default:
                break;
        }
        
        _boxVehicle1.doDynamics();
        _boxVehicle2.doDynamics();
		// Ok, done with the dynamics
        
		dSpaceCollide (space,0,&nearCallback);
		dWorldStep (world,0.05);
        
		/* remove all contact joints */
		//dJointGroupEmpty (contactgroup);
	}
    
	glutPostRedisplay();
	glutTimerFunc(25, update, 0);
}




int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	//glutInitWindowSize(1200, 800);
    
	glutCreateWindow("Carrier Command");
	glutFullScreen();
    
	//Initialize all the models and structures.
	initRendering();
    
    //dAllocateODEDataForThread(dAllocateMaskAll);
	initWorldModelling();
    
	// OpenGL callback functions.
	glutDisplayFunc(drawScene);
	glutKeyboardFunc(handleKeypress);
	glutSpecialFunc(handleSpecKeypress);
    
	// Resize callback function.
	glutReshapeFunc(handleResize);
    
	//adding here the mouse processing callbacks
	glutMouseFunc(processMouse);
	glutMotionFunc(processMouseActiveMotion);
	glutPassiveMotionFunc(processMousePassiveMotion);
	glutEntryFunc(processMouseEntry);
    
	// this is the first time to call to update.
	glutTimerFunc(25, update, 0);
    
	// main loop, hang here.
	glutMainLoop();
	return 0;
}
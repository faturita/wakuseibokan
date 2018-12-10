//
//  carrier.cpp
//  mycarrier
//
//  Created by Rodrigo Ramele on 22/05/14.
//  Copyright (c) 2014 Baufest. All rights reserved.
//

#define dSINGLE

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


#import <OpenAL/al.h>
#import <OpenAL/alc.h>
#import <AudioToolbox/AudioToolbox.h>

#import "ThreeMaxLoader.h"


/**
#include "FractalNoise.h"
#include "terrain/imageloader.h"
#include "md2model.h"
#include "terrain/Terrain.h"


#include "math/yamathutil.h"




#include "odeutils.h"
#include "units/Manta.h"
#include "units/Walrus.h"



#include "carrier.h"
**/

#include "font/DrawFonts.h"

#include "math/yamathutil.h"



#include "usercontrols.h"
#include "camera.h"

#include "openglutils.h"

#include "keplerivworld.h"

#include "units/BoxVehicle.h"
#include "units/Manta.h"

#include "openglutils.h"

#include "imageloader.h"
#include "terrain/Terrain.h"

extern  Controller controller;
extern  Camera Camera;


/* dynamics and collision objects */

extern dWorldID world;
extern dSpaceID space;
extern dBodyID body[NUM];
extern dJointID joint[NUM-1];
extern dJointGroupID contactgroup;
extern dGeomID sphere[NUM];

extern std::vector<Vehicle*> vehicles;

//extern BoxIsland _boxIsland;

extern std::vector<BoxIsland*> islands;





void disclaimer()
{
    printf ("惑星母艦\n");
    printf ("Warfare on the seas of Kepler IV\n");
}

/**
 * Draw the Head Up Display 
 *
 **/
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
    
    sprintf (str, "fps %4.2f  Cam: (%5.2f,%5.2f,%5.2f)\n", fps, Camera.pos[0],Camera.pos[1],Camera.pos[2]);
	// width, height, 0 0 upper left
	drawString(0,-30,1,str,0.2f);
    
	//glPrint(1,10,10,"HUD");
    
	//glRectf(400.0f,400.0f,450.0f,400.0f);
    
    float speed=0;
    
    if (controller.controlling >0)
        speed = vehicles[controller.controlling-1]->getSpeed();
    
	sprintf (str, "Speed:%10.2f - X,Y,Z,P (%5.2f,%5.2f,%5.2f,%5.2f)\n", speed, controller.roll,controller.pitch,controller.yaw,controller.precesion);
	drawString(0,-60,1,str,0.2f);
    
	sprintf (str, "Vehicle:%d  - Thrust:%5.2f\n", controller.controlling,controller.thrust);
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
    
    int ctrling = 0;
    
    if (controller.controlling >0 )
    {
        ctrling = controller.controlling-1;
        vehicles[ctrling]->getViewPort(up,pos,forward);
        
        Vec3f up2,pos2;
        //Camera.getViewPort(up2,pos2,forward);
    } else
    {
        Camera.getViewPort(up,pos,forward);
        
        if (Camera.dx!=0) {
            pos[0]+=(forward[0]);
            pos[1]+=(forward[1]);
            pos[2]+=(forward[2]);
            
            pos[2]+=controller.pitch;
            pos[0]+=controller.roll;
        }
    }
    
    drawSky(pos[0],pos[1],pos[2]);
    
    Camera.lookAtFrom(up, pos, forward);
    
    // Sets the camera and that changes the floor position.
    Camera.setPos(pos);
    
    // Draw CENTER OF coordinates RGB=(x,y,b)
    glPushAttrib(GL_CURRENT_BIT);
    drawArrow(3);
    glPopAttrib();
    
    // Floor is changing color.
    glPushAttrib(GL_CURRENT_BIT);
    drawFloor(Camera.pos[0],0.0f,Camera.pos[2]);
    glPopAttrib();
    
    //drawBoxIsland(300,5,300,1000,10);
    
    // Draw all terrain islands and so on.
    //_boxIsland.draw(300,5,300,1000,10);
    
    // Draw islands.
    for (int i=0; i<islands.size(); i++) {
        (islands[i]->draw());
    }

    
    // Debug: Until the engine is complete I need these boxes to tell me where I am.
    
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
    
    
    // EXPERIMENTALLY: Draw 3DS models
    obj_type object;
    //CThreeMaxLoader::Load3DS(&object,"watertower.3ds");
    //CThreeMaxLoader::draw3DSModel(object,300.0,20.0,300.0,1);

    
    
    // Draw vehicles and objects
    for (int i=0; i<vehicles.size(); i++) {
        (vehicles[i]->drawModel());
    }
    
    // GO with the HUD
    drawHUD();
    
	glDisable(GL_TEXTURE_2D);
	
	glutSwapBuffers();
}



void initRendering() {
	// Lightning
    
	glEnable(GL_LIGHTING);
    
    // Lighting not working.
	glEnable(GL_LIGHT0);
    drawLightning();
    
	// Normalize the normals (this is very expensive).
	glEnable(GL_NORMALIZE);
    
    
	// Do not show hidden faces.
	glEnable(GL_DEPTH_TEST);
    
    
    // Enable wireframes
    //glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
    
    
    glShadeModel(GL_SMOOTH); // Type of shading for the polygons
    
	glEnable(GL_COLOR_MATERIAL);
    
	// Do not show the interior faces....
	//glEnable(GL_CULL_FACE);
    
	// Blue sky !!!
	glClearColor(0.7f, 0.9f, 1.0f, 1.0f);
    
    // Initialize scene textures.
    initTextures();
    
}


void handleResize(int w, int h) {
    printf("Handling Resize: %d, %d \n", w, h);
	glViewport(0, 0, w, h);
    
    // ADDED
    glOrtho( 0, w, 0, h, -1, 1);
    
    
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, (float)w / (float)h, 1.0, Camera.pos[2]+ 10450.0f  /**+ yyy**/);
}


void update(int value)
{
	// Derive the control to the correct object
    
    if (controller.isInterrupted())
    {
        endWorldModelling();
        exit(0);
    }
    if (!controller.pause)
	{
        
        if (controller.controlling>0)
        {
            vehicles[controller.controlling-1]->doControl(controller);
        }
        
        for (int i=0; i<vehicles.size(); i++) {
            vehicles[i]->doDynamics();
        }

        
		// Ok, done with the dynamics
        
		dSpaceCollide (space,0,&nearCallback);
		dWorldStep (world,0.05);
        
		/* remove all contact joints */
		dJointGroupEmpty (contactgroup);
	}
    
	glutPostRedisplay();
	glutTimerFunc(25, update, 0);
}




int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    
    disclaimer();
	glutCreateWindow("Wakuseibokan");
    
    if (argc>1 && strcmp(argv[1],"-d")==0)
        glutInitWindowSize(1200, 800);
    else
        glutFullScreen();
    

    //dAllocateODEDataForThread(dAllocateMaskAll);
    initWorldModelling();
    
    //Initialize all the models and structures.
    initRendering();
    
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

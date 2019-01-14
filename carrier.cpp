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
#include <mutex>

#include "ThreeMaxLoader.h"

#include "font/DrawFonts.h"

#include "math/yamathutil.h"

#include "container.h"

#include "usercontrols.h"
#include "camera.h"

#include "openglutils.h"

#include "keplerivworld.h"

#include "units/BoxVehicle.h"
#include "units/Manta.h"

#include "openglutils.h"

#include "imageloader.h"
#include "terrain/Terrain.h"

#include "structures/Structure.h"

#include "map.h"

extern  Controller controller;
extern  Camera Camera;


/* dynamics and collision objects */

extern dWorldID world;
extern dSpaceID space;
extern dBodyID body[NUM];
extern dJointID joint[NUM-1];
extern dJointGroupID contactgroup;
extern dGeomID sphere[NUM];

extern container<Vehicle*> vehicles;

//extern BoxIsland _boxIsland;

extern std::vector<BoxIsland*> islands;

extern std::vector<Structure*> structures;

extern std::vector<Vehicle*> controlables;

extern std::vector<std::string> messages;


// @FIXME Change
extern GLuint _textureBox;
extern GLuint _textureMetal;


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
    
    sprintf (str, "fps %4.2f  Cam: (%5.2f\t,%5.2f\t,%5.2f\t)\n", fps, Camera.pos[0],Camera.pos[1],Camera.pos[2]);
	// width, height, 0 0 upper left
	drawString(0,-30,1,str,0.2f);
    
	//glPrint(1,10,10,"HUD");
    
	//glRectf(400.0f,400.0f,450.0f,400.0f);
    
    float speed=0;
    
    if (controller.controlling >0)
        speed = controlables[controller.controlling-1]->getSpeed();
    
    sprintf (str, "Speed:%10.2f - X,Y,Z,P (%5.2f,%5.2f,%5.2f,%5.2f)\n", speed, controller.registers.roll,controller.registers.pitch,controller.registers.yaw,controller.registers.precesion);
	drawString(0,-60,1,str,0.2f);
    
    sprintf (str, "Vehicle:%d  - Thrust:%5.2f\n", controller.controlling,controller.registers.thrust);
	drawString(0,-90,1,str,0.2f);

    if (controller.isTeletype())
    {
        sprintf(str, ">>>%s",controller.str.c_str());
        drawString(0,-180,1,str,0.2f);
    }

    // Message board
    if (messages.size()>0)
    {
        for(int i=0;i<messages.size();i++)
        {
            std::string line = messages[i];
            if (i==0)
                drawString(0,-700-i*25,1,(char*)line.c_str(),0.2f,1.0f,1.0f,0.0f);
            else
                drawString(0,-700-i*25,1,(char*)line.c_str(),0.2f);
        }

    }

    
    // Displays the target mark at the center. The position of the center cross depends on camera angles.
    glMatrixMode(GL_MODELVIEW);
	glPushMatrix(); {
		glTranslatef(0, -400, 1);
        
        glLineWidth(2.5);

        int uc=550;
        int lc=0+50;
        int w=40;
        int h=40;


        glBegin(GL_LINES);
        glVertex3f(uc,    lc-h, 0);
        glVertex3f(uc,   lc, 0);
        glEnd();

        glBegin(GL_LINES);
        glVertex3f(uc,     lc, 0);
        glVertex3f(uc+w,   lc, 0);
        glEnd();

        glBegin(GL_LINES);
        glVertex3f(uc, lc-100+h, 0);
        glVertex3f(uc, lc-100, 0);
        glEnd();

        glBegin(GL_LINES);
        glVertex3f(uc, lc-100,0);
        glVertex3f(uc+w, lc-100, 0);
        glEnd();

        glBegin(GL_LINES);
        glVertex3f(uc+100-w, lc, 0);
        glVertex3f(uc+100, lc, 0);
        glEnd();

        glBegin(GL_LINES);
        glVertex3f(uc+100, lc ,0);
        glVertex3f(uc+100, lc-h, 0);
        glEnd();

        glBegin(GL_LINES);
        glVertex3f(uc+100, lc-100+h, 0);
        glVertex3f(uc+100,lc-100,0);
        glEnd();

        glBegin(GL_LINES);
        glVertex3f(uc+100, lc-100, 0);
        glVertex3f(uc+100-w, lc-100, 0);
        glEnd();

        glBegin(GL_LINES);
        glVertex3f(uc+50-10, Camera.yAngle, 0.0);
        glVertex3f(uc+50-2, + Camera.yAngle, 0);
        glEnd();

        glBegin(GL_LINES);
        glVertex3f(uc+100-50+10, Camera.yAngle, 0.0);
        glVertex3f(uc+100-50+2, + Camera.yAngle, 0);
        glEnd();

        glBegin(GL_LINES);
        glVertex3f(uc+50, Camera.yAngle-10, 0.0);
        glVertex3f(uc+50, + Camera.yAngle-2, 0);
        glEnd();


        glBegin(GL_LINES);
        glVertex3f(uc+50, Camera.yAngle+10, 0.0);
        glVertex3f(uc+50, + Camera.yAngle+2, 0);
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
        controlables[ctrling]->getViewPort(up,pos,forward);
        
        Vec3f up2,pos2;
        //Camera.getViewPort(up2,pos2,forward);

        if (controlables[ctrling]->getType() == 3)
            Camera.yAngle = ((Manta*)controlables[ctrling])->alpha*100;
    } else
    {
        Camera.getViewPort(up,pos,forward);
        
        if (Camera.dx!=0) {
            pos[0]+=(forward[0]);
            pos[1]+=(forward[1]);
            pos[2]+=(forward[2]);
            
            pos[2]+=controller.registers.pitch;
            pos[0]+=controller.registers.roll;
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
    drawBox(10,10,1600);
    drawBox(10,10,1500);
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

    // Draw island structures.
    draw3DSModel("units/missile.3ds",1200.0+100,15.0,700.0+300.0,1,_textureBox);

    //draw3DSModel("structures/turretbase.3ds",0.0,20.0,-1000,1,_textureBox);


    for(int i=0;i<structures.size();i++)
    {
        // @NOTE Textures are loaded up to this point so they are set into the objects now.
        structures[i]->setTexture(_textureMetal);
        structures[i]->drawModel();
    }
    //draw3DSModel("structures/runway.3ds",0.0f,5.0,0.0f,10,_textureBox);


    // Draw vehicles and objects
    synchronized(vehicles.m_mutex)
    {
        for(size_t i=vehicles.first();vehicles.exists(i);i=vehicles.next(i))
        {
            (vehicles[i]->drawModel());
        }
    }
    
    // GO with the HUD
    switch (controller.view)
    {
    case 1: drawHUD();break;
    case 2: drawMap();break;
    }
    
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

static bool did=false;

void update(int value)
{
	// Derive the control to the correct object

    if (!did)
    {
        dAllocateODEDataForThread(dAllocateMaskAll);
        did=true;
    }
    
    if (controller.isInterrupted())
    {
        endWorldModelling();
        exit(0);
    }
    if (!controller.pause)
	{
        
        if (controller.controlling>0)
        {
            controlables[controller.controlling-1]->doControl(controller);
        }

        // Autocontrol (AI).
        for(int i=0;i<controlables.size();i++)
        {
            if (controlables[i]->isAuto())
                    controlables[i]->doControl();
        }


        //printf("Elements alive now: %d\n", vehicles.size());
        // As the sync problem only arises when you delete something, there's no problem here.
        for(size_t i=vehicles.first();vehicles.exists(i);i=vehicles.next(i)) {
            vehicles[i]->doDynamics();
            vehicles[i]->tick();
        }


        synchronized(vehicles.m_mutex)
        {
            for(size_t i=vehicles.first();vehicles.exists(i);i=vehicles.next(i))
            {
                //printf("Type and ttl: %d, %d\n", vehicles[i]->getType(),vehicles[i]->getTtl());
                if (vehicles[i]->getType()==5 && vehicles[i]->getTtl()==0)
                {
                    //printf("Eliminating....\n");
                    dBodyDisable(vehicles[i]->getBodyID());
                    vehicles.erase(i);
                    //delete vehicles[i];
                    //dBodyDestroy(vehicles[i]->getBodyID());
                }
            }
        }

        
		// Ok, done with the dynamics
        
        dSpaceCollide (space,0,&nearCallback);
        dWorldStep (world,0.05);
        

        //dWorldQuickStep()

		/* remove all contact joints */
		dJointGroupEmpty (contactgroup);
	}
    
	glutPostRedisplay();
    // @NOTE: update time should be adapted to real FPS.
    glutTimerFunc(25, update, 0);
}




int main(int argc, char** argv) {
	glutInit(&argc, argv);

    srand (time(NULL));

    // Switch up OpenGL version (at the time of writing compatible with 2.1)
    if (true)
        glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    else
        glutInitDisplayMode (GLUT_3_2_CORE_PROFILE | GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    
    disclaimer();
	glutCreateWindow("Wakuseibokan");
    
    if (argc>1 && strcmp(argv[1],"-d")==0)
        glutInitWindowSize(1200, 800);
    else
        glutFullScreen();

    // OpenGL Configuration information
    /* get version info */
    const GLubyte* renderer;
    const GLubyte* version;

    renderer = glGetString (GL_RENDERER);
    version = glGetString (GL_VERSION);
    printf ("Renderer: %s\n", renderer);
    printf ("OpenGL version supported: %s\n", version);
    

    // Initialize ODE, create islands, structures and populate the world.
    initWorldModelling();

    const char *conf = dGetConfiguration ();

    printf("ODE Configuration: %s\n", conf);

    // Draw vehicles and objects
    printf("Size %d\n", vehicles.size());
    synchronized(vehicles.m_mutex)
    {
        for(size_t i=vehicles.first();vehicles.exists(i);i=vehicles.next(i))
        {
            printf("Index (%d) %d - %d\n", i, vehicles[i]->getType(), vehicles[i]->getTtl());
        }
    }

    //vehicles.erase(4);

    printf("Size %d\n", vehicles.size());
    synchronized(vehicles.m_mutex)
    {
        for(size_t i=vehicles.first();vehicles.exists(i);i=vehicles.next(i))
        {
            printf("Body ID (%p) Index (%d) %d - %d\n", (void*)vehicles[i]->getBodyID(), i, vehicles[i]->getType(), vehicles[i]->getTtl());
        }
    }

    //unsigned long *a = (unsigned long*)dBodyGetData(vehicles[2]->getBodyID());

    //printf("Manta is located in %lu\n",*a);
    
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

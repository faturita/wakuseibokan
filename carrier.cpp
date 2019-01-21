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

#include "sounds/sounds.h"

#include "keplerivworld.h"

#include "units/BoxVehicle.h"
#include "units/Manta.h"

#include "openglutils.h"

#include "imageloader.h"
#include "terrain/Terrain.h"

#include "structures/Structure.h"
#include "structures/Warehouse.h"
#include "structures/Hangar.h"
#include "structures/Runway.h"
#include "structures/Laserturret.h"
#include "structures/CommandCenter.h"
#include "structures/Turret.h"

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

extern container<Vehicle*> entities;

extern std::vector<BoxIsland*> islands;

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
    
    float speed=0, health=0;
    
    if (controller.controlling != CONTROLLING_NONE)
    {
        speed = entities[controller.controlling]->getSpeed();
        health = entities[controller.controlling]->getHealth();
    }
    sprintf (str, "Speed:%10.2f - X,Y,Z,P (%5.2f,%5.2f,%5.2f,%5.2f)\n", speed, controller.registers.roll,controller.registers.pitch,controller.registers.yaw,controller.registers.precesion);
	drawString(0,-60,1,str,0.2f);
    
    sprintf (str, "Vehicle:%d  - Thrust:%5.2f - Health: %5.2f\n", entities.indexOf(controller.controlling),controller.registers.thrust, health);
	drawString(0,-90,1,str,0.2f);

    if (controller.isTeletype())
    {
        sprintf(str, ">>>%s",controller.str.c_str());
        drawString(0,-180,1,str,0.2f);
    }

    static int mbrefresher = 1000;
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

        if (mbrefresher--<=0)
        {
            messages.pop_back();
            mbrefresher = 1000;
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

        Vec3f f = (Camera.getForward().normalize())*30;

        f = (Camera.fw.normalize())*30;

        int cx=1150, cy=350;

        glBegin(GL_LINES);
        glVertex3f(cx-50,  +cy+50, 0.0);
        glVertex3f(cx-50,  +cy-50, 0.0);
        glEnd();

        glBegin(GL_LINES);
        glVertex3f(cx-50,  +cy+50, 0.0);
        glVertex3f(cx+50,  +cy+50, 0.0);
        glEnd();

        glBegin(GL_LINES);
        glVertex3f(cx+50,  +cy+50, 0.0);
        glVertex3f(cx+50,  +cy-50, 0.0);
        glEnd();

        glBegin(GL_LINES);
        glVertex3f(cx-50,  +cy-50, 0.0);
        glVertex3f(cx+50,  +cy-50, 0.0);
        glEnd();

        glBegin(GL_LINES);
        glVertex3f(cx,           +cy, 0.0);
        glVertex3f(cx-f[0], +cy+f[2], 0.0);
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
    
    if (controller.controlling != CONTROLLING_NONE )
    {
        ctrling = controller.controlling;
        entities[ctrling]->getViewPort(up,pos,forward);
        
        Vec3f up2,pos2;
        //Camera.getViewPort(up2,pos2,forward);

        Camera.fw = entities[ctrling]->getForward();

        if (entities[ctrling]->getType() == MANTA)
            Camera.yAngle = ((Manta*)entities[ctrling])->alpha*100;
    } else
    {
        Camera.getViewPort(up,pos,forward);

        Camera.fw = forward;

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
    
    // Draw islands.
    for (int i=0; i<islands.size(); i++) {
        (islands[i]->draw());
    }

    // Draw vehicles and objects
    synchronized(entities.m_mutex)
    {
        for(size_t i=entities.first();entities.exists(i);i=entities.next(i))
        {
            (entities[i]->setTexture(_textureMetal));
            (entities[i]->drawModel());
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
    //glClearColor(0.7f, 0.9f, 1.0f, 1.0f);
    
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
        
        if (controller.controlling != CONTROLLING_NONE)
        {
            entities[controller.controlling]->doControl(controller);
        }

        // This should be in the command center.
        for (int i = 0; i < islands.size(); i++)
        {
            BoxIsland *island = islands[i];

            if (island->getStructures().size()<5)
            {
                CommandCenter *c = (CommandCenter*)island->getCommandCenter();
                if (c)
                {
                    if (c->getTtl()<=0)
                    {
                        // Add a structure (Structures should be randomly rotated also).
                        int which = (rand() % 30 + 1);
                        Structure *s;


                        if (1<=which && which<=5)
                            s = new LaserTurret();
                        else if (6<=which && which<=7)
                            s = new Structure();
                        else if (8<=which && which<=10)
                            s = new Runway();
                        else if (11<=which && which<=13)
                            s = new Warehouse();
                        else
                            s = new Turret();

                        int x = (rand() % 2000 + 1); x -= 1000;
                        int z = (rand() % 2000 + 1); z -= 1000;

                        island->addStructure(s,x,z,space,world);

                        entities.push_back(s);

                        if (8<=which && which<=10)
                        {
                            Structure *s2 = new Hangar();
                            x-=550;
                            island->addStructure(s2,x,z,space,world);

                            entities.push_back(s2);
                        }

                        c->restart();
                    }

                }
            }

        }


        //printf("Elements alive now: %d\n", vehicles.size());
        // As the sync problem only arises when you delete something, there's no problem here.
        for(size_t i=entities.first();entities.exists(i);i=entities.next(i)) {

            // Autocontrol (AI)
            if (entities[i]->isAuto())
            {
                entities[i]->doControl();
            }

            entities[i]->doDynamics();
            entities[i]->tick();
        }


        synchronized(entities.m_mutex)
        {
            for(size_t i=entities.first();entities.exists(i);i=entities.next(i))
            {
                //printf("Type and ttl: %d, %d\n", vehicles[i]->getType(),vehicles[i]->getTtl());
                if (entities[i]->getType()==5 && entities[i]->getTtl()==0)
                {
                    //printf("Eliminating....\n");
                    dBodyDisable(entities[i]->getBodyID());
                    entities.erase(i);
                    //delete vehicles[i];
                    //dBodyDestroy(vehicles[i]->getBodyID());
                } else if (entities[i]->getHealth()<=0)
                {
                    if (controller.controlling == i)
                        controller.controlling = CONTROLLING_NONE;


                    if (entities[i]->getType() == CARRIER)
                    {
                        char str[256];
                        sprintf(str, "Balaenidae Carrier has been destroyed !");
                        messages.insert(messages.begin(), str);
                    }


                    if (entities[i]->getBodyID()) dBodyDisable(entities[i]->getBodyID());
                    if (entities[i]->getGeom()) dGeomDisable(entities[i]->getGeom());
                    entities.erase(i);

                    explosion();
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
    // @NOTE: update time should be adapted to real FPS (lower is faster).
    glutTimerFunc(1, worldStep, 0);
}




int main(int argc, char** argv) {
	glutInit(&argc, argv);

    //srand (time(NULL));

    srand(0);

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
    if (argc>1 && strcmp(argv[1],"-test")==0)
        initWorldModelling(atoi(argv[2]));
    else
        initWorldModelling(-1);

    const char *conf = dGetConfiguration ();

    printf("ODE Configuration: %s\n", conf);

    // Draw vehicles and objects
    printf("Size %d\n", entities.size());
    synchronized(entities.m_mutex)
    {
        for(size_t i=entities.first();entities.exists(i);i=entities.next(i))
        {
            printf("Body ID (%p) Index (%d) %d - %d\n", (void*)entities[i]->getBodyID(), i, entities[i]->getType(), entities[i]->getTtl());
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
    glutTimerFunc(1, worldStep, 0);
    
	// main loop, hang here.
	glutMainLoop();
	return 0;
}

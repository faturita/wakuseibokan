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

#ifdef __APPLE__
#include <GLUT/glut.h>
#elif __linux
#include <GL/glut.h>
#endif

#include <ode/ode.h>

#include <vector>
#include <mutex>

#include "profiling.h"
#include "commandline.h"

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

#include "engine.h"

#include "structures/Structure.h"
#include "structures/Warehouse.h"
#include "structures/Hangar.h"
#include "structures/Runway.h"
#include "structures/Laserturret.h"
#include "structures/CommandCenter.h"
#include "structures/Turret.h"

#include "map.h"

#include "ai.h"

extern  Controller controller;
extern  Camera Camera;

float horizon = 100000.0f;   // 100 kmf


/* dynamics and collision objects */

extern dWorldID world;
extern dSpaceID space;
extern dBodyID body[NUM];
extern dJointID joint[NUM-1];
extern dJointGroupID contactgroup;
extern dGeomID sphere[NUM];

extern container<Vehicle*> entities;

extern std::vector<BoxIsland*> islands;

std::ofstream msgboardfile;
extern std::vector<Message> messages;

extern int aiplayer;

extern int gamemode;

// @FIXME Change
extern GLuint _textureBox;
extern GLuint _textureMetal;

float fps=0;
extern unsigned long timer;
clock_t elapsedtime;

bool wincondition=false;

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

    int aimc=50,crossc=0;
    
    char str[256];

    
    if (isnan(Camera.pos[0])) exit(1);
    
    fps = getFPS();
    
    sprintf (str, "fps %4.2f  Cam: (%10.2f,%10.2f,%10.2f)  TIME:%lu\n", fps, Camera.pos[0],Camera.pos[1],Camera.pos[2], timer);
	// width, height, 0 0 upper left
    drawString(0,-30,1,str,0.2f);
    
	//glPrint(1,10,10,"HUD");
    
	//glRectf(400.0f,400.0f,450.0f,400.0f);

    bool lostsignal = false;
    
    float speed=0, health=0, power = 0;
    
    if (controller.controllingid != CONTROLLING_NONE)
    {
        speed = entities[controller.controllingid]->getSpeed();
        health = entities[controller.controllingid]->getHealth();
        power = entities[controller.controllingid]->getPower();

        if (entities[controller.controllingid]->getType() == CEPHALOPOD)
        {
            aimc = 10;
            crossc = 195;
        }
        else if (entities[controller.controllingid]->getType() == MANTA)
        {
            aimc = 240;
            crossc = 195;
        }

        if (entities[controller.controllingid]->getSignal()<3)
        {
            lostsignal = true;
        }


    }
    sprintf (str, "Speed:%10.2f - X,Y,Z,P (%5.2f,%5.2f,%5.2f,%5.2f)\n", speed, controller.registers.roll,controller.registers.pitch,controller.registers.yaw,controller.registers.precesion);
	drawString(0,-60,1,str,0.2f);
    
    sprintf (str, "Vehicle:%d  - Thrust:%5.2f - Health: %5.2f - Power:  %5.2f\n", controller.controllingid,controller.registers.thrust, health, power);
	drawString(0,-90,1,str,0.2f);

    if (controller.isTeletype())
    {
        sprintf(str, ">>>%s",controller.str.c_str());
        drawString(0,-180,1,str,0.2f);
    }

    if (wincondition)
    {
        sprintf(str, "You have won!");
        if (getRandomInteger(1,2)==1)
            drawString(75,-270,1,str,1.0f,1.0f,1.0f,0.0f);
        else
            drawString(75,-270,1,str,1.0f,1.0f,0.0f,1.0f);
    }

    // Message Board (@TODO: At least, put this in a different function)

    // This is the amount of ticks that are used to refresh the message board.
    static int mbrefresher = 1000;
    // Message board
    if (messages.size()>0)
    {
        int msgonboard=0;
        for(int i=0;i<messages.size();i++)
        {
            if (messages[i].faction == controller.faction || messages[i].faction == BOTH_FACTION || controller.faction == BOTH_FACTION)
            {
                std::string line = messages[i].msg;
                if (msgonboard==0)
                    drawString(0,-700-msgonboard*25,1,(char*)line.c_str(),0.2f,1.0f,1.0f,0.0f);
                else
                    drawString(0,-700-msgonboard*25,1,(char*)line.c_str(),0.2f);
                msgonboard++;
            }
        }

        if (mbrefresher--<=0)
        {
            msgboardfile << timer << ":" << messages.back().msg <<  std::endl ;
            msgboardfile.flush();
            messages.pop_back();
            mbrefresher = 1000;
        }

    }

    Vec3f f = (Camera.getForward().normalize())*30;

    f = (Camera.fw.normalize())*30;


    sprintf (str, "%5.2f", Camera.getBearing());
    drawString(1150-40,-130,1,str,0.1f,0.0f,1.0f,1.0f);

    // Add typical noisy signal when the aircraft has lost their signal.
    if (lostsignal)
    {
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix(); {
            glTranslatef(0, -400, 1);

            glLineWidth(10.5);

            for(int i=0;i<1200;i++)
                for (int j=240-500;j<240+200;j++)
                {
                    float prob = ((int)(rand() % 100 + 1))/100.0f;
                    if (prob<0.01)
                    {
                        glBegin(GL_LINES);
                        glVertex3f(i,     j, 0);
                        glVertex3f(i+1,   j+1, 0);
                        glEnd();
                    }
                }

        } glPopMatrix();
    }

    
    // Displays the target mark at the center. The position of the center cross depends on camera angles.
    glMatrixMode(GL_MODELVIEW);
	glPushMatrix(); {
		glTranslatef(0, -400, 1);
        
        glLineWidth(2.5);

        int uc=550;                     // Horizontal center, screen is 1100 pixels.
        int lc=0+aimc;                   // Center is at 50.
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


        // Center cross
        int cc = crossc;

        glBegin(GL_LINES);
        glVertex3f(uc+50+Camera.xAngle-10, cc + Camera.yAngle, 0.0);
        glVertex3f(uc+50+Camera.xAngle-2, cc + Camera.yAngle, 0);
        glEnd();

        glBegin(GL_LINES);
        glVertex3f(uc+50+Camera.xAngle+2, cc + Camera.yAngle, 0.0);
        glVertex3f(uc+50+Camera.xAngle+10, cc + Camera.yAngle, 0);
        glEnd();

        glBegin(GL_LINES);
        glVertex3f(uc+50+Camera.xAngle, cc + Camera.yAngle-10, 0.0);
        glVertex3f(uc+50+Camera.xAngle, cc + Camera.yAngle-2, 0);
        glEnd();        

        glBegin(GL_LINES);
        glVertex3f(uc+50+Camera.xAngle, cc + Camera.yAngle+10, 0.0);
        glVertex3f(uc+50+Camera.xAngle, cc + Camera.yAngle+2, 0);
        glEnd();

        // Bearing arrow

        int cx=1150, cy=350;

        // Borders
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

        // Arrow.
        glBegin(GL_LINES);
        glVertex3f(cx,           +cy, 0.0);
        glVertex3f(cx-f[0], +cy+f[2], 0.0);
        glEnd();

        // Nearby units. @NOTE DO IT

        
    } glPopMatrix();
    
    
	glEnable(GL_DEPTH_TEST);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
}



void drawScene() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
    //glMatrixMode(GL_PROJECTION);
    //glLoadIdentity();
    //gluPerspective(45.0, (float)1440 / (float)900, 1.0, Camera.pos[2]+ horizon /**+ yyy**/);


	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
    
    //drawLightning();

    Vec3f up,pos,forward;
    
    int ctrling = 0;
    
    // @FIXME Safe code.  Destroy something and see what happens here.
    if (!entities.isValid(controller.controllingid))
        controller.controllingid = CONTROLLING_NONE;

    if (controller.controllingid != CONTROLLING_NONE )
    {
        ctrling = controller.controllingid;
        entities[ctrling]->getViewPort(up,pos,forward);
        
        Vec3f up2,pos2;
        //Camera.getViewPort(up2,pos2,forward);

        Camera.fw = entities[ctrling]->getForward();

        if (entities[ctrling]->getType() == MANTA)
        {
            Camera.yAngle = ((Manta*)entities[ctrling])->alpha*100;
            Camera.xAngle = ((Manta*)entities[ctrling])->beta*100;
        }
    } else
    {
        Camera.getViewPort(up,pos,forward);

        Camera.fw = forward;

        if (Camera.dx!=0) {
            //pos[0]+=(forward[0]);
            //pos[1]+=(forward[1]);
            //pos[2]+=(forward[2]);
            
            pos[2]+=controller.registers.pitch;
            pos[1]+=controller.registers.precesion;
            pos[0]+=controller.registers.roll;
        }
    }
    
    // If you comment the drawsky it will go dark (night).
    //drawSky(pos[0],pos[1],pos[2]);
    
    Camera.lookAtFrom(up, pos, forward);
    
    // Sets the camera and that changes the floor position.
    Camera.setPos(pos);
    
    // Draw CENTER OF coordinates RGB=(x,y,b)
    glPushAttrib(GL_CURRENT_BIT);
    drawArrow(3);
    glPopAttrib();
    
    // Floor is changing color.
    glPushAttrib(GL_CURRENT_BIT);
    drawFloor(Camera.pos[0],Camera.pos[1],Camera.pos[2]);
    glPopAttrib();

    // Draw islands.
    for (int i=0; i<islands.size(); i++) {
        (islands[i]->draw());
    }

    //draw3DSModel("units/walrus.3ds",1200.0+100,15.0,700.0+300.0,3,_textureBox);

    // Draw vehicles and objects
    // FPS: OpenGL is dead if I draw all the entities.  So I am just drawing objects 10k away.
    // This is a very easy enhancement with tremendous consequences in fps stability.
    synchronized(entities.m_mutex)
    {
        for(size_t i=entities.first();entities.hasMore(i);i=entities.next(i))
        {
            if ((entities[i]->getPos() - Camera.getPos()).magnitude()<10000)
            {
                (entities[i]->setTexture(_textureMetal));
                (entities[i]->drawModel());
            }
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
    drawLightning();
    
    // Initialize scene textures.
    initTextures();
    
}

void handleResize(int w, int h) {
    CLog::Write(CLog::Debug,"Handling Resize: %d, %d \n", w, h);
	glViewport(0, 0, w, h);
    
    // ADDED
    glOrtho( 0, w, 0, h, -1, 1);
    
    
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
    gluPerspective(45.0, (float)w / (float)h, 1.0, Camera.pos[2]+ horizon /**+ yyy**/);
}

static bool didODEInit=false;

void update(int value)
{
	// Derive the control to the correct object

    if (!didODEInit)
    {
        dAllocateODEDataForThread(dAllocateMaskAll);
        didODEInit=true;
    }
    
    if (controller.isInterrupted())
    {
        endWorldModelling();
        // Do extra wrap up
        msgboardfile.close();
        exit(0);
    }
    if (!controller.pause)
	{
        static Player pg(GREEN_FACTION);
        static Player pb(BLUE_FACTION);

        if (aiplayer == BLUE_AI || aiplayer == BOTH_AI)
            pb.playFaction(timer);
        if (aiplayer == GREEN_AI || aiplayer == BOTH_AI)
            pg.playFaction(timer);


        // Auto Control: The controller can be controlled by the user or by the AI
        // Each object is responsible for generating their own controlregisters as if it were a user playing
        // Hence this code gets the controlregisters if AUTO is enabled.  And then it uses the controlregister
        // to control each object as if it were exactly the user (with doControl() in the loop ahead).
        if (controller.controllingid != CONTROLLING_NONE && entities.isValid(controller.controllingid))
        {
            if (!entities[controller.controllingid]->isAuto())
            {
                entities[controller.controllingid]->doControl(controller);
            }
            else
            {
                controller.registers = entities[controller.controllingid]->getControlRegisters();
            }
        }

        // Build island structures, international water structures and repair carriers.
        buildAndRepair(space,world);

        defendIsland(timer,space,world);

        commLink(GREEN_FACTION, space,world);
        commLink(BLUE_FACTION, space, world);



        //CLog::Write(CLog::Debug,"Elements alive now: %d\n", vehicles.size());
        // As the sync problem only arises when you delete something, there's no problem here.
        for(size_t i=entities.first();entities.hasMore(i);i=entities.next(i)) {

            // Autocontrol (AI)
            if (entities[i]->isAuto())
            {
                entities[i]->doControl();
            }
            if ((entities[i]->getSpeed()>10000.0f || isnan(entities[i]->getSpeed())) && entities[i]->getType()!= ACTION)
                entities[i]->stop();
            entities[i]->doDynamics();
            entities[i]->tick();
        }


        synchronized(entities.m_mutex)
        {
            for(size_t i=entities.first();entities.hasMore(i);i=entities.next(i))
            {
                //CLog::Write(CLog::Debug,"Type and ttl: %d %p Valid %d\n",entities[i]->getType(), entities[i],entities.isValid(i));
                if ((entities[i]->getType()==ACTION || entities[i]->getType()==RAY || entities[i]->getType() == CONTROLABLEACTION) && entities[i]->getTtl()<=0)
                {
                    if (controller.controllingid == i)
                        controller.controllingid = CONTROLLING_NONE;

                   if (entities[i]->getBodyID()) dBodyDisable(entities[i]->getBodyID());
                    if (entities[i]->getGeom()) dGeomDisable(entities[i]->getGeom());
                    entities.erase(entities[i]->getGeom());
                    //delete vehicles[i];
                    //dBodyDestroy(vehicles[i]->getBodyID());
                } else if (entities[i]->getHealth()<=0)
                {
                    if (controller.controllingid == i)
                        controller.controllingid = CONTROLLING_NONE;


                    if (entities[i]->getType() == CARRIER)
                    {
                        char str[256];
                        Message m;
                        m.faction = BOTH_FACTION;
                        sprintf(str, "%s Carrier has been destroyed !", FACTION(entities[i]->getFaction()));
                        m.msg = std::string(str);
                        messages.insert(messages.begin(), m);

                        // Check winning condition (if the destroyed carrier is not yours).
                        if (controller.faction != entities[i]->getFaction())
                        {
                            wincondition = true;
                        }
                    }

                    if (entities[i]->getType() == CONTROL)
                    {
                        CommandCenter *c = (CommandCenter*)entities[i];
                        char str[256];
                        Message m;
                        m.faction = BOTH_FACTION;
                        sprintf(str, "Island %s is now a free island.", c->island->getName().c_str());
                        m.msg = std::string(str);
                        messages.insert(messages.begin(), m);
                    }

                    if (entities[i]->getType() == VehicleTypes::MANTA)
                    {
                        Manta *m = (Manta*)entities[i];
                        char str[256];
                        Message mg;
                        mg.faction = BOTH_FACTION;
                        sprintf(str, "Manta %2d has been destroyed.", NUMBERING(m->getNumber()));
                        mg.msg = std::string(str);
                        messages.insert(messages.begin(), mg);
                    }

                    if (entities[i]->getType() == VehicleTypes::WALRUS)
                    {
                        Walrus *m = (Walrus*)entities[i];
                        char str[256];
                        Message mg;
                        mg.faction = BOTH_FACTION;
                        sprintf(str, "Walrus %2d has been destroyed.", NUMBERING(m->getNumber()));
                        mg.msg = std::string(str);
                        messages.insert(messages.begin(), mg);
                    }


                    // Disable bodies and geoms.  The update will take care of the object later to delete it.
                    if (entities[i]->getBodyID()) dBodyDisable(entities[i]->getBodyID());
                    if (entities[i]->getGeom()) dGeomDisable(entities[i]->getGeom());

                    entities.erase(entities[i]->getGeom());

                    explosion();
                }
            }
        }

        
		// Ok, done with the dynamics
        clock_t inicio = clock();
        dSpaceCollide (space,0,&nearCallback);
        dWorldStep (world,0.05);
        elapsedtime = (clock() - inicio); /// CLOCKS_PER_SEC;
        

        //dWorldQuickStep(world,0.05);

		/* remove all contact joints */
		dJointGroupEmpty (contactgroup);
	}
    
	glutPostRedisplay();
    // @NOTE: update time should be adapted to real FPS (lower is faster).
    glutTimerFunc(25, worldStep, 0);
}




int main(int argc, char** argv) {
	glutInit(&argc, argv);

#ifdef DEBUG
    CLog::SetLevel(CLog::All);
#else
    CLog::SetLevel(CLog::None);
#endif

    if (isPresentCommandLineParameter(argc,argv,"-random"))
        srand (time(NULL));
    else
        srand (0);

    // Switch up OpenGL version (at the time of writing compatible with 2.1)
    if (true)
        glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    //else
        //glutInitDisplayMode (GLUT_3_2_CORE_PROFILE | GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    
    disclaimer();
	glutCreateWindow("Wakuseibokan");
    
    if (isPresentCommandLineParameter(argc,argv,"-d"))
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

    if (isPresentCommandLineParameter(argc,argv,"-free"))
        aiplayer = FREE_AI;
    else if (isPresentCommandLineParameter(argc,argv,"-aiplayerblue"))
        aiplayer = BLUE_AI;
    else if (isPresentCommandLineParameter(argc,argv,"-aiplayergreen"))
        aiplayer = GREEN_AI;
    else if (isPresentCommandLineParameter(argc,argv,"-aiplayerboth"))
        aiplayer = BOTH_AI;


    if (isPresentCommandLineParameter(argc,argv,"-bluemode"))
        controller.faction = BLUE_FACTION;
    else if (isPresentCommandLineParameter(argc,argv,"-greenmode"))
        controller.faction = GREEN_FACTION;
    else if (isPresentCommandLineParameter(argc,argv,"-godmode"))
        controller.faction = BOTH_FACTION;
    else
        controller.faction = GREEN_FACTION;


    if (isPresentCommandLineParameter(argc,argv,"-strategy"))
        gamemode = STRATEGYGAME;
    else if (isPresentCommandLineParameter(argc,argv,"-action"))
        gamemode = ACTIONGAME;

    setupWorldModelling();

    // Initialize ODE, create islands, structures and populate the world.
    if (isPresentCommandLineParameter(argc,argv,"-test"))
        initWorldModelling(atoi(getCommandLineParameter(argc,argv,"-test")));
    else if (isPresentCommandLineParameter(argc,argv,"-load"))
        loadgame();
    else
    {
        initWorldModelling();
    }


    const char *conf = dGetConfiguration ();

    CLog::Write(CLog::Debug,"ODE Configuration: %s\n", conf);

    // Draw vehicles and objects
    CLog::Write(CLog::Debug,"Size %d\n", entities.size());
    synchronized(entities.m_mutex)
    {
        for(size_t i=entities.first();entities.hasMore(i);i=entities.next(i))
        {
            CLog::Write(CLog::Debug,"Body ID (%p) Index (%d) %d - %d\n", (void*)entities[i]->getBodyID(), i, entities[i]->getType(), entities[i]->getTtl());
        }

    }

    //unsigned long *a = (unsigned long*)dBodyGetData(vehicles[2]->getBodyID());

    //CLog::Write(CLog::Debug,"Manta is located in %lu\n",*a);
    
    //Initialize all the models and structures.
    initRendering();

    //intro();

    msgboardfile.open ("messageboard.dat");
    
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
    glutTimerFunc(25, worldStep, 0);
    
	// main loop, hang here.
	glutMainLoop();
	return 0;
}

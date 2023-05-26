//
//  carrier.cpp
//  Wakuseibokan
//
//  Created by Rodrigo Ramele on 22/05/14.
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
#include <algorithm>
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

#include "sounds/sounds.h"

#include "keplerivworld.h"

#include "units/BoxVehicle.h"
#include "units/Manta.h"

#include "openglutils.h"

#include "imageloader.h"
#include "terrain/Terrain.h"

#include "engine.h"
#include "entities.h"

#include "structures/Structure.h"
#include "structures/Warehouse.h"
#include "structures/Hangar.h"
#include "structures/Runway.h"
#include "structures/Laserturret.h"
#include "structures/CommandCenter.h"
#include "structures/Turret.h"

#include "weapons/CarrierTurret.h"
#include "weapons/CarrierArtillery.h"
#include "weapons/CarrierLauncher.h"

#include "units/Stingray.h"
#include "units/Medusa.h"
#include "units/Otter.h"

#include "actions/Explosion.h"

#include "map.h"
#include "board.h"
#include "hud.h"

#include "ai.h"

#include "networking/telemetry.h"
#include "networking/ledger.h"

extern  Controller controller;
extern  Camera Camera;

float horizon = 100000.0f;   // 100 kmf

int screen_width;
int screen_height;


/* dynamics and collision objects */

extern dWorldID world;
extern dSpaceID space;
extern dJointGroupID contactgroup;

extern container<Vehicle*> entities;
extern std::vector<BoxIsland*> islands;
std::ofstream msgboardfile;
extern std::vector<Message> messages;

extern int aiplayer;

extern int gamemode;

extern int tracemode;

extern std::unordered_map<std::string, GLuint> textures;

float fps=0;
extern unsigned long timer;
clock_t elapsedtime;

bool wincondition=false;

bool mute=false;
bool cull=false;
bool wireframes=false;

FILE *ledger;

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
    glOrtho(0, 1200, 800, 0, -1, 1);            // @NOTE: This is the size of the HUD screen, it goes from x=[0,1200] , y=[-400,400]
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

    glPushAttrib(GL_CURRENT_BIT);
    glColor4f(1.0f, 1.0f, 1.0f, 1);
	glDisable(GL_DEPTH_TEST);
	glRotatef(180.0f,0,0,1);
	glRotatef(180.0f,0,1,0);

    int aimc=0,crossc=0;
    
    char str[256];

    assert( !isnan(Camera.pos[0]) || !"The height value of the Camera position is not a number.  There are numerical error somewhere.");
    
    fps = getFPS();
    
    sprintf (str, "fps %4.2f  Cam: (%10.2f,%10.2f,%10.2f) Sols: %lu TIME:%lu\n", fps, Camera.pos[0],Camera.pos[1],Camera.pos[2],timer / CYCLES_IN_SOL, timer);
	// width, height, 0 0 upper left
    drawString(0,-30,1,str,0.2f);
    
	//glPrint(1,10,10,"HUD");
    
	//glRectf(400.0f,400.0f,450.0f,400.0f);

    bool lostsignal = false;
    
    float speed=0, health=0, power = 0;

    std::string name;
    
    if (controller.controllingid != CONTROLLING_NONE)
    {
        speed = entities[controller.controllingid]->getSpeed();
        health = entities[controller.controllingid]->getHealth();
        power = entities[controller.controllingid]->getPower();
        name = entities[controller.controllingid]->getName();

        if (entities[controller.controllingid]->getType() == CEPHALOPOD)
        {
            aimc = 10;
            crossc = 195;
        }
        else if (entities[controller.controllingid]->getType() == MANTA)
        {
            aimc = 190;
            crossc = 195;
        }

        if (entities[controller.controllingid]->getSignal()<3)
        {
            lostsignal = true;
        }


    }
    sprintf (str, "Speed:%10.2f - X,Y,Z,P (%5.2f,%5.2f,%5.2f,%5.2f)\n", speed, controller.registers.roll,controller.registers.pitch,controller.registers.yaw,controller.registers.precesion);
	drawString(0,-60,1,str,0.2f);
    
    sprintf (str, "Vehicle:%d %s - Thrust:%5.2f - Health: %5.2f - Power:  %5.2f\n", controller.controllingid,name.c_str(), controller.registers.thrust, health, power);
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

        int uc=600;                     // Horizontal center, screen is 1100 pixels.
        int lc=0+aimc;                   // Center is at 50.
        int w=40;
        int h=40;

        drawOverlyMark(uc,lc,w,h);

        // Center cross
        int cc = crossc;

        /**
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
        glEnd();**/

        drawCross(uc + Camera.xAngle,lc + Camera.yAngle);

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

        /**
        Vec3f enemy(5000.0, 0.0f, 5000.0f);

        Vec3f l = Vec3f(30*(enemy[0]/10000.0f),0.0,30*(enemy[2]/10000.0f) );

        glLineWidth(4.5);
        glBegin(GL_LINES);
        glVertex3f(cx - l[0]+1, +cy+l[2]-1,  0.0);
        glVertex3f(cx - l[0]-1, +cy+l[2]+1,  0.0);
        glEnd();
        **/

        // Nearby units. @NOTE DO IT

        if (controller.controllingid != CONTROLLING_NONE)
        {
            Vehicle *me = entities[controller.controllingid];
            Vec3f meLoc = me->getPos();
            int friendlyfaction = me->getFaction();

            float closest = 10000.0f;
            float proj = 40.0f;

            for(size_t i=entities.first();entities.hasMore(i);i=entities.next(i))
            {
                Vehicle *v=entities[i];

                if (v && (v->getType() == CONTROLABLEACTION )
                        && v->getFaction()!=friendlyfaction)
                {
                    Vec3f enemy = v->getPos() - meLoc;enemy[1]=0;
                    if ((enemy).magnitude()<closest) {


                        Vec3f l = Vec3f(proj*(enemy[0]/closest),0.0,proj*(enemy[2]/closest) );

                        static int coun=0;
                        if (int((enemy).magnitude()) <= coun++ )
                        {
                            radarbeep();
                            coun=0;
                        }

                        glPointSize(4.5);
                        glColor3f(1.0,0.0,0.0);
                        glBegin(GL_LINES);
                        glVertex3f(cx - l[0]+1, +cy+l[2]-1,  0.0);
                        glVertex3f(cx - l[0]-1, +cy+l[2]+1,  0.0);
                        glEnd();
                    }
                }
                if (v &&
                        ( (v->getType() != WEAPON && v->getType() != ACTION && v->getType() != EXPLOTABLEACTION && v->getType() != CONTROLABLEACTION && v->getType() != RAY))
                        && v->getFaction()!=friendlyfaction)   // Fix this.
                {
                    Vec3f enemy = v->getPos() - meLoc;
                    if ((enemy).magnitude()<closest) {

                        Vec3f enemy = entities[i]->getPos() - meLoc;

                        Vec3f l = Vec3f(proj*(enemy[0]/closest),0.0,proj*(enemy[2]/closest) );

                        glPointSize(4.5);
                        glColor3f(1.0,0.0,1.0);
                        glBegin(GL_LINES);
                        glVertex3f(cx - l[0]+1, +cy+l[2]-1,  0.0);
                        glVertex3f(cx - l[0]-1, +cy+l[2]+1,  0.0);
                        glEnd();


                        Vec3f sc = v->screenLocation();

                        //dout << sc[0] << "," << sc[2] << " at " << sc[1] << std::endl;


                        if ((sc[1]<1 && sc[0]>0 && sc[0]<screen_width) &&
                            (sc[1]<1 && sc[2]>0 && sc[2]<screen_height) )
                        {

                            float x = 1200.0/screen_width * sc[0];
                            float y = 800.0/screen_height * sc[2] - 400.0;
                            //drawOverlyMark(x,y, 10,10);
                            drawCross(x,y);
                        }
                    }
                }
            }
        }

        
    } glPopMatrix();
    
    glPopAttrib();
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
    
    drawLightning();

    Vec3f up,pos,forward;
    
    int ctrling = 0;
    
    // @NOTE Safe code.  Destroy something and see what happens here.
    if (!entities.isValid(controller.controllingid))
    {
        controller.controllingid = CONTROLLING_NONE;
        controller.reset();
    }

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
    
    glPushAttrib(GL_CURRENT_BIT);
    drawSky(Camera.fw[0],Camera.fw[1],Camera.fw[2]);
    glPopAttrib();
    
    Camera.lookAtFrom(up, pos, forward);
    
    // Sets the camera and that changes the floor position.
    Camera.setPos(pos);
    
    // Draw CENTER OF coordinates RGB=(x,y,b)
    glPushAttrib(GL_CURRENT_BIT);
    drawArrow(3);
    glPopAttrib();
    
    glPushAttrib(GL_CURRENT_BIT);
    drawFloor(Camera.pos[0],Camera.pos[1],Camera.pos[2]);
    glPopAttrib();


    // Draw islands.  All of them are drawn.  This has a very effect of seeing the islands from the distance.
    for (int i=0; i<islands.size(); i++) {
        (islands[i]->draw());
    }

    //draw3DSModel("units/walrus.3ds",1200.0+100,15.0,700.0+300.0,3,_textureBox);

    // Movable units, and the camera, carry the living energy.
    // Get a list of points in 3D space where there are movable units and the camera.
    // Put all of them in a list.
    // And activate all the structures that are inside a 10k radius of that.
    // Deactivate all the others.
    // If a tree is in the middle of the forest and nobody is around it to see it, then, does the tree actually fall ?

    // @NOTE: I am only considering two carriers here.
    Vec3f carrierloc1(0,0,0);
    Vec3f carrierloc2(0,0,0);
    Vec3f cameraloc = Camera.getPos();


    // Draw vehicles and objects
    // FPS: OpenGL is dead if I draw all the entities.  So I am just drawing objects 10k away.
    // This is a very easy enhancement with tremendous consequences in fps stability.
    synchronized(entities.m_mutex)
    {
        for(size_t i=entities.first();entities.hasMore(i);i=entities.next(i))
        {
            if (entities[i]->getType() == CARRIER && entities[i]->getFaction() == BLUE_FACTION)
                carrierloc1 = entities[i]->getPos();
            if (entities[i]->getType() == CARRIER && entities[i]->getFaction() == GREEN_FACTION)
                carrierloc2 = entities[i]->getPos();
            if ((entities[i]->getPos() - cameraloc).magnitude()<10000)
            {
                //(entities[i]->setTexture(textures["metal"]));
                glPushAttrib(GL_CURRENT_BIT);
                (entities[i]->drawModel());
                glPopAttrib();

                entities[i]->updateScreenLocation();

                if ((entities[i]->getBodyID()))
                    dBodyEnable(entities[i]->getBodyID());
                else
                    dGeomEnable(entities[i]->getGeom());
            }
            else if ( ((entities[i]->getPos() - carrierloc1).magnitude()<10000) ||
                      ((entities[i]->getPos() - carrierloc2).magnitude()<10000) )
            {
                if (!(entities[i]->getBodyID()))
                    dGeomEnable(entities[i]->getGeom());
            }
            else if (entities[i]->getBodyID() == NULL)
            {
                dGeomDisable(entities[i]->getGeom());
            }
        }
    }

    // Daylight frequency.  The "sol" in this world lasts for 10000 cicles.  At 60 fps, 2.7 minutes.
    float daylight_frequency = 1.0/CYCLES_IN_SOL;
    float daylight =   sin(daylight_frequency * 2 * PI * timer)*0.4 + 0.6;   // From 0.2 -- 1.0

    // This is the final color that is used to paint everything on the screen.
    glColor3f(daylight,daylight,daylight);

    if (Camera.pos[1]<0) // Dark under the water (@NOTE: For the future developer: I want submarines !)
        glColor3f(0.1,0.1,0.1);

    // GO with the HUD
    switch (controller.view)
    {
    case 1: drawHUD();break;
    case 2: drawMap();break;
    case 3: drawBoard();break;
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
    if (wireframes) glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
    
    
    glShadeModel(GL_SMOOTH); // Type of shading for the polygons
    
    glEnable(GL_COLOR_MATERIAL);
    
    // Do not show the interior faces.... (FASTER but uglier.  The sky will dissapear).
    if (cull) glEnable(GL_CULL_FACE);
    
	// Blue sky !!!
    //glClearColor(0.7f, 0.9f, 1.0f, 1.0f);
    glClearColor(0.0f, 0.0f, 0.1f, 0.1f);
    //drawLightning();
    
    // Initialize scene textures.
    initTextures();
    
}

void handleResize(int w, int h) {
    CLog::Write(CLog::Debug,"Handling Resize: %d, %d \n", w, h);
	glViewport(0, 0, w, h);

    screen_width = w;
    screen_height = h;
    
    // ADDED
    glOrtho( 0, w, 0, h, -1, 1);
    
    
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
    gluPerspective(45.0, (float)w / (float)h, 1.0, Camera.pos[2]+ horizon /**+ yyy**/);
}

static bool didODEInit=false;


// Go through all the elements that appear each time on the trackrecord.
// Those who are new, create them.
// Those wo are already there, update them.
// Those who are no longer there, destroy them.
void replayupdate(int value)
{
    if (controller.isInterrupted())
    {
        endWorldModelling();
        // Do extra wrap up
        msgboardfile.close();
        if (tracemode == RECORD || tracemode == REPLAY)
        {
            fclose(ledger);
            disconnect();
        }
        exit(0);
    }

    std::vector<size_t> visited;

    if (!controller.pause)
    {
        // I assume the file is open

        // Check the controller structure and send it through a client socket to the server.

        TickRecord record;

        int ret = 1;
        while (ret>0)
        {

            // Read from the remote connection.

            //ret = receive(&record);

            ret = fread(&record, sizeof(TickRecord),1,ledger);

            if (ret>0)
            {
                //printf(" %ld vs %ld \n", record.timerparam, timer);

                visited.push_back(record.id);

                if (!entities.isValid(record.id))
                {
                    // Create a new entity which I didn't see it yet
                    createEntity(record,space,world);
                }

                if ( entities.isValid(record.id) )
                {
                    // Update the data from current entities
                    Vehicle *v = entities[record.id];

                    if (v)
                    {

                        v->deserialize(record);

                    }

                }


                if (record.timerparam != timer)
                    break;
            }

        }

        synchronized(entities.m_mutex)
        {
            // Delete the entries that fulfill the delete condition.
            for(size_t i=entities.first();entities.hasMore(i);i=entities.next(i)) {

                if ( ((entities[i]->getType()==ACTION || entities[i]->getType()==RAY || entities[i]->getType() == CONTROLABLEACTION) &&
                        entities[i]->getTtl()<=0) ||
                    (entities[i]->getHealth()<=0)
                    )
                {
                    if (controller.controllingid == i)
                    {
                        controller.controllingid = CONTROLLING_NONE;
                        controller.reset();
                    }
                    deleteEntity(i);
                }

                //if ( std::find(visited.begin(), visited.end(), i) == visited.end() )
                //{
                //    deleteEntity(i);
                //}
            }
        }


    }
    glutPostRedisplay();
    // @NOTE: update time should be adapted to real FPS (lower is faster).
    glutTimerFunc(20, worldStep, 0);
}


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
        fclose(ledger);
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


        // 1: Read the information from the sockets with the controller information that is being sent from
        //    each client
        // 2: update a Control vector with all the controllers that I have now.  This is fixed or somehow created
        //    by a joining process at the beginning of the game.  Control is a dynamic vector of Controller.
        // 3: Now go through all the objects (including Control[0] which is the person playing on the server)
        //    and execute the doControl(Control[i])


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
            {
                // @NOTE: Extra safeguard (ODE stability issues).
                entities[i]->stop();
            }
            entities[i]->doDynamics();
            entities[i]->tick();

            if (tracemode == RECORD)
            {
                record(timer,i, entities[i]);
                notify(timer, i, entities[i]);
            }
        }


        synchronized(entities.m_mutex)
        {
            for(size_t i=entities.first();entities.hasMore(i);i=entities.next(i))
            {
                //CLog::Write(CLog::Debug,"Type and ttl: %d %p Valid %d\n",entities[i]->getType(), entities[i],entities.isValid(i));
                if ((entities[i]->getType()==ACTION || entities[i]->getType()==RAY || entities[i]->getType() == CONTROLABLEACTION) &&
                        entities[i]->getTtl()<=0)
                {
                    if (controller.controllingid == i)
                    {
                        controller.controllingid = CONTROLLING_NONE;
                        controller.reset();
                    }

                    if (entities[i]->getBodyID()) { dBodyDisable(entities[i]->getBodyID()); }
                    if (entities[i]->getGeom())   { dGeomDisable(entities[i]->getGeom());   }

                    entities.erase(entities[i]->getGeom());

                } else if (entities[i]->getHealth()<=0)
                {
                    if (controller.controllingid == i)
                    {
                        controller.controllingid = CONTROLLING_NONE;
                        controller.reset();
                    }


                    if (entities[i]->getType() == CARRIER)
                    {
                        char str[256];
                        Message m;
                        m.faction = BOTH_FACTION;
                        sprintf(str, "%s Carrier has been destroyed !", FACTION(entities[i]->getFaction()));
                        m.msg = std::string(str);
                        messages.insert(messages.begin(), m);

                        // Check winning condition (if the destroyed carrier is not yours).
                        if (controller.faction != entities[i]->getFaction() && controller.faction != BOTH_FACTION)
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

                    if ((entities[i]->getType() == VehicleTypes::MANTA) || (entities[i]->getType() == VehicleTypes::WALRUS) )
                    {
                        Vehicle *v = entities[i];
                        char str[256];
                        Message mg;
                        mg.faction = BOTH_FACTION;
                        sprintf(str, "%s has been destroyed.", v->getName().c_str());
                        mg.msg = std::string(str);
                        messages.insert(messages.begin(), mg);
                    }

                    if (entities[i]->getType() != VehicleTypes::WEAPON)
                    {
                        explosion();
                        Vec3f loc = entities[i]->getPos();

                        Explosion* b1 = new Explosion();
                        b1->init();
                        b1->setTexture(textures["metal"]);
                        b1->embody(world, space);
                        b1->setPos(loc[0],loc[1],loc[2]);
                        b1->stop();

                        entities.push_back(b1, b1->getGeom());

                        Vec3f dims = entities[i]->getDimensions();

                        b1->expand(dims[0],dims[1],dims[2],2,world, space);
                    }

                    deleteEntity(i);
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
    glutTimerFunc(20, worldStep, 0);

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

    if (isPresentCommandLineParameter(argc,argv,"-mute"))
        mute = true;
    else
        mute = false;

    if (isPresentCommandLineParameter(argc,argv,"-wire"))
        wireframes = true;
    else
        wireframes = false;

    if (isPresentCommandLineParameter(argc,argv,"-cull"))
        cull = true;
    else
        cull = false;

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

    // Connects a socket to a UDP Server.  Keep in mind that the server must exist at this point for this to work.
    inittelemetry();

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

    controller.controllingid = 0;


    if (isPresentCommandLineParameter(argc,argv,"-bluemode"))
        {controller.faction = BLUE_FACTION;controller.controllingid = 1;}
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


    if (isPresentCommandLineParameter(argc,argv,"-record"))
        tracemode = RECORD;
    else if (isPresentCommandLineParameter(argc,argv,"-replay"))
        tracemode = REPLAY;
    else
        tracemode = NOTRACE;


    setupWorldModelling();
    initRendering();

    // Initialize ODE, create islands, structures and populate the world.
    if (isPresentCommandLineParameter(argc,argv,"-testcase"))
        initWorldModelling(0);
    else if (isPresentCommandLineParameter(argc,argv,"-test"))
        initWorldModelling(atoi(getCommandLineParameter(argc,argv,"-test")));
    else if (isPresentCommandLineParameter(argc,argv,"-load"))
        loadgame();
    else if (tracemode == REPLAY)
    {
        initWorldModelling();
        ledger = fopen("ledger.bin","rb");
        join_lobby();
    }
    else
    {
        if (tracemode == RECORD) {ledger = fopen("ledger.bin","wb+");        init_lobby();}
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
    //initRendering();

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

/* ============================================================================
**
** Main Program - Wakuseiboukan - 22/05/2014
**
** Copyright (C) 2014  Rodrigo Ramele
**
** For personal, educationnal, and research purpose only, this software is
** provided under the Gnu GPL (V.3) license. To use this software in
** commercial application, please contact the author.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License V.3 for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
**
** ========================================================================= */

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

#include "savegame.h"

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
#include "networking/lobby.h"

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
extern int peermode;

extern std::unordered_map<std::string, GLuint> textures;

float fps=0;
float latency=0;
extern unsigned long timer;
clock_t elapsedtime;

bool wincondition=false;

bool mute=false;
bool cull=false;
bool wireframes=false;

FILE *ledger;

std::vector<Controller*> controllers;

std::vector<TrackRecord>   track;

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
    
    sprintf (str, "fps %4.2f  Lat: %4.2f Cam: (%8.2f,%8.2f,%8.2f) Sols: %lu TIME:%lu\n", fps, latency, Camera.pos[0],Camera.pos[1],Camera.pos[2],timer / CYCLES_IN_SOL, timer);
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
        for(size_t i=0;i<messages.size();i++)
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

    snprintf(str,7,  "%5.2f",Camera.getBearing());
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

            float closerOnTarget = closest;
            int whichIsCloserOnTarget = -1; float whichX=0, whichY=0;

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
                        && v->getFaction()!=friendlyfaction)   // @FIXME: Fix this.
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

                            // @FIXME: The screen size should not be fixed.
                            float x = 1200.0/screen_width * sc[0];
                            float y = 800.0/screen_height * sc[2] - 400.0;
                            //drawOverlyMark(x,y, 10,10);
                            drawCross(x,y);

                            float distance = (Vec3f(x,0.0,y) - Vec3f(uc + Camera.xAngle,0,lc + Camera.yAngle)).magnitude();


                            if (distance<closerOnTarget)
                            {
                                closerOnTarget = distance;
                                whichIsCloserOnTarget = i;
                                whichX = x;
                                whichY = y;
                            }
                        }
                    }
                }
            }

            if (whichIsCloserOnTarget>=0)
            {
                if (entities[whichIsCloserOnTarget]->getType()==VehicleTypes::MANTA)
                    controller.target_type = TargetType::Air_To_Air;
                else
                    controller.target_type = TargetType::Air_To_Ground;
                controller.targetX = entities[whichIsCloserOnTarget]->getPos()[0];
                controller.targetY = entities[whichIsCloserOnTarget]->getPos()[1];
                controller.targetZ = entities[whichIsCloserOnTarget]->getPos()[2];
                drawOverlyMark(whichX,whichY, 10,10);
            } else {
                controller.targetX = 0;
                controller.targetY = 0;
                controller.targetZ = 0;
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
    if (controller.controllingid != CONTROLLING_NONE && !entities.isValid(controller.controllingid))
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

        Vec3f f = forward.normalize();
        f = f * controller.registers.thrust;

        pos = pos + f;

        if (Camera.dx!=0) {
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

    // Put up there a sun like in minecraft.  @TODO: Configure it a light source and adjust its trajectory
    //   from east to west crossing the zenith.
    float daylight_frequency = 1.0/CYCLES_IN_SOL;

    glPushAttrib(GL_CURRENT_BIT);
    glPushMatrix();
    glTranslatef(pos[0]-cos(daylight_frequency * 2 * PI * timer)*(horizon-100),pos[1]+200.0+sin(daylight_frequency * 2 * PI * timer)*(horizon-100),pos[2]);
    drawTheRectangularBox(textures["venus"], 800.0,800.0,800.0);
    glPopMatrix();
    glPopAttrib();
    
    // Draw CENTER OF coordinates RGB=(x,y,b)
    glPushAttrib(GL_CURRENT_BIT);
    drawArrow(3);
    glPopAttrib();
    
    // Go with the floor (the sea)
    glPushAttrib(GL_CURRENT_BIT);
    drawFloor(Camera.pos[0],Camera.pos[1],Camera.pos[2]);
    glPopAttrib();


    // Draw islands.  All of them are drawn.  This has a very effect of seeing the islands from the distance.
    for (size_t i=0; i<islands.size(); i++) {
        (islands[i]->draw());
    }

    // @NOTE: You can test quickly any 3ds model, by openning it up here and drawing it.
    //draw3DSModel("units/walrus.3ds",1200.0+100,15.0,700.0+300.0,3,_textureBox);

    // Movable units, and the camera, carry the living energy.
    // Get a list of points in 3D space where there are movable units and the camera.
    // Put all of them in a list.
    // And activate all the structures that are inside a 10k radius of that.
    // Deactivate all the others.
    // If a tree falls in the middle of the forest and nobody is around to see it, then, does the tree actually fall ?

    // @NOTE: I am only considering two carriers here.
    Vec3f carrierloc1(0,0,0);
    Vec3f carrierloc2(0,0,0);
    Vec3f cameraloc = Camera.getPos();

    float RENDER_RANGE = 10000.0;


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
            if ((entities[i]->getPos() - cameraloc).magnitude()<RENDER_RANGE)
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
            else if ( ((entities[i]->getPos() - carrierloc1).magnitude()<RENDER_RANGE) ||
                      ((entities[i]->getPos() - carrierloc2).magnitude()<RENDER_RANGE) )
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
    float daylight =   sin(daylight_frequency * 2 * PI * timer)*0.4 + 0.6;   // From 0.2 -- 1.0

    // This is the final color that is used to paint everything on the screen.
    glColor3f(daylight,daylight,daylight);

    if (Camera.pos[1]<0) // Dark under the water (@NOTE: For the future developers: submarines could be nice)
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
        clearSound();
        // Do extra wrap up
        msgboardfile.close();
        if (tracemode == RECORD || tracemode == REPLAY)
        {
            fclose(ledger);
        }
        if (peermode == CLIENT || peermode == SERVER)
        {
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

            if (peermode == CLIENT)
                ret = receive(&record);
            else if (tracemode == REPLAY)
                ret = fread(&record, sizeof(TickRecord),1,ledger);

            // Sync timers
            if (timer == 0)
            {
                timer = record.timerparam;
            }

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


                // @FIXME Check what happen with timer if they are synchronized between server and clients.
                if (record.timerparam != timer)
                    break;
            }

        }

        {

            // @FIXME: Add a CRC to the mesg to see if there are no changes DO NOT SEND IT.
            // @FIXME: Add a mark to send the current timer to the server to measure the latency.
            // @FIXME: Add a mark on the HUD to show current latency.
            ControlStructure mesg;

            mesg.controllingid = controller.controllingid;
            mesg.registers = controller.registers;
            mesg.faction = controller.faction;
            mesg.sourcetimer = timer;

            CommandOrder co = controller.pop();
            mesg.order = co;

            static crc arethereanychange = 0;

            crc val = crcSlow((uint8_t *) &mesg,  sizeof(struct ControlStructure));

            if (val != arethereanychange)
            {
                printf("Command Order: %d\n", mesg.order.command);
                sendCommand(mesg);
                arethereanychange = val;
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

// Go through all the controllers and check if there are pending orders that have not been processed
// and executes them on the entity model.  This runs only on the SERVER.
void inline processCommandOrders()
{
    for (size_t controllerindex = 0; controllerindex < controllers.size(); controllerindex++)
    {
        Controller *ctroler = controllers[controllerindex];
        if (ctroler->controllingid != CONTROLLING_NONE && entities.isValid(ctroler->controllingid))
        {
            CommandOrder co = ctroler->pop();
            if (co.command == Command::StopOrder)
            {
                Vehicle *v = entities[ctroler->controllingid];
                v->stop();
            } else if (co.command == Command::AttackOrder)
            {
                Vec3f pos(co.parameters.x,co.parameters.y, co.parameters.z);

                entities[ctroler->controllingid]->attack(pos);
                entities[ctroler->controllingid]->enableAuto();
            } else if (co.command == Command::DestinationOrder)
            {
                Vec3f target(co.parameters.x,co.parameters.y, co.parameters.z);

                entities[ctroler->controllingid]->goTo(target);
            } else if (co.command == Command::TaxiOrder)
            {
                taxiManta(entities[ctroler->controllingid]);
            } else if (co.command == Command::TelemetryOrder)
            {
                if (co.parameters.bit)
                    entities[ctroler->controllingid]->enableTelemetry();
                else
                    entities[ctroler->controllingid]->disableTelemetry();
            } else if (co.command == Command::LaunchOrder)
            {
                launchManta(entities[ctroler->controllingid]);
            } else if (co.command == Command::LandOrder)
            {
                landManta(entities[ctroler->controllingid]);
            } else if (co.command == Command::CaptureOrder)
            {
                BoxIsland *island = NULL;
                if (entities[ctroler->controllingid]->getSubType()==CEPHALOPOD)
                {
                    Cephalopod *w = (Cephalopod*) entities[ctroler->controllingid];
                    // Check if this invading unit is already on the island
                    island = w->getIsland();
                } else if (entities[ctroler->controllingid]->getType()==WALRUS)
                {

                    Walrus *w = (Walrus*) entities[ctroler->controllingid];
                    // Check if this invading unit is already on the island
                    island = w->getIsland();
                }
                Vehicle *w = entities[ctroler->controllingid];

                if (w && island)
                    captureIsland(w,island,w->getFaction(),co.parameters.typeofisland,space, world);
            } else if (co.command == Command::AutoOrder)
            {
                if (co.parameters.bit)
                    entities[ctroler->controllingid]->enableAuto();
                else
                    entities[ctroler->controllingid]->disableAuto();
            } else if (co.command == Command::DockOrder)
            {
                if (co.parameters.spawnid == VehicleSubTypes::ADVANCEDWALRUS)
                {
                    Vehicle *dock = entities[ctroler->controllingid];

                    // @FIXME: Find the walrus that is actually closer to the dock bay.  This force all the walruses to dock.
                    for(size_t i=entities.first();entities.hasMore(i);i=entities.next(i))
                    {
                        // @FIXME: Only put back the walrus that is close to the carrier.
                        //CLog::Write(CLog::Debug,"Type and ttl: %d, %d\n", vehicles[i]->getType(),vehicles[i]->getTtl());
                        if (entities[i]->getType()==WALRUS && entities[i]->getStatus()==SailingStatus::SAILING &&
                                entities[i]->getFaction()==dock->getFaction() && (dock->getPos()-entities[i]->getPos()).magnitude()<DOCK_RANGE)
                        {
                            char msg[256];
                            Message mg;
                            mg.faction = entities[i]->getFaction();
                            sprintf(msg, "%s is now back on deck.",entities[i]->getName().c_str());
                            mg.msg = std::string(msg);
                            messages.insert(messages.begin(), mg);

                            entities[i]->damage(1000);
                            if (peermode == SERVER)
                            {
                                notify(timer, i, entities[i]);
                            }

                            deleteEntity(i);
                        }
                    }
                } else if (co.parameters.spawnid == VehicleSubTypes::SIMPLEMANTA)
                {
                    for(size_t i=entities.first();entities.hasMore(i);i=entities.next(i))
                    {
                        //CLog::Write(CLog::Debug,"Type and ttl: %d, %d\n", vehicles[i]->getType(),vehicles[i]->getTtl());
                        if (entities[i]->getType()==MANTA && entities[i]->getStatus()==FlyingStatus::ON_DECK)
                        {
                            char str[256];
                            Message mg;
                            mg.faction = entities[i]->getFaction();
                            sprintf(str, "%s is now on bay.",entities[i]->getName().c_str());
                            mg.msg = std::string(str);
                            messages.insert(messages.begin(), mg);
                            //CLog::Write(CLog::Debug,"Eliminating....\n");

                            entities[i]->damage(1000);
                            if (peermode == SERVER)
                            {
                                notify(timer, i, entities[i]);
                            }

                            deleteEntity(i);
                        }
                    }
                }

            } else if (co.command == Command::SpawnOrder)
            {
                if (co.parameters.spawnid == VehicleSubTypes::ADVANCEDWALRUS)
                    spawnWalrus(space,world,entities[ctroler->controllingid]);
                else if (co.parameters.spawnid == VehicleSubTypes::SIMPLEMANTA)
                {
                    if (entities[ctroler->controllingid]->getType()==CARRIER || entities[controller.controllingid]->getType()==LANDINGABLE )
                    {
                        size_t idx = 0;
                        spawnManta(space,world,entities[ctroler->controllingid],idx);
                    }
                }
                else if (co.parameters.spawnid == VehicleSubTypes::CEPHALOPOD)
                {
                    if (entities[ctroler->controllingid]->getType()==CARRIER || entities[ctroler->controllingid]->getType()==LANDINGABLE )
                    {
                        Cephalopod* m = (Cephalopod*)(entities[ctroler->controllingid]->spawn(world,space,CEPHALOPOD,findNextNumber(entities[ctroler->controllingid]->getFaction(),MANTA,CEPHALOPOD)));

                        size_t idx = entities.push_back(m, m->getGeom());
                    }
                }
            } else if (co.command == Command::FireOrder)
            {
                if (ctroler->controllingid != CONTROLLING_NONE && entities.isValid(ctroler->controllingid))
                {
                    // @FIXME: Check if ctroler->weapon or co.parameters.weapon ??
                    Vehicle *action = (entities[ctroler->controllingid])->fire(ctroler->weapon, world,space);
                    //int *idx = new int();
                    //*idx = vehicles.push_back(action);
                    //dBodySetData( action->getBodyID(), (void*)idx);
                    size_t actionid = CONTROLLING_NONE;
                    if (action != NULL)
                    {
                        actionid = entities.push_at_the_back(action, action->getGeom());
                    }

                    // @FIXME: At this point I need to notify the controller that the fire action was executed,
                    //   which can be used to switch to control a missile or to make a sound.
                    if (controllerindex == 0 && action != NULL && action->getType() == VehicleTypes::CONTROLABLEACTION)
                    {
                        Vec3f target(co.parameters.x,co.parameters.y,co.parameters.z);
                        //Vehicle *_b = findNearestEnemyVehicle(entities[ctroler->controllingid]->getFaction(),entities[ctroler->controllingid]->getPos(),10000.0);
                        //Island *island = findNearestEnemyIsland(entities[ctroler->controllingid]->getPos(),false);
                        //_b = findCommandCenter(island);

                        //if (_b)
                        {
                                action->goTo(target);
                                action->enableAuto();

                                // @NOTE: The switch to see the missile only happens if the controlling faction can do it.
                                //switchControl(actionid);
                                //gunshot();
                        }

                    }

                }
            }
        }

    }
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


        // 1: Read the information from the sockets containing the controller information that is being sent from
        //    each client
        // 2: Update a Control vector with all the controllers that I have now.  This is fixed or somehow created
        //    by a joining process at the beginning of the game.  Control is a dynamic vector of Controller.
        // 3: Now go through all the objects (including Control[0] which is the person playing on the server)
        //    and execute the doControl(Control[i])

        ControlStructure mesg;

        int n = receiveCommand(&mesg);

        if (n!=-1)
        {
            // @FIXME: I need to go through all the registered users and get the information from their controllers.
            controllers[1]->controllingid = mesg.controllingid;
            controllers[1]->faction = mesg.faction;
            controllers[1]->registers = mesg.registers;
            controllers[1]->push(mesg.order);

        }

        // Process the orders

        processCommandOrders();


        // Auto Control: The controller can be controlled by the user or by the AI
        // Each object is responsible for generating their own controlregisters as if it were a user playing
        // Hence this code gets the controlregisters if AUTO is enabled.  And then it uses the controlregister
        // to control each object as if it were exactly the user (with doControl() in the loop ahead).
        for (size_t j = 0; j < controllers.size(); j++)
        {
            Controller *ctroler = controllers[j];
            if (ctroler->controllingid != CONTROLLING_NONE && entities.isValid(ctroler->controllingid))
            {
                if (!entities[ctroler->controllingid]->isAuto())
                {
                    entities[ctroler->controllingid]->doControl(*ctroler);
                }
                else
                {
                    ctroler->registers = entities[ctroler->controllingid]->getControlRegisters();
                }
            }
        }

        // Build island structures, international water structures and repair carriers.
        buildAndRepair(space,world);

        trackTargets();

        defendIsland(timer,space,world);

        commLink(GREEN_FACTION, space,world);
        commLink(BLUE_FACTION, space, world);


        //CLog::Write(CLog::Debug,"Elements alive now: %d\n", vehicles.size());
        // As the race condition problem only arises when you delete something, there's no problem here.
        for(size_t i=entities.first();entities.hasMore(i);i=entities.next(i)) {

            if (!entities.isValid(i))
                continue;

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
            }

            if (peermode == SERVER)
            {
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

        // @NOTE: At 00.5 the speed that you get from ODE represents in meters/ticks speed/20.
        // So if you want speed in m/s you need to do (speed * FPS)/20
        // The value that you get is independent of the computer performance and fps.
        clock_t inicio = clock();
        dSpaceCollide (space,0,&nearCallback);
        dWorldStep (world,0.05);        // 0.05
        elapsedtime = (clock() - inicio); /// CLOCKS_PER_SEC;
        

        //dWorldQuickStep(world,0.05);

		/* remove all contact joints */
		dJointGroupEmpty (contactgroup);
	}
    
	glutPostRedisplay();
    // @NOTE: update time should be adapted to real FPS (lower is faster).
    glutTimerFunc(20, worldStep, 0);

}


/*
 * GLUT callbacks:
 */
static void update_fade_factor(void)
{
    int milliseconds = glutGet(GLUT_ELAPSED_TIME);
    sinf((float)milliseconds * 0.001f) * 0.5f + 0.5f;
    glutPostRedisplay();
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

    controller.controllingid = 1;


    if (isPresentCommandLineParameter(argc,argv,"-bluemode"))
        {controller.faction = BLUE_FACTION;controller.controllingid = 2;}
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


    if (isPresentCommandLineParameter(argc,argv,"-client"))
        peermode = CLIENT;
    else if (isPresentCommandLineParameter(argc,argv,"-server"))
        peermode = SERVER;
    else
        peermode = SERVER;


    setupWorldModelling();
    initRendering();
    initSound();

    // Initialize ODE, create islands, structures and populate the world.
    if (isPresentCommandLineParameter(argc,argv,"-testcase"))
        initWorldModelling(0);
    else if (isPresentCommandLineParameter(argc,argv,"-test"))
        initWorldModelling(atoi(getCommandLineParameter(argc,argv,"-test")));
    else if (isPresentCommandLineParameter(argc,argv,"-load"))
        loadgame(std::string(getCommandLineParameter(argc,argv,"-load")));
    else if (tracemode == REPLAY)
    {
        initWorldModelling();
        ledger = fopen("ledger.bin","rb");
    }
    else
    {
        if (tracemode == RECORD) {ledger = fopen("ledger.bin","wb+");}
        initWorldModelling();
    }

    if (peermode == CLIENT)
        join_lobby();
    else if (peermode == SERVER)
        init_lobby();


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


    controllers.push_back(&controller);

    if (peermode==SERVER)
    {
        controllers.push_back(new Controller());
        setupControllerServer();

    }

    if (peermode == CLIENT)
    {
        setupControllerClient();
    }


    //unsigned long *a = (unsigned long*)dBodyGetData(vehicles[2]->getBodyID());

    //CLog::Write(CLog::Debug,"Manta is located in %lu\n",*a);
    
    //Initialize all the models and structures.
    //initRendering();

    intro();

    msgboardfile.open ("messageboard.dat");
    
	// OpenGL callback functions.
	glutDisplayFunc(drawScene);
	glutKeyboardFunc(handleKeypress);
	glutSpecialFunc(handleSpecKeypress);
    //glutIdleFunc(&update_fade_factor);
    
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

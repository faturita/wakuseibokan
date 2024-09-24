#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <stdarg.h>
#include <math.h>
#include <string.h>

#ifdef __APPLE__
#include <GLUT/glut.h>
#elif __linux
#include <GL/glut.h>
#endif

#include <ode/ode.h>

#include <vector>

#include <iostream>
#include <unordered_map>

#include "profiling.h"
#include "container.h"
#include "ThreeMaxLoader.h"

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

#include "units/Vehicle.h"
#include "structures/CommandCenter.h"

#include "engine.h"

extern std::vector<BoxIsland*> islands;

extern container<Vehicle*> entities;

extern  Controller controller;

extern unsigned long timer;

extern int width;
extern int height;
extern int mapzoom;
extern int cx,cy;


float X(float x);

float Y(float y);

void drawIntro()
{
    // This will make things dark.
    glDisable(GL_LIGHTING);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    int xsize = width/mapzoom;
    int ysize = height/mapzoom;

    if (mapzoom==1)
    {
        cx = width/2;
        cy = height/2;
    }

    glOrtho(cx-xsize/2, cx+xsize/2, cy+ysize/2, cy-ysize/2, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glColor4f(1.0f, 1.0f, 1.0f, 1);
    glDisable(GL_DEPTH_TEST);
    glRotatef(180.0f,0,0,1);
    glRotatef(180.0f,0,1,0);



    glClearColor(0.0f,0.0f,0.0f,1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    char str[256], str2[256], str3[256], str4[256], str5[256];

    static int letter=0, letter2=0, letter3=0, letter4=0, letter5=0;
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix(); {
        glTranslatef(0, -400, 1);

        memset(str,0,256);memset(str2,0,256);memset(str3,0,256);memset(str4,0,256);memset(str5,0,256);

        sprintf(str, "Wakuseibokan");
        if (getRandomInteger(1,2)==1)
            drawString(400,300,0,str,0.5f,0.1f,1.0f,1.0f);
        else
            drawString(400,300,0,str,0.5f,0.1f,1.0f,0.0f);

        strncpy(str, "Excited warfare on the seas of a Kepler IV.", letter);
        drawString(0,0,0,str,0.2f,0.1f,1.0f,1.0f);

        if (timer % 1 ==0 && letter <=43)
            letter++;

        strncpy(str2, "The time has come to conquer Kepler IV. Humanity is now able to travel further away ", letter2);
        drawString(0,-50,0,str2,0.2f,0.1f,1.0f,1.0f);
        strncpy(str3, "from our solar system, and it has now reached exoplanets. Two companies sent survey", letter3);
        drawString(0,-90,0,str3,0.2f,0.1f,1.0f,1.0f);
        strncpy(str4, "parties in the form of two AI carriers that aim to control the vast archipielago of ", letter4);
        drawString(0,-130,0,str4,0.2f,0.1f,1.0f,1.0f);
        strncpy(str5, "Kepler IV. These AIs can be controlled remotely in real-time by space folding uplinks.", letter5);
        drawString(0,-170,0,str5,0.2f,0.1f,1.0f,1.0f);

        if (letter >43 && timer % 10 ==0 && letter2 <=83)
            letter2++;
        if (letter2 >83 && timer % 8 ==0 && letter3 <=83)
            letter3++;
        if (letter3 >83 && timer % 6 ==0 && letter4 <=83)
            letter4++;
        if (letter4 >83 && timer % 4 ==0 && letter5 <=84)
            letter5++;

        if (letter5 >= 85)
        {
            strncpy(str, "Press ! to engage the uplink.", 30);
            drawString(400,-300,0,str,0.2f,1.0f,1.0f,1.0f);
        }

    } glPopMatrix();

    glEnable(GL_DEPTH_TEST);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glEnable(GL_LIGHTING);
}


void drawBoard()
{
    // This will make things dark.
    glDisable(GL_LIGHTING);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    int xsize = width/mapzoom;
    int ysize = height/mapzoom;

    if (mapzoom==1)
    {
        cx = width/2;
        cy = height/2;
    }

    glOrtho(cx-xsize/2, cx+xsize/2, cy+ysize/2, cy-ysize/2, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glColor4f(1.0f, 1.0f, 1.0f, 1);
    glDisable(GL_DEPTH_TEST);
    glRotatef(180.0f,0,0,1);
    glRotatef(180.0f,0,1,0);



    glClearColor(0.0f,0.0f,0.0f,1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    char str[256];

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix(); {
        glTranslatef(0, -400, 1);

        sprintf(str, "Message Board");
        drawString(500,300,0,str,0.2f,0.1f,1.0f,1.0f);

        std::ifstream messageboard("messageboard.dat");
        std::string message;

        // @FIXME: We need a slider because quickly we will run out of space.
        int counter = 0, showingcounter=0;;
        while (messageboard.good())
        {
            std::getline (messageboard, message);

            if (controller.slider<=counter)
            {
                size_t m = message.find("|");
                size_t n = message.find(":");
                if (n != std::string::npos && m != std::string::npos)
                {
                    std::string faction = message.substr(0,m);
                    std::string timestamp = message.substr(m+1,n);
                    std::string msg = message.substr(n+1);

                    // @NOTE: Check file integrity otherwise this will generate in unexpected behaviour.

                    if (controller.faction == FACTIONS::BOTH_FACTION || controller.faction == atoi(faction.c_str()))
                    {
                        char bfr[256];
                        sprintf(bfr, "%08ul:%s", atol(timestamp.c_str()), msg.c_str());

                        //std::cout << message << std::endl;
                        drawString(10,200-(showingcounter++)*25,0,(char *)bfr,0.2f,0.1f,1.0f,1.0f);
                    }
                }

            }
            counter++;
        }

        messageboard.close();

    } glPopMatrix();

    glEnable(GL_DEPTH_TEST);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glEnable(GL_LIGHTING);
}

void drawEntities()
{
    // This will make things dark.
    glDisable(GL_LIGHTING);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    int xsize = width/mapzoom;
    int ysize = height/mapzoom;

    if (mapzoom==1)
    {
        cx = width/2;
        cy = height/2;
    }

    glOrtho(cx-xsize/2, cx+xsize/2, cy+ysize/2, cy-ysize/2, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glColor4f(1.0f, 1.0f, 1.0f, 1);
    glDisable(GL_DEPTH_TEST);
    glRotatef(180.0f,0,0,1);
    glRotatef(180.0f,0,1,0);



    glClearColor(0.0f,0.0f,0.0f,1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    char str[256];

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix(); {
        glTranslatef(0, -400, 1);

        int counter = 0;
        for(size_t i=entities.first();entities.hasMore(i);i=entities.next(i))
        {
            CLog::Write(CLog::Debug,"[%d]: Body ID (%16p) Position (%d) Type: %d\n", i,(void*)entities[i]->getBodyID(), entities.indexOf(i), entities[i]->getType());
            sprintf(str, "[%3d]\t%s", i, entities[i]->getName().c_str());
            drawString(X(+550000.0-((int)(counter/78))*150000.0)-10,Y(+400000.0-(counter%78)*10000.0)-20,0,str,0.1f,1.0f,1.0f,1.0f);
            counter++;
        }


    } glPopMatrix();

    glEnable(GL_DEPTH_TEST);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glEnable(GL_LIGHTING);
}

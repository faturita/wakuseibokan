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

//int cx=1200/2,cy=800/2;

extern int cx, cy;


void drawBoard()
{
    // This will make things dark.

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    int xsize = 1200;
    int ysize = 800;



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
    sprintf (str, "Kepler IV Sea");
    // width, height, 0 0 upper left
    drawString(0,-30,1,str,0.2f,1.0f,1.0f,1.0f);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix(); {
        glTranslatef(0, -400, 1);

        int counter = 0;
        for(size_t i=entities.first();entities.hasMore(i);i=entities.next(i))
        {
            CLog::Write(CLog::Debug,"[%d]: Body ID (%16p) Position (%d) Type: %d\n", i,(void*)entities[i]->getBodyID(), entities.indexOf(i), entities[i]->getType());
            sprintf(str, "[%3d]\tType: %d", entities.indexOf(i), entities[i]->getType());
            drawString(10,300-counter*40,0,str,0.2f,1.0f,1.0f,1.0f);
            counter++;
        }


    } glPopMatrix();

    glEnable(GL_DEPTH_TEST);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

}

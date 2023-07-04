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

#include "map.h"


extern std::vector<BoxIsland*> islands;

extern container<Vehicle*> entities;

extern  Controller controller;

std::unordered_map<std::string, GLuint> maptextures;

extern int screen_width;
extern int screen_height;

// 1200 x 800
// 1920 x 1080
int width = 1200;
int height = 800;
int mapzoom=1;
int cx=width/2,cy=height/2;

/**
 * 0 --------------------------------- 1200 --------------------------------------------|
 *
 * 800
 * |
 * |
 * |                        600, 0 is the center
 * |
 * |
 * |
 * 0
 */


/**
 * +550kpf <----> -550 kpf
 *
 *  +400kpf
 *  |
 *  |
 *  |
 *  -400kpf
 */



void placeMark(int x, int y, int size, const char* modelName)
{
    GLuint _texture;

    if (maptextures.find(std::string(modelName)) == maptextures.end())
    {
        // @FIXME: This means that the image is loaded every time this mark is rendered, which is wrong.
        Image* image = loadBMP(modelName);
        _texture = loadTexture(image);
        delete image;

        maptextures[std::string(modelName)]=_texture;

    } else {
        _texture = maptextures[std::string(modelName)];
    }

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, _texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glColor3f(1.0f, 1.0f, 0.0f);

    glBegin(GL_QUADS);

    //Front face
    glNormal3f(0.0, 0.0f, 1.0f);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-size / 2 + x, -size / 2 + y, 0);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(size / 2 + x, -size / 2 + y, 0);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(size / 2 + x, size / 2 + y, 0);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(-size / 2 + x, size / 2 + y, 0);

    glEnd();

    // @NOTE: This is very important.
    glDisable(GL_TEXTURE_2D);
}

void placeIsland(int x, int y, int size, const char* modelName, const char *name)
{
    char str[256];

    GLuint _texture;

    if (maptextures.find(std::string(modelName)) == maptextures.end())
    {
        Image* image = loadBMP(modelName);
        _texture = loadTexture(image);
        delete image;

        maptextures[std::string(modelName)]=_texture;

    } else {
        _texture = maptextures[std::string(modelName)];
    }

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, _texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glColor3f(1.0f, 1.0f, 0.0f);

    glBegin(GL_QUADS);

    int BOX_SIZE=size;

    //Front face
    glNormal3f(0.0, 0.0f, 1.0f);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-BOX_SIZE / 2 + x, -BOX_SIZE / 2 + y, 0);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(BOX_SIZE / 2 + x, -BOX_SIZE / 2 + y, 0);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(BOX_SIZE / 2 + x, BOX_SIZE / 2 + y, 0);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(-BOX_SIZE / 2 + x, BOX_SIZE / 2 + y, 0);


    glEnd();
}

void zoommapin()
{
    if (mapzoom < 11)
        mapzoom++;
}

void zoommapout()
{
    if (mapzoom>1)
        mapzoom--;
}

void centermap(int ccx, int ccy)
{
    // @FIXME: Parametrize all the resolution values.
    int xsize = 1200/mapzoom;
    int ysize = 800/mapzoom;

    // Screen width and height come from OpenGL.
    cx = (int)(ccx*(xsize)/(float)screen_width)+cx-xsize/2; // 1440
    cy = (int)(ccy*(ysize)/(float)screen_height)+cy-ysize/2;  // 900
}

Vec3f setLocationOnMap(int ccx, int ccy)
{
    // @FIXME: Parametrize all the resolution values.
    int xsize = 1200/mapzoom;
    int ysize = 800/mapzoom;

    // Screen width and height come from OpenGL.
    ccx = (int)(ccx*(xsize)/(float)screen_width)+cx-xsize/2;
    ccy = (int)(ccy*(ysize)/(float)screen_height)+cy-ysize/2;

    // Map coordinates in kmf are centered at (0,0) which is the center of the screen.  Positive is upwards, left.
    ccx = ccx - 600;
    ccy = ccy - 400;

    ccy = ccy * (-1);
    ccx = ccx * (-1);

    Vec3f loc(ccx*1 kmf, 0.0f, ccy * 1 kmf);

    return loc;
}


float X(float x)
{
    return (600-(x/1000));
}

float Y(float y)
{
    return 0+(y/1000);
}

void placeMark(Vec3f pos, int iconsize, const char* modelName)
{
    placeMark(X(pos[0]),Y(pos[2]),iconsize,modelName);
}

void placeIsland(BoxIsland *b, int iconsize)
{
    placeIsland(X(b->getX()),Y(b->getZ()),iconsize, b->getModelName().c_str(), b->getName().c_str());
}


void drawMap()
{
    // This will make things dark.

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
    sprintf (str, "Kepler IV Sea");
    // width, height, 0 0 upper left
    drawString(0,-30,1,str,0.2f,1.0f,1.0f,1.0f);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix(); {
        glTranslatef(0, -400, 1);

        // Let's draw the typical grid of naval cartmaking.  It is 12x8. Each square is 100 kmf squared.
        // (+,0,0),(0,0,+) is upwards-right.  The screen is (+,0),(0,+) downward-right.
        for(int i=0;i<10;i++)
        {
            glLineWidth(2.5);
            glColor3f(0.0, 1.0, 0.0);
            glBegin(GL_LINES);
            glVertex3f(   1, -500+i*100, 0.0);
            glVertex3f(1200, -500+i*100, 0.0);
            glEnd();
        }

        for(int i=0;i<12;i++)
        {
            glLineWidth(2.5);
            glColor3f(0.0, 1.0, 0.0);
            glBegin(GL_LINES);
            glVertex3f(   1+i*100,  500, 0.0);
            glVertex3f(   1+i*100, -500, 0.0);
            glEnd();
        }

        glLineWidth(2.5);
        glColor3f(0.0, 1.0, 0.0);
        glBegin(GL_LINES);
        glVertex3f(   1200,  500, 0.0);
        glVertex3f(   1200, -500, 0.0);
        glEnd();


        /**
        glLineWidth(2.5);
        glColor3f(1.0, 0.0, 0.0);
        glBegin(GL_LINES);
        glVertex3f(590, 0.0, 0.0);
        glVertex3f(690, 0, 0);
        glEnd();


        glBegin(GL_LINES);
        glVertex3f(590, 1, 0.0);
        glVertex3f(690, + 1, 0);
        glEnd();

        **/


        for(size_t i=0;i<islands.size();i++)
        {
            BoxIsland *b = islands[i];

            {//        600-(b->getX()/1000)-10,(b->getZ()/1000)-20
            int cx= X(b->getX());
            int cy=Y(b->getZ());
            float r =20;
                glLineWidth(2.0f);
                glColor3f(1.0f, 1.0f, 1.0f);
                glBegin(GL_LINE_LOOP);
                glNormal3f(0.0, 0.0f, 1.0f);
                for (int ii = 0; ii < 100; ii++)   {
                    float theta = 2.0f * 3.1415926f * float(ii) / float(100);//get the current angle
                    float x = r * cosf(theta);//calculate the x component
                    float y = r * sinf(theta);//calculate the y component
                    glVertex3f(x + cx, y + cy, 0.0f);//output vertex
                }
                glEnd();
            }
        }

        // Let's show all the islands.  Green and Blue faction.
        for(size_t i=0;i<islands.size();i++)
        {
            BoxIsland *b = islands[i];

            Structure *d = b->getCommandCenter();

            // The color in the map defines the faction.
            Vec3f color(1.0f,1.0f,1.0f);

            if (d)
            {
                CommandCenter *cd = (CommandCenter*)d;

                if (cd->getFaction()==GREEN_FACTION)
                {
                    color[0]=0.0f;color[1]=1.0f;color[2]=0;
                } else {
                    color[0]=0.0f;color[1]=0.0f;color[2]=1.0;
                }

            }

            drawString(X(b->getX())-10,Y(b->getZ())-20,0,(char*)b->getName().c_str(),0.1f,color[0],color[1],color[2]);
        }

        int iconsize = 10;

        if (mapzoom>5)
            iconsize = 4;

        // Now show all the available units.
        synchronized(entities.m_mutex)
        {
            for(size_t i=entities.first();entities.hasMore(i);i=entities.next(i))
            {
                if (entities[i]->getFaction() == controller.faction || controller.faction == BOTH_FACTION)
                {
                    if (entities[i]->getType() == CARRIER)
                    {
                        placeMark(entities[i]->getPos(),iconsize,"units/carriertarget.bmp");
                    } else if (entities[i]->getType() == WALRUS)
                    {
                        placeMark(entities[i]->getPos(),iconsize,"units/walrusicon.bmp");
                    } else if (entities[i]->getType() == MANTA)
                    {
                        if (entities[i]->getSubType() == MEDUSA)
                        {
                            placeMark(entities[i]->getPos(),iconsize,"units/mantaicon.bmp");

                        } else
                        {
                            placeMark(entities[i]->getPos(),iconsize,"units/mantaicon.bmp");
                        }
                    }
                }
            }
        }

        for(size_t i=0;i<islands.size();i++)
        {
            BoxIsland *b = islands[i];

            // The size '5' is 4000x4000 (instead of 3600x3600) which is the real size of every island.
            placeIsland(b,iconsize);
        }

        // @FIXME: This sould be a list of targets and icons to show on the map.
        if (controller.controllingid != CONTROLLING_NONE)
        {
            Vehicle *_b = entities[controller.controllingid];

            Vec3f target = _b->getDestination();

            if (target.magnitude()>=1)
            {
                placeMark(target,iconsize,"units/target.bmp");
            }
        }



    } glPopMatrix();

    glEnable(GL_DEPTH_TEST);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

}

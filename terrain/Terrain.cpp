/* Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above notice and this permission notice shall be included in all copies
 * or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* 
 * Terrain Class - Handle island generation and heightmaps.
 *
 */
#include <stdlib.h>

#define dSINGLE

#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include "../imageloader.h"
#include "../math/vec3f.h"
#include "Terrain.h"

// Islands of 3600x3600 (based on heightmaps of 60x60). Max height is 100m.
// (remember that the zero value of the heightmap for the island is 5 m bellow sea level).
#define TERRAIN_SIDE_LENGTH 60.0f
#define TERRAIN_MAX_HEIGHT  100.0f
#define TERRAIN_SCALE       60.0f


// The texture is preloaded somewhere else.
extern GLuint _textureLand;


using namespace std;

//
// Loads a terrain from a heightmap.  The heights of the terrain range from
//   -height / 2 to height / 2.
//
Terrain* loadTerrain(const char* filename, float height)
{
    Image* image = loadBMP(filename);
    Terrain* t = new Terrain(image->width, image->height);
    for(int y = 0; y < image->height; y++) {
        for(int x = 0; x < image->width; x++) {
            unsigned char color =
            (unsigned char)image->pixels[3 * (y * image->width + x)];
            float h = height * ((color / 255.0f) - 0.5f);
            t->setHeight(x, y, h+height/2.0);
            //printf("%4d,%4d,%10.5f\n", x,y,h+height/2.0);
        }
    }
    
    delete image;
    t->computeNormals();
    return t;
}


//
// This function is called by the world modeller to map x,z coordinates to heights.
//
// @param pUserData contains the pointer to Terrain object that can be used to make the mapping.
//
//
dReal hedightfield_callback( void* pUserData, int x, int z )
{
    static dReal h = 5000;
    
    Terrain *_landmass = (Terrain*) pUserData;
    
    h = _landmass->getHeight(x, z);
       
    //printf("%4d,%4d,%10.5f\n", x,z,h);
    
    return h;
}

dGeomID BoxIsland::buildTerrainModel(dSpaceID space, const char *model )
{
    _landmass = loadTerrain(model, TERRAIN_MAX_HEIGHT);
    
    modelname = model;

    printf("Island: %s\n", model);
    printf("Landmass width: %d\n", _landmass->width());
    printf("Landmass heigth: %d\n",_landmass->length());
    
    dHeightfieldDataID heightid = dGeomHeightfieldDataCreate();
    
    float realside = TERRAIN_SIDE_LENGTH * TERRAIN_SCALE * REAL( 1.0 );
    
    // Create a finite heightfield.
    dGeomHeightfieldDataBuildCallback( heightid, _landmass, hedightfield_callback,
                                      realside , realside, HFIELD_WSTEP, HFIELD_DSTEP,
                                      REAL( 1.0 ), REAL( 0.0 ), REAL( 0.0 ), 0 );
    
    // These boundaries are used to limit how the heightfield affects objects.
    dGeomHeightfieldDataSetBounds( heightid, REAL( -4.0 ), TERRAIN_MAX_HEIGHT ); // +6000 decia
    
    dGeomID gheight = dCreateHeightfield( space, heightid, 1 );
    
    dGeomSetPosition( gheight, X, Y, Z );

    islandGeom = gheight;
    
    return gheight;
}

dGeomID BoxIsland::getGeom()
{
    return islandGeom;
}

std::string BoxIsland::getModelName()
{
    return modelname;
}

float BoxIsland::getX()
{
    return X;
}
float BoxIsland::getZ()
{
    return Z;
}

std::string BoxIsland::getName()
{
    return name;
}

void BoxIsland::setName(std::string newname)
{
    BoxIsland::name = newname;
}

void BoxIsland::setLocation(float x, float y, float z)
{
    X = x;
    Y = y;
    Z = z;
}

void BoxIsland::draw()
{
    glPushMatrix();
    //glTranslatef(300.0, 0.0f,300.0);
    glTranslatef(X,Y,Z);
    drawTerrain(_landmass,TERRAIN_SCALE);
    glPopMatrix();
    
    //drawBoxIsland(300,5,300,600,5*2);
}

/**
 * x, z values are SCALED.  They are relative to the island center in 3600x3600 dimensions (3.6 km)
 *
 * @brief BoxIsland::addStructure
 * @param structure
 * @param x
 * @param z
 * @param space
 * @param world
 * @return
 */
Structure* BoxIsland::addStructure(Structure* structure, float x, float z, dSpaceID space, dWorldID world)
{
    // This should be half the height of the structure. @FIXME
    float heightOffset = 5.0+_landmass->getHeight((int)(x/TERRAIN_SCALE)+TERRAIN_SCALE/2,(int)(z/TERRAIN_SCALE)+TERRAIN_SCALE/2);
    structure->init();
    structure->setPos(X+x,heightOffset,Z+z);
    structure->embody(world,space);
    structure->onIsland(this);

    // @NOTE: when the structure is destroyed this pointer must be eliminated.
    structures.push_back(structure);

    return structure;
}

void BoxIsland::tick()
{
    //structures.push_back(addStructure(new Runway(),       0.0f,    0.0f,space,world));
    //structures.push_back(addStructure(new Hangar()   , -550.0f,    0.0f,space,world));
    //structures.push_back(addStructure(new Turret()   ,  100.0f, -100.0f,space,world));

}

void drawTerrain(Terrain *_landmass, float fscale)
{
    drawTerrain(_landmass, fscale, 0.3f, 0.9f, 0.0f);
}



void drawTerrain(Terrain *_landmass, float fscale,float r,float g,float b)
{
    float scale = fscale; //1.0f;//fscale / max(_landmass->width() - 1, _landmass->length() - 1);
    glScalef(scale, 1, scale);
    glTranslatef(-(float)(_landmass->width() - 1) / 2,
                 0.0f,
                 -(float)(_landmass->length() - 1) / 2);
    
    //glColor3f(r,g,b);
    
    
    
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, _textureLand);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glColor3f(1.0f, 1.0f, 1.0f);
    
    
    
    
    for(int z = 0; z < _landmass->length() - 1; z++) {
        //Makes OpenGL draw a triangle at every three consecutive vertices
        glBegin(GL_TRIANGLE_STRIP);
        
        float t0 = (float)z / (_landmass->length() - 1); // t0 in [0,1]
        float t1 = (float)(z + 1) / (_landmass->length() - 1); // t1 in [0,1]
        
        for(int x = 0; x < _landmass->width(); x++) {
            Vec3f normal = _landmass->getNormal(x, z);
            glNormal3f(normal[0], normal[1], normal[2]);
            //glVertex3f(x, hedightfield_callback(_landmass,x,z),z);
            float s = (float)x / (_landmass->width()- 1); // s in [0, 1]
            glTexCoord2f(s, t0);
            
            glVertex3f(x, _landmass->getHeight(x, z), z);
            normal = _landmass->getNormal(x, z + 1);
            glNormal3f(normal[0], normal[1], normal[2]);
            glTexCoord2f(s, t1);
            
            glVertex3f(x, _landmass->getHeight(x, z + 1), z + 1);
            //glVertex3f(x, hedightfield_callback(_landmass,x,z+1), z+1);
        }
        glEnd();
    }
}

void buildTerrainModel(dSpaceID space, Terrain *_landmass, float fscale,float xx,float yy,float zz)
{
    float slopeData[_landmass->width()*_landmass->length()];
    float scale = fscale / fmax(_landmass->width() - 1, _landmass->length() - 1);
    fscale = 10.0f;
    for(int z = 0; z < _landmass->length() - 1; z++) {

        for(int x = 0; x < _landmass->width(); x++) {
            slopeData[z*_landmass->width() +x] = 0; /**_landmass->getHeight(x, z)*fscale+**/;
        }
    }

    float xsamples = _landmass->width(),zsamples = _landmass->width(), xdelta = 10, zdelta =10;

    dHeightfieldDataID slopeHeightData = dGeomHeightfieldDataCreate (); // data geom

    float width = xsamples*xdelta; // 5 samples at delta of 1 unit
    float depth = zsamples*zdelta; // 5 samples at delta of 1 unit

    dGeomHeightfieldDataBuildSingle(slopeHeightData,slopeData,
                                    0,width,depth, xsamples, zsamples, 1.0f, 5.0f,10.0f, 0); // last 4

    //dGeomHeightfieldDataSetBounds (slopeHeightData, 0.0f, 100.0f); // sort

    dGeomID slopeHeightID = dCreateHeightfield(space, slopeHeightData, 1); // fff

    dGeomSetPosition(slopeHeightID,xx,yy,zz);

}


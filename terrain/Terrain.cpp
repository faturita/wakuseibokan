#include <stdlib.h>

#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include "../imageloader.h"
#include "../math/vec3f.h"
#include "Terrain.h"

using namespace std;

//Loads a terrain from a heightmap.  The heights of the terrain range from
//-height / 2 to height / 2.
Terrain* loadTerrain(const char* filename, float height) {
	Image* image = loadBMP(filename);
	Terrain* t = new Terrain(image->width, image->height);
	for(int y = 0; y < image->height; y++) {
		for(int x = 0; x < image->width; x++) {
			unsigned char color =
				(unsigned char)image->pixels[3 * (y * image->width + x)];
			float h = height * ((color / 255.0f) - 0.5f);
			t->setHeight(x, y, h+height/2.0);
            printf("%4d,%4d,%10.5f\n", x,y,h+height/2.0);
		}
	}

	delete image;
	t->computeNormals();
	return t;
}


//
// This function is called by the world modeller to map x,z coordinates to heights.
dReal hedightfield_callback( void* pUserData, int x, int z )
{
    static dReal h = 5000;
    
    Terrain *_landmass = (Terrain*) pUserData;
    
    h = _landmass->getHeight(x, z);

    //h = (-1) * fabs(x - 30) + 30;
    
    
    //printf("%4d,%4d,%10.5f\n", x,z,h);
    
    return h;
}

dGeomID BoxIsland::buildTerrainModel(dSpaceID space, const char *model )
{
    //_terrain = loadTerrain("terrain/island.bmp", 20);
    
    //_vulcano = loadTerrain("terrain/vulcano.bmp",20);
    
    //_baltimore = loadTerrain("terrain/baltimore.bmp", 40);
    
    //_landmass = loadTerrain("terrain/baltimore.bmp", 60);
    
    _landmass = loadTerrain(model, 60);
    
    printf("Landmass width: %d\n", _landmass->width());
    printf("Landmass heigth: %d\n",_landmass->length());
    
    dHeightfieldDataID heightid = dGeomHeightfieldDataCreate();
    
    // Create a finite heightfield.
    dGeomHeightfieldDataBuildCallback( heightid, _landmass, hedightfield_callback,
                                      REAL( 600.0 ), REAL (600.0), HFIELD_WSTEP, HFIELD_DSTEP,
                                      REAL( 1.0 ), REAL( 0.0 ), REAL( 0.0 ), 0 );
    
    // These boundaries are used to limit how the heightfield affects objects.
    dGeomHeightfieldDataSetBounds( heightid, REAL( -4.0 ), REAL( +6000.0 ) );
    
    dGeomID gheight = dCreateHeightfield( space, heightid, 1 );
    
    dGeomSetPosition( gheight, X, Y, Z );
    
    return gheight;
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
        drawTerrain(_landmass,10.0f);
        glPopMatrix();
        
        //drawBoxIsland(300,5,300,600,5*2);
    }

void drawTerrain(Terrain *_landmass, float fscale)
{
    drawTerrain(_landmass, fscale, 0.3f, 0.9f, 0.0f);
}

extern GLuint _textureLand;

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



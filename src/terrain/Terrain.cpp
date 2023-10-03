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

#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include "../profiling.h"
#include "../imageloader.h"
#include "../math/vec3f.h"
#include "Terrain.h"

// Islands of 3600x3600 (based on heightmaps of 60x60). Max height is 100m.
// (remember that the zero value of the heightmap for the island is 5 m bellow sea level).
#define TERRAIN_SIDE_LENGTH 60.0f
#define TERRAIN_MAX_HEIGHT  100.0f
#define TERRAIN_SCALE       60.0f


// The texture is preloaded somewhere else.
extern std::unordered_map<std::string, GLuint> textures;


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

            // @NOTE:  Heightmap color 0 is FIXME
            //if (color == 1) h =  height * (((unsigned char)0 / 255.0f) - 0.5f);
            //if (color == 0) h =  -70.0f;
            t->setHeight(x, y, h+height/2.0);
            //CLog::Write(CLog::Debug,"%4d,%4d,%10.5f\n", x,y,h+height/2.0);
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
       
    //CLog::Write(CLog::Debug,"%4d,%4d,%10.5f\n", x,z,h);
    
    return h;
}

BoxIsland::BoxIsland(container<Vehicle*> *enti)
{
    entities = enti;
}
dGeomID BoxIsland::buildTerrainModel(dSpaceID space, const char *model )
{
    _landmass = loadTerrain(model, TERRAIN_MAX_HEIGHT);

    modelname = model;

    islandspace = dHashSpaceCreate(space);
    //islandspace = space;

    CLog::Write(CLog::Debug,"Island: %s\n", model);
    CLog::Write(CLog::Debug,"Landmass width: %d\n", _landmass->width());
    CLog::Write(CLog::Debug,"Landmass heigth: %d\n",_landmass->length());
    
    dHeightfieldDataID heightid = dGeomHeightfieldDataCreate();
    
    float realside = TERRAIN_SIDE_LENGTH * TERRAIN_SCALE * REAL( 1.0 );
    
    // Create a finite heightfield.
    dGeomHeightfieldDataBuildCallback( heightid, _landmass, hedightfield_callback,
                                      realside , realside, HFIELD_WSTEP, HFIELD_DSTEP,
                                      REAL( 1.0 ), REAL( 0.0 ), REAL( 0.0 ), 0 );
    
    // These boundaries are used to limit how the heightfield affects objects.
    dGeomHeightfieldDataSetBounds( heightid, REAL( -4.0 ), TERRAIN_MAX_HEIGHT ); // +6000 decia
    
    dGeomID gheight = dCreateHeightfield( islandspace, heightid, 1 );
    
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

Vec3f BoxIsland::getPos()
{
    return Vec3f(getX(), 0.0, getZ());
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

Vec3f BoxIsland::getPosAtDesiredHeight(float desiredHeight)
{
    float heightOffset = 0;
    float x = 0;
    float z = 0;

    for (int j=0;j<4;j++)
    {
        float t = (rand() % 360 + 1);

        t = t * PI/180.0f;
        float adjusted = t * 4.0/(2.0*PI);
        int rounded = round(adjusted);
        adjusted = rounded * (2.0*PI)/4.0 + PI/2.0;


        for(int radius = 0;radius<2500;radius++)
        {
            x = cos(t);
            z = sin(t);

            x = x * radius;
            z = z * radius;

            if (x>1799) x = 1799;
            if (x<-1799) x = -1799;
            if (z>1799) z = 1799;
            if (z<-1799) z = -1799;

            assert ( _landmass != NULL || !"Landmass is null !  This is quite weird.");

            // @FIXME Put this line in a different function and use it from there.  Repeated code here.
            heightOffset = +_landmass->getHeight((int)(x/TERRAIN_SCALE)+TERRAIN_SCALE/2,(int)(z/TERRAIN_SCALE)+TERRAIN_SCALE/2);

            CLog::Write(CLog::Debug,"(%10.5f - %10.5f  %d: %10.5f,%10.5f) Desired %10.5f vs Height %10.5f\n", t*180.0/PI, adjusted, radius,x,z,desiredHeight, heightOffset);

            if (  heightOffset>desiredHeight )
            {
                return Vec3f(x, heightOffset, z);
            }
        }
    }
    assert (false || !"You are asking for a height which is invalid for this island.");

    return getPos();
}




/**
 * Add the specified structure in a desirable "desiredHeight".  Get a random angle and determines the position in polar coordinate.  After that start
 * from the center and try to find the closed height to the desired one.  As all the islands fit into the 36x36, there should be one height that matches.
 *
 * @brief BoxIsland::addStructureAtDesiredHeight
 * @param structure
 * @param world
 * @param desiredHeight
 * @return
 */
Structure* BoxIsland::addStructureAtDesiredHeight(Structure *structure, dWorldID world, float desiredHeight)
{
    float heightOffset = 0;
    float x = 0;
    float z = 0;
    float angle = 0;

    float t = (rand() % 360 + 1);

    t = t * PI/180.0f;
    float adjusted = t * 4.0/(2.0*PI);
    int rounded = round(adjusted);
    adjusted = rounded * (2.0*PI)/4.0 + PI/2.0;

    int minradius = 2500;
    float mindistance = 60;

    for(int radius = 2500;radius>0;radius--)
    {
        x = cos(t);
        z = sin(t);

        x = x * radius;
        z = z * radius;

        if (x>1799) x = 1799;
        if (x<-1799) x = -1799;
        if (z>1799) z = 1799;
        if (z<-1799) z = -1799;

        assert ( _landmass != NULL || !"Landmass is null !  This is quite weird.");

        // @FIXME Put this line in a different function and use it from there.  Repeated code here.
        heightOffset = +_landmass->getHeight((int)(x/TERRAIN_SCALE)+TERRAIN_SCALE/2,(int)(z/TERRAIN_SCALE)+TERRAIN_SCALE/2);

        //dout << heightOffset << std::endl;

        if (  abs(heightOffset-desiredHeight)<0.5 )
        {
            return addStructure(structure,x,z,-t+3*(PI/2), world);
        }

        if ( abs(heightOffset-desiredHeight)<mindistance )
        {
            minradius = radius;
            mindistance = abs(heightOffset-desiredHeight);
        }

    }

    if (mindistance < 60)
    {
        x = cos(t);
        z = sin(t);

        x = x * minradius;
        z = z * minradius;

        if (x>1799) x = 1799;
        if (x<-1799) x = -1799;
        if (z>1799) z = 1799;
        if (z<-1799) z = -1799;

        return addStructure(structure,x,z,-t+3*(PI/2), world);
    }

#ifdef DEBUG
    assert( false || !"I did my best but couldn't find a place where to put what you want me to put at the desired place.");
#endif

    // @NOTE Give up.
    return addStructure(structure,x,z,-t+3*(PI/2), world);
}


/**
 * x, z values are SCALED.  They are relative to the island center in 3600x3600 dimensions (3.6 km)
 * This function generates x,z.  Only values that are above certain height are used to generate the structure.
 *
 * @brief BoxIsland::addStructure
 * @param structure
 * @param space
 * @param world
 * @return
 */
Structure* BoxIsland::addStructure(Structure *structure, dWorldID world)
{
    float heightOffset = 0;
    float x = 0;
    float z = 0;
    float angle = 0;

    int breakcounter = 0;

    // @FIXME: While putting docks I need to calculate the heightmap gradient and use that orientation.
    do {

        // @NOTE Make a parameter out of the size of the island.
        x = (rand() % 3550 + 1); x -= 1800;
        z = (rand() % 3550 + 1); z -= 1800;
        angle = (rand() % 360) * (2.0*PI/360);

        assert ( _landmass != NULL || !"Landmass is null !  This is quite weird.");

        // @FIXME Put this line in a different function and use it from there.  Repeated code here.
        heightOffset = +_landmass->getHeight((int)(x/TERRAIN_SCALE)+TERRAIN_SCALE/2,(int)(z/TERRAIN_SCALE)+TERRAIN_SCALE/2);

        CLog::Write(CLog::Debug,"Height %10.5f\n", heightOffset);

        if (breakcounter++==10) break;

    } while (!structure->checkHeightOffset(heightOffset));

    return addStructure(structure,x,z,angle, world);

}

/**
 * x, z values are SCALED.  They are relative to the island center in 3600x3600 dimensions (3.6 km)
 *
 * @brief BoxIsland::addStructure
 * @param structure
 * @param x
 * @param z
 * @param angle in Radians !
 * @param space
 * @param world
 * @return
 */
Structure* BoxIsland::addStructure(Structure* structure, float x, float z, float angle, dWorldID world)
{
    // This should be half the height of the structure. @FIXME
    float heightOffset = +_landmass->getHeight((int)(x/TERRAIN_SCALE)+TERRAIN_SCALE/2,(int)(z/TERRAIN_SCALE)+TERRAIN_SCALE/2);
    structure->init();
    structure->embody(world,islandspace);
    structure->setPos(X+x,heightOffset,Z+z);
    structure->rotate(angle);
    structure->onIsland(this);

    // @NOTE:  Once structures are destroyed, this reference will be dangling.  However, checkstructures will verify that all the indeces
    //    that are stored here are valid, and will delete those which are not.  Some form of prunning.
    size_t structureId = entities->push_back(structure,structure->getGeom());
    structures.push_back(structureId);

    if (structure->getType() == CONTROL)
        commandCenterId = structureId;          //@NOTE: commandCenterId is an index for entities !

    return structure;
}

void BoxIsland::checkStructures()
{
    for(int i=0;i<structures.size();i++)
    {
        // @NOTE:  once structuers are deleted, even if they are replaced by other vehicles, this will check if the structure yet belong
        //    to the island and will delete it otherwise.  @FIXME: check if this is a structure but belongs to another island.
        if (!(entities->isValid(structures[i]) && entities->operator[](structures[i])->getType()>=COLLISIONABLE) )
        {
            structures.erase(structures.begin()+i);
        }
    }
}


std::vector<size_t> BoxIsland::getStructures()
{
    checkStructures();
    return structures;
}


Structure* BoxIsland::getCommandCenter()
{
    // Just a small improvement.  When the command center is there, it is accessed directly.
    if (commandCenterId !=0 &&
            (entities->isValid(commandCenterId)) &&
            (entities->operator[](commandCenterId)->getType()==CONTROL)   )
    {
        Structure *s = (Structure*) entities->operator[](commandCenterId);
        return s;
    }

    checkStructures();
    for(int i=0;i<structures.size();i++)
    {
        Structure* s = NULL;
        if (entities->isValid(structures[i]) && entities->operator[](structures[i])->getType()>=COLLISIONABLE)
        {
            Structure *s = (Structure*) entities->operator[](structures[i]);
            if (s && s->getType() == CONTROL)
                return s;
        }
    }
    return NULL;
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
    glBindTexture(GL_TEXTURE_2D, textures["land"]);
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
            
            // @NOTE: Z-Fighting.  if the height is 0, move only the view a little bit lower.
            glVertex3f(x, (  _landmass->getHeight(x, z) <= 1 ? _landmass->getHeight(x, z)-10.9 : _landmass->getHeight(x, z) ) , z);
            normal = _landmass->getNormal(x, z + 1);
            glNormal3f(normal[0], normal[1], normal[2]);
            glTexCoord2f(s, t1);
            
            // @NOTE: Z-Fighting.  if the height is 0, move only the view a little bit lower.
            glVertex3f(x, (_landmass->getHeight(x, z + 1)<=1?_landmass->getHeight(x, z + 1)-10.9:_landmass->getHeight(x, z + 1)), z + 1);
            //glVertex3f(x, hedightfield_callback(_landmass,x,z+1), z+1);
        }
        glEnd();
    }

    glDisable(GL_TEXTURE_2D);
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


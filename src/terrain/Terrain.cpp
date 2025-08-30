/* ============================================================================
**
** Terrain - Wakuseiboukan - 22/05/2014
**
** Handle island generation and ODE heightmap.
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

#include <stdlib.h>
#include <fstream>

#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include "../FractalNoise.h"
#include "../profiling.h"
#include "../imageloader.h"
#include "../math/vec3f.h"
#include "../openglutils.h"
#include "Terrain.h"

// Islands of 3600x3600 (based on heightmaps of 60x60). Max height is 100m.
// (remember that the zero value of the heightmap for the island is 5 m bellow sea level).
#define TERRAIN_SIDE_LENGTH 60
#define TERRAIN_MAX_HEIGHT  100.0f
#define TERRAIN_SCALE       60.0f


// The texture is preloaded somewhere else.
extern std::unordered_map<std::string, GLuint> textures;


using namespace std;


Terrain* loadFractalTerrain(float height)
{
    Terrain* t = new Terrain(60, 60);

    int sideLength = 65;	// Accepts 2^n+1, for (4 <= n <= 10).
    TFracVal* buff = new TFracVal[65*65];
    CFractalNoise myNoise(buff, sideLength);

    //
    for (int i=0 ; i<sideLength*sideLength ; i++)
        buff[i] = FRACVAL_UNINIT;


    myNoise.SetVal(0,0,0);
    myNoise.SetVal(sideLength-1,0,0);
    myNoise.SetVal(0, sideLength-1,0);
    myNoise.SetVal(sideLength-1,sideLength-1,0);


    //
    //  3. Make some noise with the Generate() interface:
    //
    int seed = getRandomInteger(1,1000);
    float adjust = 32000.0f;
    float frequency = 2.5f;
    myNoise.Generate(seed, adjust, frequency);



    for(int y = 0; y < 60; y++) {
        for(int x = 0; x < 60; x++) {
            unsigned char color = 0;
            float h = height * ((color / 255.0f) - 0.5f);
            t->setHeight(x, y, h+height/2.0);
        }
    }

    int r = getRandomInteger(1,4);

    for(int y = 1; y < 59; y++) {
        for(int x = 1; x < 59; x++) {
            float h = myNoise.GetVal(x,y) /100.0;

            // @NOTE:  Heightmap color 0 is FIXME
            //if (color == 1) h =  height * (((unsigned char)0 / 255.0f) - 0.5f);
            //if (color == 0) h =  -70.0f;

            if (r==1) t->setHeight(y, x, h+height/2.0);
            else if (r==2) t->setHeight(59-x, y, h+height/2.0);
            else if (r==3) t->setHeight(x, 59-y, h+height/2.0);
            else t->setHeight(x, y, h+height/2.0);
            //CLog::Write(CLog::Debug,"%4d,%4d,%10.5f\n", x,y,h+height/2.0);
        }
    }


    delete [] buff;
    buff = 0;
    //
    //	Note: To create an island approximation, pre-seed the buffer 'edges' with zero and
    //	the centre point with some positive value.

    t->computeNormals();
    return t;
}

Terrain* loadRegularTerrain(float height)
{
    Terrain* t = new Terrain(60, 60);

    for(int y = 0; y < 60; y++) {
        for(int x = 0; x < 60; x++) {
            unsigned char color = 0;
            float h = height * ((color / 255.0f) - 0.5f);

            // @NOTE:  Heightmap color 0 is FIXME
            //if (color == 1) h =  height * (((unsigned char)0 / 255.0f) - 0.5f);
            //if (color == 0) h =  -70.0f;
            t->setHeight(x, y, h+height/2.0);
            //CLog::Write(CLog::Debug,"%4d,%4d,%10.5f\n", x,y,h+height/2.0);
        }
    }

    for(int y = 10; y < 50; y++) {
        for(int x = 10; x < 50; x++) {
            unsigned char color = 15;
            float h = height * ((color / 255.0f) - 0.5f);

            // @NOTE:  Heightmap color 0 is FIXME
            //if (color == 1) h =  height * (((unsigned char)0 / 255.0f) - 0.5f);
            //if (color == 0) h =  -70.0f;
            t->setHeight(x, y, h+height/2.0);
            //CLog::Write(CLog::Debug,"%4d,%4d,%10.5f\n", x,y,h+height/2.0);
        }
    }


    for(int y = 25; y < 35; y++) {
        for(int x = 25; x < 35; x++) {
            unsigned char color = 20;
            float h = height * ((color / 255.0f) - 0.5f);

            // @NOTE:  Heightmap color 0 is FIXME
            //if (color == 1) h =  height * (((unsigned char)0 / 255.0f) - 0.5f);
            //if (color == 0) h =  -70.0f;
            t->setHeight(x, y, h+height/2.0);
            //CLog::Write(CLog::Debug,"%4d,%4d,%10.5f\n", x,y,h+height/2.0);
        }
    }

    t->computeNormals();
    return t;
}

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
            if (color == 0) h =  -70.0f;
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

extern std::unordered_map<std::string, GLuint> maptextures;

dGeomID BoxIsland::buildTerrainModel(dSpaceID space, Terrain *p_landmass, const char *model )
{
    _landmass = p_landmass;
    modelname = model;

    char *pixels2 = new char[TERRAIN_SIDE_LENGTH * TERRAIN_SIDE_LENGTH * 3];

    for(int x=0;x<60;x++)
        for(int y=0;y<60;y++)
        {

            float height = _landmass->getHeight(x,y);

            height = (height - 0) / (255 - 0);
            height = height * (255-80) + 80;


            pixels2[x*60*3 + y*3 + 0] = (char)(height/TERRAIN_MAX_HEIGHT)*255;
            pixels2[x*60*3 + y*3 + 1] = (char)(height/TERRAIN_MAX_HEIGHT)*255;
            pixels2[x*60*3 + y*3 + 2] = (char)(height/TERRAIN_MAX_HEIGHT)*255;
        }

    Image image(pixels2,TERRAIN_SIDE_LENGTH,TERRAIN_SIDE_LENGTH);

    GLuint _texture;

    if (maptextures.find(std::string(modelname)) == maptextures.end())
    {
        _texture = loadTexture(&image);

        maptextures[std::string(modelname)]=_texture;

    }


    // This is the ODE space for the island.
    islandspace = dHashSpaceCreate(space);


    // @NOTE: Image destructor eliminates the pixels2 buffer.
    return buildModel();
}


dGeomID BoxIsland::buildFractalTerrainModel(dSpaceID space )
{
    dynamic_island = true;
    return buildTerrainModel(space, loadFractalTerrain(TERRAIN_MAX_HEIGHT), getName().c_str());
}


dGeomID BoxIsland::buildRegularTerrainModel(dSpaceID space )
{
    dynamic_island = true;
    return buildTerrainModel(space, loadRegularTerrain(TERRAIN_MAX_HEIGHT), getName().c_str());
}

dGeomID BoxIsland::buildTerrainModel(dSpaceID space, const char *model )
{
        _landmass = loadTerrain(model, TERRAIN_MAX_HEIGHT);

        modelname = model;
        islandspace = dHashSpaceCreate(space);
        //islandspace = space;

        return buildModel();
}



dGeomID BoxIsland::buildModel()
{

    CLog::Write(CLog::Debug,"Island: %s\n", modelname.c_str());
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

float BoxIsland::getHeight(float x, float z)
{
    if (x>1799) return 0;
    if (x<-1799) return 0;
    if (z>1799) return 0;
    if (z<-1799) return 0;

    x = x-getPos()[0];
    z = z-getPos()[2];

    x = x / TERRAIN_SCALE;
    z = z / TERRAIN_SCALE;

    x += TERRAIN_SCALE/2;
    z += TERRAIN_SCALE/2;

    return _landmass->getHeight((int)x, (int)z);
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

void BoxIsland::draw(Vec3f offset)
{
    glPushMatrix();
    //glTranslatef(300.0, 0.0f,300.0);

    Vec3f p=Vec3f(X,Y,Z);
    p = adjustViewLocation(offset, X,Y,Z);

    glTranslatef(p[0],p[1],p[2]);
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
 * Add the specified structure in a desirable "desiredHeight".  Get a random angle and determines the position in polar coordinates.  After that start
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

    float minHeightDifference;
    float minradius;
    float t;

    for(int tries = 0; tries < 10; tries++)
    {
        // @NOTE: This is a random angle in radians.
        // It is used to determine the position in polar coordinates.

        t = (rand() % 360 + 1);

        t = t * PI/180.0f;
        float adjusted = t * 4.0/(2.0*PI);
        int rounded = round(adjusted);
        adjusted = rounded * (2.0*PI)/4.0 + PI/2.0;

        minradius = 2500;
        minHeightDifference = 60;

        for(int radius = 2500;radius>0;radius--)
        {
            x = cos(t);
            z = sin(t);

            x = x * radius;
            z = z * radius;

            x = clipped(x, -1799, 1799);
            z = clipped(z, -1799, 1799);

            assert ( _landmass != NULL || !"Landmass is null !  This is quite weird.");

            // @FIXME Put this line in a different function and use it from there.  Repeated code here.
            heightOffset = +_landmass->getHeight((int)(x/TERRAIN_SCALE)+TERRAIN_SCALE/2,(int)(z/TERRAIN_SCALE)+TERRAIN_SCALE/2);

            float heightDifference = abs(heightOffset - desiredHeight);

            if (  heightDifference<0.5 )
            {
                std::cout << "Best available height found: " << (desiredHeight + heightDifference)
                        << " (Difference: " << heightDifference << ") at (" << x << ", " << z << ")" << std::endl;
                return addStructure(structure,x,z,-t+3*(PI/2), world);
            }

            if ( heightDifference<minHeightDifference )
            {
                minradius = radius;
                minHeightDifference = heightDifference;
            }

        }
    }

    if (minHeightDifference < 60)
    {
        x = cos(t);
        z = sin(t);

        x = x * minradius;
        z = z * minradius;

        if (x>1799) x = 1799;
        if (x<-1799) x = -1799;
        if (z>1799) z = 1799;
        if (z<-1799) z = -1799;

        std::cout << "Not so good available height found: " << (desiredHeight + minHeightDifference)
                        << " (Difference: " << minHeightDifference << ") at (" << x << ", " << z << ")" << std::endl;
        return addStructure(structure,x,z,-t+3*(PI/2), world);
    }

#ifdef DEBUG
    // @NOTE: I am disabling this because the code to handle that situation when no location is found is not easy to do.
    assert( true || !"I did my best but couldn't find a place where to put what you want me to put at the desired place.");
#endif

    // @NOTE Give up.
    std::cout << "Really bad height found: " << (desiredHeight + minHeightDifference)
                        << " (Difference: " << minHeightDifference << ") at (" << x << ", " << z << ")" << std::endl;
    return addStructure(structure,x,z,-t+3*(PI/2), world);
}

Vec3f computeMainDirection(const std::vector<Vec3f>& points) {
    if (points.empty()) return {0,0,0};

    // Step 1: compute centroid
    Vec3f centroid(0,0,0);
    for (const auto& p : points) centroid += p;
    centroid = centroid * (1.0f / points.size());

    // Step 2: compute covariance matrix in XZ plane
    float sum_xx = 0, sum_xz = 0, sum_zz = 0;
    for (const auto& p : points) {
        float dx = p[0] - centroid[0];
        float dz = p[2] - centroid[2];
        sum_xx += dx * dx;
        sum_xz += dx * dz;
        sum_zz += dz * dz;
    }

    // Covariance matrix:
    // [ sum_xx , sum_xz ]
    // [ sum_xz , sum_zz ]

    // Step 3: largest eigenvector (analytical 2x2 solution)
    float trace = sum_xx + sum_zz;
    float det   = sum_xx * sum_zz - sum_xz * sum_xz;
    float term  = std::sqrt((trace * trace) / 4.0f - det);

    float lambda1 = trace / 2.0f + term;
    //float lambda2 = trace / 2.0f - term; // not needed, but is the minor axis

    Vec3f dir;
    if (std::fabs(sum_xz) > 1e-6f) {
        dir = {lambda1 - sum_zz, 0, sum_xz};
    } else {
        dir = (sum_xx >= sum_zz) ? Vec3f{1,0,0} : Vec3f{0,0,1};
    }

    return dir.normalize();
}
/**
 * @brief BoxIsland::addRectangularStructureOnFlatTerrain
 *
 * Adds a rectangular structure, like a runway, to the island terrain. The function
 * searches for a location where the structure's length can be placed with minimal
 * height variation, preventing it from clipping into or floating above the terrain.
 *
 * It iterates through random positions and angles, and for each candidate spot,
 * it checks multiple points along the length of the structure to ensure a
 * relatively flat surface. The final placement height is determined by the average
 * height of the terrain at the chosen location.
 *
 * @param structure The Structure object to be placed. Must have a valid 'length' attribute.
 * @param world The dWorldID for the physics engine.
 * @param searchRadius The maximum radius from the center to search for a location.
 * @param searchTries The number of random starting positions to attempt.
 * @param angleTries The number of angle orientations to check for each position.
 * @return A pointer to the placed Structure if successful, or NULL if no suitable location is found.
 */
Structure* BoxIsland::addRectangularStructureOnFlatTerrain(Structure *structure, dWorldID world, float searchRadius, int searchTries, int angleTries)
{

    assert ( structure != NULL || structure->getLength() > 0 || !"Structure is null or has no length. Weird...." );

    std::vector<int> hist(250, 0);

    std::vector<std::vector<Vec3f>> heights(250);

    for (int x=-1799; x<=1799; x+=TERRAIN_SCALE)
    {
        for (int z=-1799; z<=1799; z+=TERRAIN_SCALE)
        {
            float h = _landmass->getHeight((int)(x/TERRAIN_SCALE)+TERRAIN_SCALE/2,(int)(z/TERRAIN_SCALE)+TERRAIN_SCALE/2);

            if (h>0 && (int)h != 3)
            {
                hist[(int)h]++; 
                heights[(int)h].emplace_back(Vec3f(x, h, z));
            }

        }
    }

    // Good, now find the max height
    // use std function to find a max in an array
    auto count = *std::max_element(hist.begin(), hist.end());

    int height = std::distance(hist.begin(), std::max_element(hist.begin(), hist.end()));

    std::cout << "Found " << count << " of spots with height " << height << std::endl;

    for (const auto& pos : heights[height])
    {
        std::cout << " - Position: " << pos << std::endl;
    }

    // Pick randomly one from the list.
    int pos = getRandomInteger(0, heights[height].size()-1);

    Vec3f selected = heights[height][pos];

    float x = selected[0];
    float z = selected[2];

    float t = 45;  // The structure orientation is positive rotation towards west with 0 aligned to north.

    // The heightmap is vertically mirrored.

    t = ASRADS(getRandomInteger(0,360));

    //int radius = getRandomInteger(0,2500);

    //radius = 100;

    //x = cos(t) * radius;
    //z = sin(t) * radius;

    x = clipped(x, -1799, 1799);
    z = clipped(z, -1799, 1799);

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

/**
 * Attach an existing structure without adding it to entities (from replay and networking).
 *
 * @brief BoxIsland::attachStructure
 * @param structure
 * @param structureId
 * @param x             Real x,y,z parameters
 * @param y
 * @param z
 * @param angle in Radians !
 * @param world
 * @return
 */
Structure* BoxIsland::attachStructure(Structure* structure, size_t structureId, float x, float y, float z, float angle, dWorldID world)
{
    structure->init();
    structure->embody(world,islandspace);
    structure->setPos(x,y,z);
    structure->rotate(angle);
    structure->onIsland(this);

    structures.push_back(structureId);

    if (structure->getType() == CONTROL)
        commandCenterId = structureId;          //@NOTE: commandCenterId is an index for entities !

    return structure;

}

//v->init();
//v->embody(world,space); //islandspace, Island is missing
//v->setPos(Vec3f(record.location.pos1,record.location.pos2, record.location.pos3));
//v->rotate(record.orientation);
//v->onIsland(this);

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

size_t BoxIsland::getIslandId()
{
    return islandId;
}
void BoxIsland::setIslandId(size_t pIslandId)
{
    islandId = pIslandId;
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
    //glColor3f(1.0f, 1.0f, 1.0f);
    
    
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


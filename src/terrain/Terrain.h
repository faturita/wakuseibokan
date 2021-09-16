/*
 * Terrain.h
 *
 *  Created on: Jan 10, 2011
 *      Author: faturita
 */

#ifndef TERRAIN_H_
#define TERRAIN_H_

#include <iostream>
#include <vector>

#include "../odeutils.h"
#include "../openglutils.h"
#include "island.h"
#include "../structures/Structure.h"
#include "../container.h"


#define HFIELD_WSTEP			60			// Vertex count along edge >= 2
#define HFIELD_DSTEP			60


//Represents a terrain, by storing a set of heights and normals at 2D locations
class Terrain {
	private:
		int w; //Width
		int l; //Length
		float** hs; //Heights
		Vec3f** normals;
		bool computedNormals; //Whether normals is up-to-date
	public:
		Terrain(int w2, int l2) {
			w = w2;
			l = l2;

			hs = new float*[l];
			for(int i = 0; i < l; i++) {
				hs[i] = new float[w];
			}

			normals = new Vec3f*[l];
			for(int i = 0; i < l; i++) {
				normals[i] = new Vec3f[w];
			}

			computedNormals = false;
		}

		~Terrain() {
			for(int i = 0; i < l; i++) {
				delete[] hs[i];
			}
			delete[] hs;

			for(int i = 0; i < l; i++) {
				delete[] normals[i];
			}
			delete[] normals;
		}

		int width() {
			return w;
		}

		int length() {
			return l;
		}

		//Sets the height at (x, z) to y
		void setHeight(int x, int z, float y) {
			hs[z][x] = y;
			computedNormals = false;
		}

		//Returns the height at (x, z)
		float getHeight(int x, int z) {
            assert ((z>=0 && z<l && x>=0 && x<w) || !"Terrain has reached out of scope.");
            return hs[z][x];
		}

		//Computes the normals, if they haven't been computed yet
		void computeNormals() {
			if (computedNormals) {
				return;
			}

			//Compute the rough version of the normals
			Vec3f** normals2 = new Vec3f*[l];
			for(int i = 0; i < l; i++) {
				normals2[i] = new Vec3f[w];
			}

			for(int z = 0; z < l; z++) {
				for(int x = 0; x < w; x++) {
					Vec3f sum(0.0f, 0.0f, 0.0f);

					Vec3f out;
					if (z > 0) {
						out = Vec3f(0.0f, hs[z - 1][x] - hs[z][x], -1.0f);
					}
					Vec3f in;
					if (z < l - 1) {
						in = Vec3f(0.0f, hs[z + 1][x] - hs[z][x], 1.0f);
					}
					Vec3f left;
					if (x > 0) {
						left = Vec3f(-1.0f, hs[z][x - 1] - hs[z][x], 0.0f);
					}
					Vec3f right;
					if (x < w - 1) {
						right = Vec3f(1.0f, hs[z][x + 1] - hs[z][x], 0.0f);
					}

					if (x > 0 && z > 0) {
						sum += out.cross(left).normalize();
					}
					if (x > 0 && z < l - 1) {
						sum += left.cross(in).normalize();
					}
					if (x < w - 1 && z < l - 1) {
						sum += in.cross(right).normalize();
					}
					if (x < w - 1 && z > 0) {
						sum += right.cross(out).normalize();
					}

					normals2[z][x] = sum;
				}
			}

			//Smooth out the normals
			const float FALLOUT_RATIO = 0.5f;
			for(int z = 0; z < l; z++) {
				for(int x = 0; x < w; x++) {
					Vec3f sum = normals2[z][x];

					if (x > 0) {
						sum += normals2[z][x - 1] * FALLOUT_RATIO;
					}
					if (x < w - 1) {
						sum += normals2[z][x + 1] * FALLOUT_RATIO;
					}
					if (z > 0) {
						sum += normals2[z - 1][x] * FALLOUT_RATIO;
					}
					if (z < l - 1) {
						sum += normals2[z + 1][x] * FALLOUT_RATIO;
					}

					if (sum.magnitude() == 0) {
						sum = Vec3f(0.0f, 1.0f, 0.0f);
					}
					normals[z][x] = sum;
				}
			}

			for(int i = 0; i < l; i++) {
				delete[] normals2[i];
			}
			delete[] normals2;

			computedNormals = true;
		}

		//Returns the normal at (x, z)
		Vec3f getNormal(int x, int z) {
			if (!computedNormals) {
				computeNormals();
			}
			return normals[z][x];
		}
};

Terrain* loadTerrain(const char* filename, float height);

void drawTerrain(Terrain *_landmass, float fscale,float r,float g,float b);

void drawTerrain(Terrain *_landmass, float fscale);

void drawTerrain(Terrain *_landmass, float fscale,float r,float g,float b);


class BoxIsland : public Island
{
private:
    Terrain *_landmass;
    
    dReal heightfield_callback( void* pUserData, int x, int z );
    
    float X,Y,Z;

    dGeomID islandGeom;
    dSpaceID islandspace;

    std::vector<size_t> structures;
    container<Vehicle*> *entities;

    size_t commandCenterId=0;

    std::string name;

    std::string modelname;
    
public:

    BoxIsland(container<Vehicle*> *entities);
    dGeomID buildTerrainModel(dSpaceID space, const char* model);
    
    //void draw(float x, float y, float z, float side, float height);
    void draw();
    void setLocation(float x, float y, float z);

    dGeomID getGeom();

    Structure* addStructure(Structure* structure, float x, float z,  float angle, dWorldID world);
    Structure* addStructure(Structure* structure,  dWorldID world);
    Structure* addStructureAtDesiredHeight(Structure *structure, dWorldID world, float desiredHeight);

    float getX();
    float getZ();
    Vec3f getPos();
    Vec3f getPosAtDesiredHeight(float desiredHeight);

    std::string getName();
    void setName(std::string name);

    std::string getModelName();
    Structure* getCommandCenter();
    std::vector<size_t> getStructures();
    void checkStructures();
};


#endif /* TERRAIN_H_ */

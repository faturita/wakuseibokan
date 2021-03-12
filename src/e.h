#ifndef ENGINE_H
#define ENGINE_H


#include <vector>
#include <mutex>
#include <unordered_map>

#include "container.h"

#include "odeutils.h"

#include "sounds/sounds.h"

#include "keplerivworld.h"

// Add a new interface to an enbodied object.
#include "units/BoxVehicle.h"
#include "units/Walrus.h"
#include "units/Manta.h"
#include "units/SimplifiedDynamicManta.h"
#include "units/Buggy.h"
#include "units/MultiBodyVehicle.h"
#include "units/Balaenidae.h"
#include "units/Beluga.h"

#include "terrain/Terrain.h"

#include "structures/Structure.h"
#include "structures/Runway.h"
#include "structures/Hangar.h"
#include "structures/Turret.h"
#include "structures/Warehouse.h"
#include "structures/Laserturret.h"
#include "structures/CommandCenter.h"

#include "actions/Gunshot.h"

enum FACTIONS {GREEN_FACTION = 1, BLUE_FACTION = 2};

#define FACTION(m) ( m == GREEN_FACTION ? "Balaenidae" : "Beluga")

// SYNC
Vehicle* gVehicle(dBodyID body);

// SYNC
Vehicle* gVehicle(dGeomID geom);

void gVehicle(Vehicle* &v1, Vehicle* &v2, dBodyID b1, dBodyID b2, Structure* &s1, Structure* &s2, dGeomID g1, dGeomID g2);


// SYNC
bool stranded(Vehicle *carrier, Island *island);


bool departed(Vehicle *walrus);


// SYNC
bool arrived(Vehicle *walrus, Island *island);


// SYNC
bool landed(Vehicle *manta, Island *island);


// SYNC
bool isMineFire(Vehicle* vehicle, Gunshot *g);


// SYNC
bool hit(Vehicle *vehicle, Gunshot *g);


bool hit(Structure* structure);

// SYNC
bool rayHit(Vehicle *vehicle, LaserRay *l);


// SYNC
bool releasecontrol(Vehicle* vehicle);


void  releasecontrol(dBodyID body);


// SYNC
bool  isType(Vehicle *vehicle, int type);

bool isType(dBodyID body, int types[], int length);

bool  isType(dBodyID body, int type);


bool  isManta(dBodyID body);


bool  isManta(Vehicle* vehicle);


bool  isCarrier(dBodyID body);


bool  isCarrier(Vehicle* vehicle);


bool  isWalrus(Vehicle* vehicle);


bool  isAction(dBodyID body);


bool  isAction(Vehicle* vehicle);

bool isRay(dGeomID o);

bool  isRunway(Structure* s);


Island* getIsland(dGeomID candidate);


bool  isIsland(dGeomID candidate);


// SYNC
bool  groundcollisions(Vehicle *vehicle);

void  groundcollisions(dBodyID body);

CommandCenter* findCommandCenter(Island *island);
Manta* findManta(int status);
Walrus* findWalrus(int status, int faction);
Walrus* findWalrus(int faction);
void list();

int findNextNumber(int type);

void buildAndRepair(dSpaceID space, dWorldID world);

Manta* spawnManta(dSpaceID space, dWorldID world,Vehicle *spawner);

Walrus* spawnWalrus(dSpaceID space, dWorldID world, Vehicle *spawner);

void launchManta(Vehicle *v);

BoxIsland* findNearestIsland(Vec3f Po);
BoxIsland* findNearestEmptyIsland(Vec3f Po);

Vehicle* findCarrier(int faction);

void captureIsland(BoxIsland *island, int faction, dSpaceID space, dWorldID world);

void playFaction(int faction, dSpaceID space, dWorldID world);


#endif // ENGINE_H

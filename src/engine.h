#ifndef ENGINE_H
#define ENGINE_H


#include <vector>
#include <mutex>
#include <unordered_map>

#ifdef __linux
#include <functional>
#elif __APPLE__
#endif

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
#include "units/AdvancedManta.h"
#include "units/Cephalopod.h"
#include "units/Medusa.h"
#include "units/Stingray.h"
#include "units/Otter.h"

#include "terrain/Terrain.h"

#include "structures/Structure.h"
#include "structures/Runway.h"
#include "structures/Hangar.h"
#include "structures/Turret.h"
#include "structures/Warehouse.h"
#include "structures/Laserturret.h"
#include "structures/CommandCenter.h"
#include "structures/Artillery.h"
#include "structures/Launcher.h"
#include "structures/Dock.h"
#include "structures/Antenna.h"
#include "structures/Factory.h"
#include "structures/Radar.h"

#include "actions/Gunshot.h"
#include "actions/Missile.h"

#include "actions/Explosion.h"

#include "weapons/CarrierArtillery.h"
#include "weapons/CarrierLauncher.h"
#include "weapons/CarrierTurret.h" 

using TrackRecord = std::tuple<dGeomID, dGeomID, std::function<bool(dGeomID,dGeomID)>>;

enum AIPLAYERSTATUS { FREE_AI, BLUE_AI, GREEN_AI, BOTH_AI};


#define DOCK_RANGE      600
#define COMM_RANGE      50000
#define CLOSE_RANGE     1000

// SYNC
Vehicle* gVehicle(dGeomID geom);

void gVehicle(Vehicle* &v1, Vehicle* &v2,Structure* &s1, Structure* &s2, dGeomID g1, dGeomID g2);

void deleteEntity(size_t i);

// SYNC
bool stranded(Vehicle *carrier, Island *island);

bool departed(dSpaceID space);
bool departed(Vehicle *walrus);


// SYNC
bool arrived(Vehicle *walrus, Island *island);
bool arrived(dSpaceID s, Island *island);


// SYNC
bool landed(Vehicle *manta, Island *island);


// SYNC
bool isMineFire(Vehicle* vehicle, Gunshot *g);


// SYNC
bool hit(Vehicle *vehicle, Gunshot *g);


bool hit(Structure* structure, Gunshot *g);

// SYNC
bool rayHit(Vehicle *vehicle, LaserRay *l);


// SYNC
bool releasecontrol(Vehicle* vehicle);


void  releasecontrol(dGeomID geom);


// SYNC
bool  isType(Vehicle *vehicle, int type);

bool isType(dGeomID geom, int types[], int length);

bool  isType(dGeomID geom, int type);


bool  isManta(dGeomID body);


bool  isManta(Vehicle* vehicle);


bool  isCarrier(dGeomID body);


bool  isCarrier(Vehicle* vehicle);


bool  isWalrus(Vehicle* vehicle);


bool  isAction(dGeomID body);


bool  isAction(Vehicle* vehicle);

bool isRay(dGeomID o);

bool  isRunway(Structure* s);

bool isSubType(dGeomID geom, int subtype);
bool isSubType(Vehicle *v, int subtype);


Island* getIsland(dGeomID candidate);


bool  isIsland(dGeomID candidate);


// SYNC
bool  groundcollisions(Vehicle *vehicle);

void  groundcollisions(dGeomID body);

CommandCenter* findCommandCenter(Island *island);
Manta* findMantaByOrder(int faction, int order);
Manta* findMantaByFactionAndNumber(size_t &pos, int faction, int number);
Manta* findMantaBySubTypeAndFactionAndNumber(size_t &index, VehicleSubTypes subtype, int faction, int number);
Manta* findManta(int faction);
Manta* findManta(int faction, int status);
Manta* findManta(int faction, int status, Vec3f around);
Manta* findNearestManta(int status, int faction, Vec3f l, float threshold = 100000 kmf);
Walrus* findWalrus(int status, int faction);
Walrus* findWalrus(int faction);
Walrus* findWalrus(int status, int faction, int order);
Walrus* findWalrusByOrder(int faction, int order);
Walrus* findWalrusByOrder2(int faction, int order);
Walrus* findWalrusByFactionAndNumber(size_t &pos, int faction, int number);
Walrus* findNearestWalrus(int faction, Vec3f l, float threshold);
void list();

int findNextNumber(int faction, int type, int subtype);

void  buildAndRepair(dSpaceID space, dWorldID world);
void  buildAndRepair(bool force, dSpaceID space, dWorldID world);

void  defendIsland(unsigned long timer,dSpaceID space, dWorldID world);

void commLink(int faction, dSpaceID space, dWorldID world);


Manta* spawnManta(dSpaceID space, dWorldID world,Vehicle *spawner, size_t &idx);

Walrus* spawnWalrus(dSpaceID space, dWorldID world, Vehicle *spawner);
void dockWalrus(Vehicle *dock);
void dockManta();

Manta* spawnCephalopod(dSpaceID space, dWorldID world, Vehicle *spawner);

Manta* launchManta(Vehicle *v);
void landManta(Vehicle *v);
void landManta(Vehicle *landplace, Manta *m);
Manta* taxiManta(Vehicle *v);

BoxIsland* findNearestIsland(Vec3f Po);
BoxIsland* findNearestEmptyIsland(Vec3f Po);
BoxIsland* findIslandByName(std::string islandname);
BoxIsland* findNearestEnemyIsland(Vec3f Po, bool empty);
BoxIsland* findNearestEnemyIsland(Vec3f Po, bool empty, int friendlyfaction);
BoxIsland* findNearestEnemyIsland(Vec3f Po, bool empty, int friendlyfaction, float threshold);

Antenna* findAntennaFromIsland(BoxIsland *is);
Structure* findStructureFromIsland(BoxIsland *is, int subtype);

Vehicle* findNearestEnemyVehicle(int friendlyfaction,int type, Vec3f l, float threshold);
Vehicle* findNearestEnemyVehicle(int friendlyfaction,Vec3f l, float threshold);
Vehicle* findCarrier(int faction);
std::vector<size_t> findNearestEnemyVehicles(int friendlyfaction, int type, Vec3f l, float threshold);

void captureIsland(Vehicle *b, BoxIsland *island, int faction, int typeofisland, dSpaceID space, dWorldID world);
void captureIsland(BoxIsland *island, int faction, int typeofisland, dSpaceID space, dWorldID world);
void wipeEnemyStructures(BoxIsland *island, int faction);

void groundexplosion(Vehicle* v, dWorldID world, dSpaceID space);
void waterexplosion(Vehicle* v, dWorldID world, dSpaceID space);
bool structurecollisions(Structure *s, Vehicle *vehicle);

void trackTargets();

#endif // ENGINE_H

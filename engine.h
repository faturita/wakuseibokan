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

// SYNC
Vehicle* gVehicle(dBodyID body);


void gVehicle(Vehicle* &v1, Vehicle* &v2, dBodyID b1, dBodyID b2, Structure* &s1, Structure*s2, dGeomID g1, dGeomID g2);


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
bool releasecontrol(Vehicle* vehicle);


void inline releasecontrol(dBodyID body);


// SYNC
bool inline isType(Vehicle *vehicle, int type);



bool inline isType(dBodyID body, int type);


bool inline isManta(dBodyID body);


bool inline isManta(Vehicle* vehicle);


bool inline isCarrier(dBodyID body);


bool inline isCarrier(Vehicle* vehicle);


bool inline isWalrus(Vehicle* vehicle);


bool inline isAction(dBodyID body);


bool inline isAction(Vehicle* vehicle);



bool inline isRunway(Structure* s);


Island* getIsland(dGeomID candidate);


bool inline isIsland(dGeomID candidate);


// SYNC
bool inline groundcollisions(Vehicle *vehicle);

void inline groundcollisions(dBodyID body);


#endif // ENGINE_H

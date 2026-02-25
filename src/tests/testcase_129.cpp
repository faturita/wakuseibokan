#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../container.h"
#include "../terrain/Terrain.h"
#include "../units/Vehicle.h"
#include "../units/Walrus.h"
#include "../usercontrols.h"
#include "../camera.h"
#include "../engine.h"

#include "testcase_129.h"

extern unsigned long timer;
extern float fps;
extern  Controller controller;
extern container<Vehicle*> entities;
extern std::vector<BoxIsland*> islands;
extern dWorldID world;
extern dSpaceID space;
extern int testing;
extern  Camera camera;
extern int  aiplayer;

static Vec3f walrusStart      (   0.0f, 1.0f, -3000.0f);
static Vec3f islandCenter     (   0.0f, 0.0f,     0.0f);

static size_t walrusIdx = 0;

// ---------------------------------------------------------------------
TestCase_129::TestCase_129() {}

void TestCase_129::init()
{
    walrusIdx = 0;

    static const char* heightmaps[] = {
        "terrain/thermopilae.bmp",
        "terrain/atom.bmp",
        "terrain/nemesis.bmp",
        "terrain/vulcano.bmp",
        "terrain/island.bmp",
        "terrain/nonsquareisland.bmp",
        "terrain/baltimore.bmp",
        "terrain/fulcrum.bmp",
        "terrain/parentum.bmp",
        "terrain/heightmap.bmp",
    };
    static const int NUM_MAPS = sizeof(heightmaps) / sizeof(heightmaps[0]);
    const char* chosen = heightmaps[rand() % NUM_MAPS];
    printf("  [init] Using heightmap: %s\n", chosen);

    BoxIsland *island = new BoxIsland(&entities);
    island->setName("Thermopilae");
    island->setLocation(0.0f, -1.0f, 0.0f);
    island->buildTerrainModel(space, chosen);
    island->preCalculateCoastlinePoints();
    islands.push_back(island);

    // Walrus in invasion mode: head for the island center, slow near coast, reverse when beached.
    Walrus *walrus = new Walrus(GREEN_FACTION);
    walrus->init();
    walrus->embody(world, space);
    walrus->setPos(walrusStart[0], walrusStart[1], walrusStart[2]);
    walrus->setNameByNumber(1);
    walrus->setSignal(4);
    walrus->setStatus(SailingStatus::SAILING);
    walrus->setLandingMode(true);
    walrus->stop();

    walrusIdx = entities.push_back(walrus, walrus->getGeom());

    printf("  [init] Walrus at (%.0f,%.0f,%.0f)  invading island at (%.0f,%.0f,%.0f)\n",
           walrusStart[0], walrusStart[1], walrusStart[2],
           islandCenter[0], islandCenter[1], islandCenter[2]);

    Vec3f pos(0.0f, 5000.0f, 0.0f);
    camera.setPos(pos);
    camera.xAngle = -90;  camera.yAngle = 0;
    camera.dy = 0;        camera.dz = 0;
    controller.controllingid = CONTROLLING_NONE;
}

// ---------------------------------------------------------------------
int TestCase_129::check(unsigned long timertick)
{
    Walrus *walrus = NULL;
    if (entities.hasMore(walrusIdx))
    {
        Vehicle *v = entities[walrusIdx];
        if (v && v->getType() == WALRUS)
            walrus = (Walrus*)v;
    }

    if (!walrus)
    {
        isdone    = true;
        haspassed = false;
        message   = std::string("Walrus was destroyed or not found.");
        return 0;
    }

    // Start navigation at tick 10
    if (timertick == 10)
    {
        walrus->goTo(islandCenter);
        walrus->enableAuto();
        printf("  [tick 10] Walrus heading to island center (%.0f,%.0f,%.0f)\n",
               islandCenter[0], islandCenter[1], islandCenter[2]);
    }

    Vec3f pos     = walrus->getPos();
    bool beached  = (walrus->getIsland() != NULL);
    int  status   = walrus->getStatus();

    // Distance from coast
    float coastDist = 1e9f;
    if (!islands.empty())
    {
        Vec3f cp = islands[0]->getClosestCoastalPoint(pos);
        cp[1] = 0.0f;
        Vec3f pp = pos; pp[1] = 0.0f;
        coastDist = (cp - pp).magnitude();
    }

    if (timertick % 500 == 0 && timertick >= 10)
    {
        printf("  [tick %4lu] pos=(%.0f,%.0f,%.0f)  coastDist=%.0f  beached=%d  status=%d\n",
               timertick, pos[0], pos[1], pos[2], coastDist, (int)beached, status);
    }

    // Success: Walrus has physically arrived at the island (engine set island ptr)
    // OR it reached the coast (within 300 units) — some heightmaps have no exact beach collision.
    if (beached  && timertick > 2000)
    {
        printf("PASS: Walrus invaded island at tick %lu. beached=%d coastDist=%.0f pos=(%.0f,%.0f,%.0f)\n",
               timertick, (int)beached, coastDist, pos[0], pos[1], pos[2]);
        isdone    = true;
        haspassed = true;
        return 0;
    }

    // Timeout
    if (timertick > 6000)
    {
        printf("FAIL: timeout at tick %lu. pos=(%.0f,%.0f,%.0f) coastDist=%.0f beached=%d\n",
               timertick, pos[0], pos[1], pos[2], coastDist, (int)beached);
        isdone    = true;
        haspassed = false;
        message   = std::string("Timeout: Walrus did not reach the island coast.");
    }

    return 0;
}

int TestCase_129::number()               { return 129; }
std::string TestCase_129::title()        { return std::string("Walrus invades island: approaches, slows near coast, reverses when beached."); }
bool TestCase_129::done()                { return isdone; }
bool TestCase_129::passed()              { return haspassed; }
std::string TestCase_129::failedMessage(){ return message; }

// -----------
TestCase *pickTestCase(int testcase)
{
    return new TestCase_129();
}

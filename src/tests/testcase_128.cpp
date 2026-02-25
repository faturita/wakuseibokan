#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../container.h"

#include "../terrain/Terrain.h"

#include "../units/Vehicle.h"
#include "../units/Seal.h"
#include "../usercontrols.h"
#include "../camera.h"

#include "../engine.h"

#include "testcase_128.h"

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

// Destination on the far side of the island
static Vec3f sealDestination(0.0f, 0.0f, 3000.0f);

TestCase_128::TestCase_128()
{

}

void TestCase_128::init()
{
    // Place the island at the center so it lies between start and destination
    BoxIsland *island = new BoxIsland(&entities);
    island->setName("Thermopilae");
    island->setLocation(0.0f, -1.0, 0.0f);
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

    island->buildTerrainModel(space, chosen);
    island->preCalculateCoastlinePoints();
    islands.push_back(island);

    // Spawn a Seal on the south side of the island
    Walrus *seal = new Walrus(GREEN_FACTION);
    seal->init();
    seal->embody(world, space);
    seal->setPos(0.0f, 1.0f, -3000.0f);
    seal->setNameByNumber(1);
    seal->setStatus(SailingStatus::SAILING);
    seal->stop();
    seal->setSignal(4);

    entities.push_back(seal, seal->getGeom());

    // Top-down camera to observe the whole scene
    Vec3f pos(0.0f, 4000.0f, 0.0f);
    camera.setPos(pos);
    camera.dy = 0;
    camera.dz = 0;
    camera.xAngle = -90;
    camera.yAngle = 0;
    controller.controllingid = CONTROLLING_NONE;
}


int TestCase_128::check(unsigned long timertick)
{
    Walrus *seal = findWalrus(GREEN_FACTION);

    if (!seal)
    {
        isdone = true;
        haspassed = false;
        message = std::string("Could not find Seal unit.");
        return 0;
    }

    // At tick 100, order the Seal to navigate to the opposite side of the island
    if (timertick == 100)
    {
        printf("  Seal starts at (%.0f, %.0f, %.0f), heading to (%.0f, %.0f, %.0f)\n",
               seal->getPos()[0], seal->getPos()[1], seal->getPos()[2],
               sealDestination[0], sealDestination[1], sealDestination[2]);

        seal->goTo(sealDestination);
        seal->enableAuto();
    }

    // Log progress periodically
    if (timertick % 500 == 0 && timertick > 0)
    {
        float dist = (seal->getPos() - sealDestination).magnitude();
        printf("  Tick %lu: Seal pos=(%.0f, %.0f, %.0f) dist_to_dest=%.0f autoStatus=%d\n",
               timertick,
               seal->getPos()[0], seal->getPos()[1], seal->getPos()[2],
               dist,
               (int)seal->getAutoStatus());
    }

    // Check if the Seal reached the destination (within 400 units, matching Walrus roundederror)
    float dist = (seal->getPos() - sealDestination).magnitude();
    if (dist < 400.0f)
    {
        printf("PASS: Seal navigated around island and reached destination at tick %lu (dist=%.0f).\n",
               timertick, dist);
        isdone = true;
        haspassed = true;
        return 0;
    }

    // Timeout
    if (timertick > 5000)
    {
        float finalDist = (seal->getPos() - sealDestination).magnitude();
        printf("FAIL: Seal did not reach destination after %lu ticks. dist=%.0f pos=(%.0f,%.0f,%.0f)\n",
               timertick, finalDist,
               seal->getPos()[0], seal->getPos()[1], seal->getPos()[2]);
        isdone = true;
        haspassed = false;
        message = std::string("Timeout: Seal did not navigate around the island to reach the destination.");
    }

    return 0;
}

int TestCase_128::number()
{
    return 128;
}

std::string TestCase_128::title()
{
    return std::string("Seal navigates around an island using potential fields to reach the opposite side.");
}

bool TestCase_128::done()
{
    return isdone;
}
bool TestCase_128::passed()
{
    return haspassed;
}
std::string TestCase_128::failedMessage()
{
    return message;
}

// -----------
TestCase *pickTestCase(int testcase)
{
    return new TestCase_128();
}

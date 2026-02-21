#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../container.h"

#include "../terrain/Terrain.h"

#include "../units/Vehicle.h"
#include "../usercontrols.h"
#include "../camera.h"

#include "../engine.h"

#include "../structures/CommandCenter.h"
#include "../structures/Dock.h"
#include "../structures/Warehouse.h"

#include "testcase_126.h"

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

TestCase_126::TestCase_126()
{

}

void TestCase_126::init()
{
    // Create a small island using the atom heightmap (Yokatsu-style, small coastline)
    BoxIsland *island = new BoxIsland(&entities);
    island->setName("TestDockSpacing");
    island->setLocation(0.0f, -1.0, 0.0f);
    island->buildTerrainModel(space, "terrain/atom.bmp");
    island->preCalculateCoastlinePoints();
    islands.push_back(island);

    // Add a command center (logistics island)
    island->addStructure(new CommandCenter(GREEN_FACTION, LOGISTICS_ISLAND), 800.0f, -100.0f, 0, world);

    // Camera: top-down view from above the island to visually verify dock placement
    Vec3f pos(0.0f, 1700.5f, -3000);
    camera.setPos(pos);
    camera.dy = 0;
    camera.dz = 0;
    camera.xAngle = 0;
    camera.yAngle = 38;
    controller.controllingid = CONTROLLING_NONE;
}


int TestCase_126::check(unsigned long timertick)
{
    // Place docks at tick 100
    if (timertick == 100)
    {
        BoxIsland *island = islands[0];

        std::vector<Vec3f> dockPositions;

        // Place 3 docks, each time passing existing dock positions so the new one is placed far away
        for (int i = 0; i < 3; i++)
        {
            Structure *dock = new Dock(GREEN_FACTION);
            Structure *result = island->addStructureAtCoast(dock, world, dockPositions);

            Vec3f pos = result->getPos() - island->getPos();
            printf("  Dock %d placed at local pos (%.0f, %.0f, %.0f)\n",
                   i + 1, pos[0], pos[1], pos[2]);
            dockPositions.push_back(pos);
        }

        printf("  Total docks placed: %d\n", (int)dockPositions.size());

        // Verify: all placed docks have reasonable spacing
        for (size_t i = 0; i < dockPositions.size(); i++)
        {
            for (size_t j = i + 1; j < dockPositions.size(); j++)
            {
                float dx = dockPositions[i][0] - dockPositions[j][0];
                float dz = dockPositions[i][2] - dockPositions[j][2];
                float dist = sqrt(dx * dx + dz * dz);
                printf("  Distance between dock %zu and dock %zu: %.0f\n", i + 1, j + 1, dist);

                if (dist < 100.0f)
                {
                    printf("  FAIL: Docks %zu and %zu are overlapping (%.0f)\n", i + 1, j + 1, dist);
                    isdone = true;
                    haspassed = false;
                    message = std::string("Docks placed too close together (overlapping).");
                    return 0;
                }
            }
        }

        printf("Docks placed. Waiting until tick 2100 for visual verification...\n");
    }

    // Wait until tick 2100 so the scene can be visually inspected
    if (timertick >= 2100)
    {
        printf("PASS: 3 docks placed with farthest-point spacing.\n");
        isdone = true;
        haspassed = true;
    }

    return 0;
}

int TestCase_126::number()
{
    return 126;
}

std::string TestCase_126::title()
{
    return std::string("Dock placement: farthest-point selection spreads docks around coastline.");
}

bool TestCase_126::done()
{
    return isdone;
}
bool TestCase_126::passed()
{
    return haspassed;
}
std::string TestCase_126::failedMessage()
{
    return message;
}

// -----------
TestCase *pickTestCase(int testcase)
{
    return new TestCase_126();
}

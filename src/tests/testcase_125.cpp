#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../container.h"

#include "../terrain/Terrain.h"

#include "../units/Vehicle.h"
#include "../units/Balaenidae.h"
#include "../usercontrols.h"
#include "../camera.h"

#include "../engine.h"
#include "../ai.h"

#include "../structures/CommandCenter.h"
#include "../structures/Dock.h"
#include "../structures/WindTurbine.h"
#include "../structures/Warehouse.h"

#include "../weapons/CarrierTurret.h"
#include "../weapons/CarrierArtillery.h"

#include "testcase_125.h"

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

TestCase_125::TestCase_125()
{

}

void TestCase_125::init()
{
    // Island 0: nearby friendly island with a dock (where the carrier should eventually dock)
    BoxIsland *island0 = new BoxIsland(&entities);
    island0->setName("HomeDock");
    island0->setLocation(0.0f, -1.0, 0.0f);
    island0->buildTerrainModel(space, "terrain/thermopilae.bmp");
    island0->preCalculateCoastlinePoints();
    islands.push_back(island0);

    // Friendly structures on island 0
    island0->addStructure(new CommandCenter(GREEN_FACTION, LOGISTICS_ISLAND), 0.0f, 0.0f, 0, world);
    island0->addStructureAtCoast(new Dock(GREEN_FACTION), world);
    island0->addStructure(new Warehouse(GREEN_FACTION), 400.0f, 400.0f, 0, world);

    // Island 1: far-away friendly island (40000 units) — the "next hop" that the strategy will pick
    // The carrier doesn't have enough fuel to reach this island.
    BoxIsland *island1 = new BoxIsland(&entities);
    island1->setName("Waypoint");
    island1->setLocation(40000.0f, -1.0, 0.0f);
    island1->buildTerrainModel(space, "terrain/thermopilae.bmp");
    island1->preCalculateCoastlinePoints();
    islands.push_back(island1);

    // Friendly structures on island 1
    island1->addStructure(new CommandCenter(GREEN_FACTION, LOGISTICS_ISLAND), 0.0f, 0.0f, 0, world);
    island1->addStructureAtCoast(new Dock(GREEN_FACTION), world);

    // Island 2: empty island (80000 units) — the final destination the strategy aims for
    // Having an empty island ensures the strategy has a real target and picks island 1 as next hop.
    BoxIsland *island2 = new BoxIsland(&entities);
    island2->setName("Target");
    island2->setLocation(80000.0f, -1.0, 0.0f);
    island2->buildTerrainModel(space, "terrain/thermopilae.bmp");
    island2->preCalculateCoastlinePoints();
    islands.push_back(island2);
    // No structures on island 2 — it's empty

    // GREEN carrier near island 0, with LOW fuel (not enough to reach island 1)
    // Distance to island 1 = ~40000; fuel needed = 40000*(1000/254000)+50 ~ 207
    // Give it 100 fuel — clearly not enough.
    Balaenidae *_b = new Balaenidae(GREEN_FACTION);
    _b->init();
    dSpaceID carrier_space = _b->embody_in_space(world, space);
    _b->setPos(3000.0f, 20.5f, -8000.0f);   // Near island 0 but not within RANGE[2] (6000)
    _b->stop();
    _b->setPower(100);

    entities.push_back(_b, _b->getGeom());

    // Carrier weapons (required for a proper carrier setup)
    CarrierTurret *_bo = new CarrierTurret(GREEN_FACTION);
    _bo->init();
    _bo->embody(world, carrier_space);
    _bo->attachTo(world, _b, -40.0f, 20.0f + 5, -210.0f);
    _bo->stop();
    entities.push_back(_bo, _bo->getGeom());

    CarrierArtillery *_w1 = new CarrierArtillery(GREEN_FACTION);
    _w1->init();
    _w1->embody(world, carrier_space);
    _w1->attachTo(world, _b, -40.0, 27.0f, +210.0f);
    _w1->stop();
    entities.push_back(_w1, _w1->getGeom());

    // Camera
    Vec3f pos(3000, 300, -8000);
    camera.setPos(pos);
    camera.dy = 0;
    camera.dz = 0;
    camera.xAngle = -90;
    camera.yAngle = 0;
    controller.controllingid = CONTROLLING_NONE;

    // Enable GREEN AI
    aiplayer = GREEN_AI;
    controller.faction = BOTH_FACTION;
}


int TestCase_125::check(unsigned long timertick)
{
    Vehicle *carrier = findCarrier(GREEN_FACTION);

    if (!carrier)
    {
        printf("FAIL: Carrier not found!\n");
        isdone = true;
        haspassed = false;
        message = std::string("Carrier disappeared.");
        return 0;
    }

    // Log state every 500 ticks
    if (timertick % 500 == 0)
    {
        printf("  Tick %lu: Carrier power=%.0f autoStatus=%d sailStatus=%d pos=(%.0f,%.0f,%.0f)\n",
               timertick,
               (float)carrier->getPower(),
               (int)carrier->getAutoStatus(),
               (int)carrier->getStatus(),
               carrier->getPos()[0], carrier->getPos()[1], carrier->getPos()[2]);
    }

    // After some time, check that the carrier transitioned to DOCKING (autoStatus == DOCKING)
    // rather than getting stuck in APPROACHFRIENDLYISLAND forever.
    // The AI needs a few ticks for playStrategy to run and set the nextIsland,
    // then transitions fire.
    if (timertick > 2000 && timertick % 100 == 0)
    {
        // Success condition: carrier is docking or has docked at island 0
        if (carrier->getAutoStatus() == AutoStatus::DOCKING)
        {
            printf("PASS: Carrier transitioned to DOCKING at tick %lu (power=%.0f).\n",
                   timertick, (float)carrier->getPower());
            isdone = true;
            haspassed = true;
            return 0;
        }

        if (carrier->getStatus() == SailingStatus::DOCKED)
        {
            printf("PASS: Carrier DOCKED at tick %lu (power=%.0f).\n",
                   timertick, (float)carrier->getPower());
            isdone = true;
            haspassed = true;
            return 0;
        }
    }

    // Timeout
    if (timertick > 20000)
    {
        printf("FAIL: Timeout at tick %lu. Carrier stuck: autoStatus=%d sailStatus=%d power=%.0f\n",
               timertick,
               (int)carrier->getAutoStatus(),
               (int)carrier->getStatus(),
               (float)carrier->getPower());
        isdone = true;
        haspassed = false;
        message = std::string("Timeout: Carrier did not transition to DOCKING. Likely stuck in APPROACHFRIENDLYISLAND.");
    }

    return 0;
}

int TestCase_125::number()
{
    return 125;
}

std::string TestCase_125::title()
{
    return std::string("AI: Carrier with low fuel escapes APPROACHFRIENDLYISLAND and docks.");
}

bool TestCase_125::done()
{
    return isdone;
}
bool TestCase_125::passed()
{
    return haspassed;
}
std::string TestCase_125::failedMessage()
{
    return message;
}

// -----------
TestCase *pickTestCase(int testcase)
{
    return new TestCase_125();
}

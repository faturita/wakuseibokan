#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../container.h"

#include "../terrain/Terrain.h"

#include "../units/Vehicle.h"
#include "../units/CargoShip.h"
#include "../usercontrols.h"
#include "../camera.h"

#include "../engine.h"

#include "../structures/CommandCenter.h"
#include "../structures/Dock.h"
#include "../structures/Warehouse.h"

#include "testcase_127.h"

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

TestCase_127::TestCase_127()
{

}

void TestCase_127::init()
{
    BoxIsland *island = new BoxIsland(&entities);
    island->setName("RefillTestIsland");
    island->setLocation(0.0f, -1.0, 0.0f);
    island->buildTerrainModel(space, "terrain/thermopilae.bmp");
    island->preCalculateCoastlinePoints();
    islands.push_back(island);

    // Command center
    island->addStructure(new CommandCenter(GREEN_FACTION, LOGISTICS_ISLAND), 800.0f, -100.0f, 0, world);

    // A dock at the coast (starts with 0 cargo)
    Structure *dock = island->addStructureAtCoast(new Dock(GREEN_FACTION), world);

    // Spawn a CargoShip away from the island, sailing idle, loaded with 500 fuel
    CargoShip *cargo = new CargoShip(GREEN_FACTION);
    cargo->init();
    cargo->setNameByNumber(1);
    cargo->embody(world, space);
    // Position the cargoship 5000 units away along the dock's forward direction
    Vec3f dockFwd = dock->getForward().normalize();
    Vec3f startPos = dock->getPos() + dockFwd * 5000.0f;
    cargo->setPos(startPos[0], 1.0f, startPos[2]);
    cargo->stop();
    cargo->setStatus(SailingStatus::SAILING);
    cargo->setCargo(CargoTypes::POWERFUEL, 500);

    entities.push_back(cargo, cargo->getGeom());

    // Camera: top-down view to see the island and the approaching cargoship
    Vec3f pos(0.0f, 1700.5f, -3000);
    camera.setPos(pos);
    camera.dy = 0;
    camera.dz = 0;
    camera.xAngle = 0;
    camera.yAngle = 38;
    controller.controllingid = CONTROLLING_NONE;
}


int TestCase_127::check(unsigned long timertick)
{
    Vehicle *cargoship = NULL;
    Vehicle *dock = NULL;

    // Find the CargoShip and Dock each tick
    for (size_t i = entities.first(); entities.hasMore(i); i = entities.next(i))
    {
        if (entities[i]->getSubType() == VehicleSubTypes::CARGOSHIP)
            cargoship = entities[i];
    }

    BoxIsland *island = islands[0];
    std::vector<size_t> strs = island->getStructures();
    for (size_t i = 0; i < strs.size(); i++)
    {
        if (entities[strs[i]]->getSubType() == VehicleSubTypes::DOCK)
        {
            dock = entities[strs[i]];
            break;
        }
    }

    if (!dock || !cargoship)
    {
        isdone = true;
        haspassed = false;
        message = std::string("Could not find dock or cargoship.");
        return 0;
    }

    // At tick 100, order the CargoShip to dock on the island
    if (timertick == 100)
    {
        printf("  Ordering CargoShip to dock. CargoShip fuel=%d, Dock fuel=%d\n",
               cargoship->getCargo(CargoTypes::POWERFUEL), dock->getCargo(CargoTypes::POWERFUEL));

        cargoship->setDestination(dock->getPos() - dock->getForward().normalize() * 400);
        cargoship->setAutoStatus(AutoStatus::DOCKING);
        cargoship->enableAuto();
    }

    // Log status periodically
    if (timertick % 500 == 0)
    {
        printf("  Tick %lu: CargoShip status=%d autoStatus=%d fuel=%d dist=%.0f\n",
               timertick,
               (int)cargoship->getStatus(),
               (int)cargoship->getAutoStatus(),
               cargoship->getCargo(CargoTypes::POWERFUEL),
               (cargoship->getPos() - dock->getPos()).magnitude());
    }

    // Once the CargoShip has docked, call refill and verify
    if (cargoship->getStatus() == SailingStatus::DOCKED && !haspassed)
    {
        int cargoBefore = cargoship->getCargo(CargoTypes::POWERFUEL);
        int dockBefore = dock->getCargo(CargoTypes::POWERFUEL);

        printf("  CargoShip DOCKED at tick %lu. Before refill: CargoShip fuel=%d, Dock fuel=%d\n",
               timertick, cargoBefore, dockBefore);

        refill(cargoship);

        int cargoAfter = cargoship->getCargo(CargoTypes::POWERFUEL);
        int dockAfter = dock->getCargo(CargoTypes::POWERFUEL);

        printf("  After refill: CargoShip fuel=%d, Dock fuel=%d\n", cargoAfter, dockAfter);

        if (cargoAfter >= cargoBefore)
        {
            printf("  FAIL: CargoShip fuel did not decrease (before=%d, after=%d)\n", cargoBefore, cargoAfter);
            isdone = true;
            haspassed = false;
            message = std::string("CargoShip fuel was not transferred to Dock after refill.");
            return 0;
        }

        if (dockAfter <= dockBefore)
        {
            printf("  FAIL: Dock fuel did not increase (before=%d, after=%d)\n", dockBefore, dockAfter);
            isdone = true;
            haspassed = false;
            message = std::string("Dock did not receive fuel from CargoShip after refill.");
            return 0;
        }

        printf("  CargoShip transferred %d fuel to Dock.\n", dockAfter - dockBefore);
        haspassed = true;
    }

    // After passing, wait 2000 extra ticks for visual verification
    if (haspassed && timertick >= 2000)
    {
        printf("PASS: CargoShip sailed to dock and transferred fuel.\n");
        isdone = true;
    }

    // Timeout
    if (timertick > 20000)
    {
        printf("FAIL: Timeout. CargoShip never docked. status=%d dist=%.0f\n",
               (int)cargoship->getStatus(),
               (cargoship->getPos() - dock->getPos()).magnitude());
        isdone = true;
        haspassed = false;
        message = std::string("Timeout: CargoShip did not dock on island.");
    }

    return 0;
}

int TestCase_127::number()
{
    return 127;
}

std::string TestCase_127::title()
{
    return std::string("CargoShip sails to island dock and transfers fuel via refill.");
}

bool TestCase_127::done()
{
    return isdone;
}
bool TestCase_127::passed()
{
    return haspassed;
}
std::string TestCase_127::failedMessage()
{
    return message;
}

// -----------
TestCase *pickTestCase(int testcase)
{
    return new TestCase_127();
}

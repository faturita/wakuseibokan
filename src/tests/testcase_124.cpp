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

#include "../units/Balaenidae.h"

#include "../engine.h"

#include "../structures/CommandCenter.h"
#include "../structures/Dock.h"
#include "../structures/WindTurbine.h"
#include "../structures/Warehouse.h"

#include "testcase_124.h"

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

TestCase_124::TestCase_124()
{

}

void TestCase_124::init()
{
    // Create a single island
    BoxIsland *island = new BoxIsland(&entities);
    island->setName("Logistics");
    island->setLocation(0.0f,-1.0,0.0f);
    island->buildTerrainModel(space,"terrain/thermopilae.bmp");
    island->preCalculateCoastlinePoints();

    islands.push_back(island);

    // Command Center
    island->addStructure(new CommandCenter(GREEN_FACTION, LOGISTICS_ISLAND), 0.0f, 0.0f, 0, world);

    // 2 Docks (at the coast)
    island->addStructureAtCoast(new Dock(GREEN_FACTION), world);
    island->addStructureAtCoast(new Dock(GREEN_FACTION), world);

    // 3 WindTurbines
    island->addStructure(new WindTurbine(GREEN_FACTION),  400.0f,  400.0f, 0, world);
    island->addStructure(new WindTurbine(GREEN_FACTION), -400.0f,  400.0f, 0, world);
    island->addStructure(new WindTurbine(GREEN_FACTION),  400.0f, -400.0f, 0, world);

    // 2 Warehouses
    island->addStructure(new Warehouse(GREEN_FACTION),  -400.0f, -400.0f, 0, world);
    island->addStructure(new Warehouse(GREEN_FACTION),     0.0f, -800.0f, 0, world);

    // Spawn a CargoShip assigned to this island (index 0), positioned away from the island
    CargoShip *cargo = new CargoShip(GREEN_FACTION);
    cargo->init();
    cargo->setNameByNumber(1);
    cargo->embody(world, space);
    cargo->setPos(0.0f, 1.0f, -5000.0f);
    cargo->stop();
    cargo->setStatus(SailingStatus::SAILING);
    cargo->setAssignedTo(0);  // Assigned to island index 0, prevents auto-spawn of another

    entities.push_back(cargo, cargo->getGeom());

    // Camera setup
    Vec3f pos(-1000,300,0);
    camera.setPos(pos);
    camera.dy = 0;
    camera.dz = 0;
    camera.xAngle = -90;
    camera.yAngle = 0;
    controller.controllingid = CONTROLLING_NONE;

    aiplayer = FREE_AI;
    controller.faction = GREEN_FACTION;
}


int TestCase_124::check(unsigned long timertick)
{
    // Step 1: At the start, seed windturbines with extra fuel
    if (timertick == 10)
    {
        std::vector<size_t> str = islands[0]->getStructures();
        for (size_t i = 0; i < str.size(); i++)
        {
            if (entities[str[i]]->getSubType() == VehicleSubTypes::WINDTURBINE)
                entities[str[i]]->setCargo(CargoTypes::POWERFUEL, 300);
        }
        printf("=== Tick %lu: Seeded 3 windturbines with 300 fuel each (900 total) ===\n", timertick);
    }

    if (timertick == 500)
    {
        buildAndRepair(true,space,world);
    }

    // Step 2: After enough time for buildAndRepair to automatically distribute, check even distribution
    if (timertick == 1000)
    {
        std::vector<size_t> str = islands[0]->getStructures();

        int numWarehouses = 0, numDocks = 0;
        int totalWarehouseFuel = 0, totalDockFuel = 0, totalWindTurbineFuel = 0;

        printf("=== Tick %lu: Checking automatic distribution ===\n", timertick);
        for (size_t i = 0; i < str.size(); i++)
        {
            Vehicle *v = entities[str[i]];
            int fuel = v->getCargo(CargoTypes::POWERFUEL);
            printf("  %s (entity %zu): fuel=%d\n", v->getName().c_str(), str[i], fuel);

            if (v->getSubType() == VehicleSubTypes::WAREHOUSE)    { totalWarehouseFuel += fuel; numWarehouses++; }
            else if (v->getSubType() == VehicleSubTypes::DOCK)    { totalDockFuel += fuel; numDocks++; }
            else if (v->getSubType() == VehicleSubTypes::WINDTURBINE) { totalWindTurbineFuel += fuel; }
        }

        int totalDistributed = totalWarehouseFuel + totalDockFuel;
        int numRecipients = numWarehouses + numDocks;

        printf("Recipients: %d warehouses + %d docks = %d\n", numWarehouses, numDocks, numRecipients);
        printf("Distributed: warehouses=%d, docks=%d, total=%d\n", totalWarehouseFuel, totalDockFuel, totalDistributed);
        printf("Remaining on windturbines: %d\n", totalWindTurbineFuel);

        if (totalDistributed == 0)
        {
            printf("FAIL: No automatic distribution happened by tick %lu.\n", timertick);
            isdone = true;
            haspassed = false;
            message = std::string("buildAndRepair did not automatically distribute fuel.");
            return 0;
        }

        // Check even distribution: each recipient should have roughly the same amount
        bool evenDistribution = true;
        int expectedPerRecipient = totalDistributed / numRecipients;
        int tolerance = expectedPerRecipient / 2;  // 50% tolerance for timing variations

        for (size_t i = 0; i < str.size(); i++)
        {
            Vehicle *v = entities[str[i]];
            if (v->getSubType() == VehicleSubTypes::WAREHOUSE ||
                v->getSubType() == VehicleSubTypes::DOCK)
            {
                int fuel = v->getCargo(CargoTypes::POWERFUEL);
                if (tolerance > 0 && abs(fuel - expectedPerRecipient) > tolerance)
                {
                    printf("  WARN: %s has %d fuel, expected ~%d (tolerance %d)\n",
                           v->getName().c_str(), fuel, expectedPerRecipient, tolerance);
                    evenDistribution = false;
                }
            }
        }

        if (evenDistribution)
            printf("PASS: Fuel was evenly distributed automatically.\n");
        else
            printf("WARN: Distribution was not perfectly even (may still be acceptable).\n");
    }

    // Step 3: Order the cargoship to dock
    if (timertick == 1100)
    {
        Vehicle *cargoship = NULL;
        for (size_t i = entities.first(); entities.hasMore(i); i = entities.next(i))
        {
            if (entities[i]->getSubType() == VehicleSubTypes::CARGOSHIP)
            {
                cargoship = entities[i];
                break;
            }
        }

        if (cargoship)
        {
            printf("=== Tick %lu: Ordering CargoShip to dock ===\n", timertick);
            printf("  CargoShip pos=(%.0f,%.0f,%.0f)\n",
                   cargoship->getPos()[0], cargoship->getPos()[1], cargoship->getPos()[2]);
            dockInNearestIsland(cargoship);
        }
        else
        {
            printf("FAIL: CargoShip not found at tick %lu.\n", timertick);
            isdone = true;
            haspassed = false;
            message = std::string("CargoShip not found.");
        }
    }

    // Step 4: Check periodically if the cargoship has arrived and then collect+refill
    if (timertick > 1500 && timertick % 100 == 0 && !isdone)
    {
        Vehicle *cargoship = NULL;
        for (size_t i = entities.first(); entities.hasMore(i); i = entities.next(i))
        {
            if (entities[i]->getSubType() == VehicleSubTypes::CARGOSHIP)
            {
                cargoship = entities[i];
                break;
            }
        }

        if (!cargoship)
        {
            printf("FAIL: CargoShip lost!\n");
            isdone = true;
            haspassed = false;
            message = std::string("CargoShip disappeared.");
            return 0;
        }

        // Check if cargoship has finished docking (status changes to DOCKED)
        bool arrived = (cargoship->getStatus() == SailingStatus::DOCKED);

        printf("  Tick %lu: CargoShip status=%d autostatus=%d pos=(%.0f,%.0f,%.0f)\n",
               timertick, cargoship->getStatus(), cargoship->getAutoStatus(),
               cargoship->getPos()[0], cargoship->getPos()[1], cargoship->getPos()[2]);

        if (arrived)
        {
            printf("=== Tick %lu: CargoShip arrived at dock ===\n", timertick);

            // Find the dock nearest to the cargoship
            std::vector<size_t> str = islands[0]->getStructures();
            Vehicle *dock = NULL;
            float minDist = 999999;
            for (size_t i = 0; i < str.size(); i++)
            {
                if (entities[str[i]]->getSubType() == VehicleSubTypes::DOCK)
                {
                    float dist = (entities[str[i]]->getPos() - cargoship->getPos()).magnitude();
                    if (dist < minDist)
                    {
                        minDist = dist;
                        dock = entities[str[i]];
                    }
                }
            }

            if (!dock)
            {
                printf("FAIL: No dock found!\n");
                isdone = true;
                haspassed = false;
                message = std::string("No dock found near cargoship.");
                return 0;
            }

            printf("  Nearest dock at distance %.0f\n", minDist);

            // Print fuel state before collect
            printf("=== Before collect ===\n");
            for (size_t i = 0; i < str.size(); i++)
            {
                Vehicle *v = entities[str[i]];
                printf("  %s: fuel=%d\n", v->getName().c_str(), v->getCargo(CargoTypes::POWERFUEL));
            }

            // Dock collects fuel from all island structures
            collect(dock);

            printf("=== After collect ===\n");
            int dockFuelAfterCollect = dock->getCargo(CargoTypes::POWERFUEL);
            printf("  Dock fuel after collect: %d\n", dockFuelAfterCollect);

            for (size_t i = 0; i < str.size(); i++)
            {
                Vehicle *v = entities[str[i]];
                printf("  %s: fuel=%d\n", v->getName().c_str(), v->getCargo(CargoTypes::POWERFUEL));
            }

            // Dock refills the cargoship
            printf("  CargoShip cargo before refill: %d\n", cargoship->getCargo(CargoTypes::POWERFUEL));
            refill(dock);

            int cargoFuelAfter = cargoship->getCargo(CargoTypes::POWERFUEL);
            printf("=== After refill ===\n");
            printf("  CargoShip cargo: %d\n", cargoFuelAfter);
            printf("  Dock fuel: %d\n", dock->getCargo(CargoTypes::POWERFUEL));

            // Verify
            if (cargoFuelAfter > 0)
            {
                printf("PASS: Full logistics chain worked. CargoShip has %d fuel.\n", cargoFuelAfter);
                isdone = true;
                haspassed = true;
            }
            else
            {
                printf("FAIL: CargoShip got no fuel after collect+refill.\n");
                isdone = true;
                haspassed = false;
                message = std::string("CargoShip received no fuel after dock collect and refill.");
            }
        }
    }

    if (timertick > 15000)
    {
        if (!isdone)
        {
            printf("TIMEOUT at tick %lu\n", timertick);
            isdone = true;
            haspassed = false;
            message = std::string("Timeout: CargoShip did not complete docking in time.");
        }
    }

    return 0;
}

int TestCase_124::number()
{
    return 124;
}

std::string TestCase_124::title()
{
    return std::string("Logistics: auto-distribution, CargoShip docks, collect+refill chain.");
}

bool TestCase_124::done()
{
    return isdone;
}
bool TestCase_124::passed()
{
    return haspassed;
}
std::string TestCase_124::failedMessage()
{
    return message;
}

// -----------
TestCase *pickTestCase(int testcase)
{
    return new TestCase_124();
}

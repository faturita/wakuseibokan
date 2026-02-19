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

#include "../units/Balaenidae.h"

#include "../engine.h"

#include "../structures/CommandCenter.h"
#include "../structures/Dock.h"
#include "../structures/WindTurbine.h"
#include "../structures/Warehouse.h"

#include "testcase_123.h"

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

TestCase_123::TestCase_123()
{

}

void TestCase_123::init()
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


int TestCase_123::check(unsigned long timertick)
{
    bool fuelWasDistributed = false;
    bool windturbinesDrained = false;
    bool evenDistribution = false;

    int numWarehouses = 0, numDocks = 0;

    // Step 1: Seed windturbines with fuel and force distribution
    if (timertick == 100)
    {
        std::vector<size_t> str = islands[0]->getStructures();

        // Give each windturbine 300 fuel (total 900 across 3 turbines)
        for (size_t i = 0; i < str.size(); i++)
        {
            if (entities[str[i]]->getSubType() == VehicleSubTypes::WINDTURBINE)
            {
                entities[str[i]]->setCargo(CargoTypes::POWERFUEL, 300);
            }
        }

        printf("=== Before buildAndRepair ===\n");
        for (size_t i = 0; i < str.size(); i++)
        {
            Vehicle *v = entities[str[i]];
            printf("  %s (entity %zu): fuel=%d\n", v->getName().c_str(), str[i],
                   v->getCargo(CargoTypes::POWERFUEL));
        }

        // Force buildAndRepair to trigger the logistics distribution
        buildAndRepair(true, space, world);

        printf("=== After buildAndRepair ===\n");
        int totalWarehouseFuel = 0, totalDockFuel = 0, totalWindTurbineFuel = 0;

        for (size_t i = 0; i < str.size(); i++)
        {
            Vehicle *v = entities[str[i]];
            int fuel = v->getCargo(CargoTypes::POWERFUEL);
            printf("  %s (entity %zu): fuel=%d\n", v->getName().c_str(), str[i], fuel);

            if (v->getSubType() == VehicleSubTypes::WAREHOUSE)
            {
                totalWarehouseFuel += fuel;
                numWarehouses++;
            }
            else if (v->getSubType() == VehicleSubTypes::DOCK)
            {
                totalDockFuel += fuel;
                numDocks++;
            }
            else if (v->getSubType() == VehicleSubTypes::WINDTURBINE)
            {
                totalWindTurbineFuel += fuel;
            }
        }

        int totalDistributed = totalWarehouseFuel + totalDockFuel;
        int numRecipients = numWarehouses + numDocks;  // 2 warehouses + 2 docks = 4

        printf("=== Results ===\n");
        printf("Recipients: %d warehouses + %d docks = %d total\n", numWarehouses, numDocks, numRecipients);
        printf("Fuel distributed: warehouses=%d, docks=%d, total=%d\n",
               totalWarehouseFuel, totalDockFuel, totalDistributed);
        printf("Fuel remaining on windturbines: %d\n", totalWindTurbineFuel);

        // Verify: fuel was distributed from windturbines to warehouses and docks
        fuelWasDistributed = (totalDistributed > 0);
        windturbinesDrained = (totalWindTurbineFuel == 0);

        // Verify: distribution is even — each recipient should get roughly the same amount
        // With 900 fuel / 4 recipients = 225 each
        evenDistribution = true;
        int expectedPerRecipient = 900 / numRecipients;
        int tolerance = expectedPerRecipient / 3;  // Allow ~33% tolerance

        for (size_t i = 0; i < str.size(); i++)
        {
            Vehicle *v = entities[str[i]];
            if (v->getSubType() == VehicleSubTypes::WAREHOUSE ||
                v->getSubType() == VehicleSubTypes::DOCK)
            {
                int fuel = v->getCargo(CargoTypes::POWERFUEL);
                if (abs(fuel - expectedPerRecipient) > tolerance)
                {
                    printf("FAIL: %s has %d fuel, expected ~%d (tolerance %d)\n",
                           v->getName().c_str(), fuel, expectedPerRecipient, tolerance);
                    evenDistribution = false;
                }
            }
        }
    }

    if (timertick == 1000)
    {
        if (fuelWasDistributed && windturbinesDrained && evenDistribution)
        {
            printf("PASS: Fuel evenly distributed to %d warehouses and %d docks.\n",
                   numWarehouses, numDocks);
            isdone = true;
            haspassed = true;
        }
        else
        {
            printf("FAIL: distributed=%d, drained=%d, even=%d\n",
                   fuelWasDistributed, windturbinesDrained, evenDistribution);
            isdone = true;
            haspassed = false;
            message = std::string("Fuel was not evenly distributed to warehouses and docks.");
        }
    }

    if (timertick > 1500)
    {
        if (!isdone)
        {
            isdone = true;
            haspassed = false;
            message = std::string("Timeout: test did not complete.");
        }
    }

    return 0;
}

int TestCase_123::number()
{
    return 123;
}

std::string TestCase_123::title()
{
    return std::string("Logistics: WindTurbines produce fuel, Warehouses store it, Docks collect it.");
}

bool TestCase_123::done()
{
    return isdone;
}
bool TestCase_123::passed()
{
    return haspassed;
}
std::string TestCase_123::failedMessage()
{
    return message;
}

// -----------
TestCase *pickTestCase(int testcase)
{
    return new TestCase_123();
}

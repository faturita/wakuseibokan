#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <vector>

#include "../container.h"

#include "../terrain/Terrain.h"

#include "../units/Vehicle.h"
#include "../usercontrols.h"
#include "../camera.h"

#include "../units/Balaenidae.h"
#include "../units/Beluga.h"

#include "../units/Otter.h"

#include "../units/Wheel.h"
#include "../units/BoxVehicle.h"
#include "../actions/ArtilleryAmmo.h"

#include "../engine.h"

#include "../weapons/CarrierArtillery.h"
#include "../weapons/CarrierTurret.h"
#include "../weapons/CarrierLauncher.h"


#include "testcase_121.h"


extern unsigned long timer;

extern  Controller controller;
extern container<Vehicle*> entities;
extern std::vector<BoxIsland*> islands;
extern dWorldID world;
extern dSpaceID space;
extern int testing;
extern  Camera Camera;
extern int  aiplayer;

TestCase_121::TestCase_121()
{

}

void TestCase_121::init()
{
    BoxIsland *nemesis = new BoxIsland(&entities);
    nemesis->setName("Atom");
    nemesis->setLocation(0.0f,-1.0,0.0f);

    std::vector<std::string> islandshape;

    islandshape.push_back("terrain/sentinel.bmp");
    islandshape.push_back("terrain/atom.bmp");
    islandshape.push_back("terrain/baltimore.bmp");
    islandshape.push_back("terrain/fulcrum.bmp");
    islandshape.push_back("terrain/gaijin.bmp");
    islandshape.push_back("terrain/goku.bmp");
    islandshape.push_back("terrain/nemesis.bmp");
    islandshape.push_back("terrain/nonsquareisland.bmp");
    islandshape.push_back("terrain/parentum.bmp");
    islandshape.push_back("terrain/thermopilae.bmp");
    islandshape.push_back("terrain/tristan.bmp");
    islandshape.push_back("terrain/vulcano.bmp");
    islandshape.push_back("terrain/vulcrum.bmp");

    std::string shape = islandshape[getRandomInteger(0,islandshape.size()-1)];

    nemesis->buildTerrainModel(space,shape.c_str());

    islands.push_back(nemesis);


    Structure *t1 = islands[0]->addStructure(new CommandCenter(GREEN_FACTION, FACTORY_ISLAND)    ,       200.0f,    -100.0f,0,world);
    Structure *t2 = islands[0]->addStructureAtDesiredHeight(new Dock(GREEN_FACTION), world, 0);
    Structure *t3 = islands[0]->addStructureAtDesiredHeight(new Dock(GREEN_FACTION), world, 0);
    Structure *t4 = islands[0]->addStructureAtDesiredHeight(new Dock(GREEN_FACTION), world, 0);
    Structure *t5 = islands[0]->addStructureAtDesiredHeight(new Dock(GREEN_FACTION), world, 2);
    Structure *t6 = islands[0]->addStructureAtDesiredHeight(new Dock(GREEN_FACTION), world, 2);
    Structure *t7 = islands[0]->addStructureAtDesiredHeight(new Dock(GREEN_FACTION), world, 2);
    Structure *t8 = islands[0]->addStructureAtDesiredHeight(new Dock(GREEN_FACTION), world, 2);
    Structure *t9 = islands[0]->addStructureAtDesiredHeight(new Dock(GREEN_FACTION), world, 2);

    //Vec3f pos(0.0,1.32, - 3500);
    Vec3f pos(-10,1.32,10);
    Camera.setPos(pos);

    aiplayer = BOTH_AI;
    controller.faction = BOTH_FACTION;

}

int TestCase_121::check(unsigned long timertick)
{

    if (timertick == 1000)
    {
        haspassed = true;
        std::vector<size_t> structures = islands[0]->getStructures();
        for(size_t i=0;i<structures.size();i++)
        {
            if ((entities[structures[i]]->getPos()[1]-2)>2)
            {
                if (entities[structures[i]]->getTypeId() == EntityTypeId::TDock)
                {
                    isdone = true;
                    haspassed = false;
                    message = std::string("The location of the structure is not the desired location");
                    dout << entities[structures[i]]->getPos() << std::endl;
                }
            }
        }
        isdone = true;
    }


    if (timertick > 15000)
    {
        isdone = true;
        haspassed = false;
        message = std::string("The timeout has occurred and nothing happened.");
    }

    return 0;
}

int TestCase_121::number()
{
    return 121;

}

std::string TestCase_121::title()
{
    return std::string("Check Docks location on island");
}


bool TestCase_121::done()
{
    return isdone;
}
bool TestCase_121::passed()
{
    return haspassed;
}
std::string TestCase_121::failedMessage()
{
    return message;
}


// -----------
TestCase *pickTestCase(int testcase)
{
    return new TestCase_121();
}


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
#include "../units/Beluga.h"

#include "../units/Otter.h"

#include "../units/Wheel.h"

#include "../engine.h"

#include "testcase_105.h"


extern unsigned long timer;

extern  Controller controller;
extern container<Vehicle*> entities;
extern std::vector<BoxIsland*> islands;
extern dWorldID world;
extern dSpaceID space;
extern int testing;
extern  Camera Camera;
extern int  aiplayer;

TestCase_105::TestCase_105()
{

}

void TestCase_105::init()
{
    BoxIsland *nemesis = new BoxIsland(&entities);
    nemesis->setName("Atom");
    nemesis->setLocation(0.0f,-1.0,0.0f);
    nemesis->buildTerrainModel(space,"terrain/fulcrum.bmp");

    islands.push_back(nemesis);

    // Entities will be added later in time.
    Balaenidae *_b = new Balaenidae(GREEN_FACTION);
    _b->init();
    _b->embody(world,space);
    _b->setPos(0.0f,20.5f,-4000.0f);
    _b->stop();

    entities.push_back(_b, _b->getGeom());

    Vec3f pos(0.0,1.32, - 3500);
    Camera.setPos(pos);

    aiplayer = BOTH_AI;
    controller.faction = BOTH_FACTION;

}

int TestCase_105::check(unsigned long timertick)
{

    if (timertick > 4000)
    {
        Walrus* _w = findWalrus(BLUE_FACTION);

        isdone = true;
        haspassed = true;

        if (_w)
        {
            isdone = false;
            haspassed = false;
        }

        BoxIsland *is = islands[0];

        if (is->getCommandCenter() == NULL)
        {
            isdone = false;
            haspassed = false;
        }
    }


    if (timertick > 15000)
    {
        isdone = true;
        haspassed = false;
        message = std::string("The timeout has occurred and nothing happened.");
    }

    return 0;
}

int TestCase_105::number()
{
    return 105;

}

std::string TestCase_105::title()
{
    return std::string("New Amphibious Walrus enters a difficult landing island.");
}


bool TestCase_105::done()
{
    return isdone;
}
bool TestCase_105::passed()
{
    return haspassed;
}
std::string TestCase_105::failedMessage()
{
    return message;
}


// -----------
TestCase *pickTestCase(int testcase)
{
    return new TestCase_105();
}


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
#include "../units/BoxVehicle.h"
#include "../actions/ArtilleryAmmo.h"

#include "../engine.h"
#include "../profiling.h"

#include "../weapons/CarrierArtillery.h"
#include "../weapons/CarrierTurret.h"
#include "../weapons/CarrierLauncher.h"


#include "testcase_112.h"



extern unsigned long timer;

extern  Controller controller;
extern container<Vehicle*> entities;
extern std::vector<BoxIsland*> islands;
extern dWorldID world;
extern dSpaceID space;
extern int testing;
extern  Camera Camera;
extern int  aiplayer;


TestCase_112::TestCase_112()
{

}

void TestCase_112::init()
{


    BoxIsland *nemesis = new BoxIsland(&entities);
    nemesis->setName("Atom");
    nemesis->setLocation(0.0f,-1.0,0.0f);
    nemesis->buildTerrainModel(space,"terrain/thermopilae.bmp");

    islands.push_back(nemesis);



    //Vec3f pos(0.0,1.32, - 3500);
    Vec3f pos(-10,1.32,10);
    Camera.setPos(pos);

    aiplayer = FREE_AI;
    controller.faction = BOTH_FACTION;


}

int TestCase_112::check(unsigned long timertick)
{
    if (timertick > 6000)
    {
        isdone = true;
        haspassed = false;
        message = std::string("Match is over.");

        Vehicle *_b1 = findWalrus(GREEN_FACTION);

        Vehicle *_b2 = findWalrus(BLUE_FACTION);

        if (_b1)
        {
            printf("GREEN 1 : Health:%d\n", _b1->getHealth());
        }

        if (_b2)
        {
            printf("BLUE 2 : Health:%d\n", _b2->getHealth());
        }

    }

    return 0;
}

int TestCase_112::number()
{
    return 112;

}

std::string TestCase_112::title()
{
    return std::string("Checking factory of entities....");
}


bool TestCase_112::done()
{
    return isdone;
}
bool TestCase_112::passed()
{
    return haspassed;
}
std::string TestCase_112::failedMessage()
{
    return message;
}


// -----------
TestCase *pickTestCase(int testcase)
{
    return new TestCase_112();
}

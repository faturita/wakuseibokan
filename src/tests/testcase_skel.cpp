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

#include "../weapons/CarrierArtillery.h"
#include "../weapons/CarrierTurret.h"
#include "../weapons/CarrierLauncher.h"

#include "testcase_<TC>.h"

extern unsigned long timer;

extern  Controller controller;
extern container<Vehicle*> entities;
extern std::vector<BoxIsland*> islands;
extern dWorldID world;
extern dSpaceID space;
extern int testing;
extern  Camera camera;
extern int  aiplayer;

TestCase_<TC>::TestCase_118()
{

}


void TestCase_<TC>::init()
{



}

int TestCase_<TC>::check(unsigned long timertick)
{

    if (timertick > 18000)
    {
        Vehicle* _b = findCarrier(GREEN_FACTION);

        isdone = false;
        haspassed = false;

        if (_b)
        {
            isdone = true;
            haspassed = true;
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

int TestCase_<TC>::number()
{
    return <TC>;

}

std::string TestCase_<TC>::title()
{
    return std::string("Write the test/scenario description here");
}


bool TestCase_<TC>::done()
{
    return isdone;
}
bool TestCase_<TC>::passed()
{
    return haspassed;
}
std::string TestCase_<TC>::failedMessage()
{
    return message;
}


// -----------
TestCase *pickTestCase(int testcase)
{
    return new TestCase_<TC>();
}


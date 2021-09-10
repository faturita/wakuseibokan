#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../container.h"

#include "../terrain/Terrain.h"

#include "../units/Vehicle.h"
#include "../usercontrols.h"

#include "../units/Balaenidae.h"
#include "../units/Beluga.h"

#include "../units/AdvancedWalrus.h"

#include "../weapons/CarrierArtillery.h"
#include "../weapons/CarrierTurret.h"

#include "../engine.h"

#include "testcase_102.h"


extern unsigned long timer;

extern  Controller controller;
extern container<Vehicle*> entities;
extern std::vector<BoxIsland*> islands;
extern dWorldID world;
extern dSpaceID space;
extern int testing;

TestCase_102::TestCase_102()
{

}

void TestCase_102::init()
{
    BoxIsland *nemesis = new BoxIsland(&entities);
    nemesis->setName("Nemesis");
    nemesis->setLocation(0.0f,-1.0,0.0f);
    nemesis->buildTerrainModel(space,"terrain/thermopilae.bmp");

    islands.push_back(nemesis);

    // Strategy Game
    Balaenidae *_b = new Balaenidae(GREEN_FACTION);
    _b->init();
    //dSpaceID carrier_space = space;
    //_b->embody(world, space);
    dSpaceID carrier_space = _b->embody_in_space(world, space);
    //_b->setPos(0.0f + 0.0 kmf,20.5f,-4000.0f + 0.0 kmf);
    _b->setPos(0 kmf, 0, 0 kmf - 4000.0f);
    _b->stop();

    entities.push_back(_b, _b->getGeom());


    CarrierTurret * _bo= new CarrierTurret(GREEN_FACTION);
    _bo->init();
    _bo->embody(world, carrier_space);
    _bo->attachTo(world,_b, -40.0f, 20.0f + 5, -210.0f);
    _bo->stop();

    entities.push_back(_bo, _bo->getGeom());


    CarrierArtillery * _w1= new CarrierArtillery(GREEN_FACTION);
    _w1->init();
    _w1->embody(world, carrier_space);
    _w1->attachTo(world,_b, -40.0, 27.0f, +210.0f);
    _w1->stop();

    entities.push_back(_w1, _w1->getGeom());

}

int TestCase_102::check(unsigned long timertick)
{

    if (timertick == 1000)
    {
        Vehicle *_b = findCarrier(GREEN_FACTION);

        if (_b)
        {
            _b->destroy();
        }
    }

    if (timertick > 1000)
    {
        bool foundOne = false;
        for(size_t i=entities.first();entities.hasMore(i);i=entities.next(i))
        {
            Vehicle *v = entities[i];

            if (v->getType() == VehicleTypes::WEAPON)
            {
                foundOne = true;
            }

        }

        if (foundOne)
        {
            isdone=true;
            haspassed=false;
            message = std::string("Some body object which was attached to the entity was not eliminated.");
        }
        else
        {
            isdone=true;
            haspassed=true;
        }


    }


    if (timertick > 10000)
    {
        isdone = true;
        haspassed = false;
        message = std::string("The timeout has occurred and nothing happened.");
    }


    return 0;
}

int TestCase_102::number()
{
    return 102;

}

std::string TestCase_102::title()
{
    return std::string("Carrier Multibody entity.");
}


bool TestCase_102::done()
{
    return isdone;
}
bool TestCase_102::passed()
{
    return haspassed;
}
std::string TestCase_102::failedMessage()
{
    return message;
}


// -----------
TestCase *pickTestCase(int testcase)
{
    return new TestCase_102();
}


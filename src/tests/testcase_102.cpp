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

    Beluga *_bg = new Beluga(BLUE_FACTION);
    _bg->init();
    dSpaceID carrier_space_beluga = _bg->embody_in_space(world, space);
    _bg->setPos(0.0,20.5f,-4000.0f);
    _bg->stop();

    entities.push_back(_bg, _bg->getGeom());


    CarrierTurret * _bl= new CarrierTurret(BLUE_FACTION);
    _bl->init();
    _bl->embody(world, carrier_space_beluga);
    _bl->attachTo(world,_bg, +30.0f, 20.0f - 3, +204.0f);
    _bl->stop();

    _bg->addWeapon(entities.push_back(_bl, _bl->getGeom()));

    CarrierTurret * _br= new CarrierTurret(BLUE_FACTION);
    _br->init();
    _br->embody(world, carrier_space_beluga);
    _br->attachTo(world,_bg, -45.0f, 20.0f - 3, +204.0f);
    _br->stop();

    _bg->addWeapon(entities.push_back(_br, _br->getGeom()));


    CarrierArtillery * _wr= new CarrierArtillery(BLUE_FACTION);
    _wr->init();
    _wr->embody(world, carrier_space_beluga);
    _wr->attachTo(world,_bg, -40.0, 27.0f+5, -230.0f);
    _wr->stop();

    _bg->addWeapon(entities.push_back(_wr, _wr->getGeom()));

    CarrierArtillery * _wl= new CarrierArtillery(BLUE_FACTION);
    _wl->init();
    _wl->embody(world, carrier_space_beluga);
    _wl->attachTo(world,_bg, +40.0, 27.0f+2, -230.0f);
    _wl->stop();

    _bg->addWeapon(entities.push_back(_wl, _wl->getGeom()));

    CarrierLauncher * _cf= new CarrierLauncher(BLUE_FACTION);
    _cf->init();
    _cf->embody(world, carrier_space_beluga);
    _cf->attachTo(world,_bg, +40.0, 27.0f+2, 0.0);
    _cf->stop();

    _bg->addWeapon(entities.push_back(_cf, _cf->getGeom()));

}

int TestCase_102::check(unsigned long timertick)
{

    if (timertick == 1000)
    {
        Vehicle *_b = findCarrier(BLUE_FACTION);

        if (_b)
        {
            _b->destroy();
        }
    }

    if (timertick > 1200)
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


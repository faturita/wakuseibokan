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


#include "testcase_108.h"


extern unsigned long timer;

extern  Controller controller;
extern container<Vehicle*> entities;
extern std::vector<BoxIsland*> islands;
extern dWorldID world;
extern dSpaceID space;
extern int testing;
extern  Camera Camera;
extern int  aiplayer;

TestCase_108::TestCase_108()
{

}

void TestCase_108::init()
{
    BoxIsland *nemesis = new BoxIsland(&entities);
    nemesis->setName("Atom");
    nemesis->setLocation(0.0f,-1.0,0.0f);
    nemesis->buildTerrainModel(space,"terrain/fulcrum.bmp");

    islands.push_back(nemesis);

    Beluga *_bg = new Beluga(BLUE_FACTION);
    _bg->init();
    dSpaceID carrier_space_beluga = _bg->embody_in_space(world, space);
    _bg->setPos(-5000.0,20.5f,-14000.0f);
    _bg->stop();

    entities.push_back(_bg, _bg->getGeom());


    CarrierTurret * _bl= new CarrierTurret(BLUE_FACTION);
    _bl->init();
    _bl->embody(world, carrier_space_beluga);
    _bl->attachTo(world,_bg, +30.0f, 20.0f - 3, +204.0f);
    _bl->stop();

    entities.push_back(_bl, _bl->getGeom());

    CarrierTurret * _br= new CarrierTurret(BLUE_FACTION);
    _br->init();
    _br->embody(world, carrier_space_beluga);
    _br->attachTo(world,_bg, -45.0f, 20.0f - 3, +204.0f);
    _br->stop();

    entities.push_back(_br, _br->getGeom());


    CarrierArtillery * _wr= new CarrierArtillery(BLUE_FACTION);
    _wr->init();
    _wr->embody(world, carrier_space_beluga);
    _wr->attachTo(world,_bg, -40.0, 27.0f+5, -230.0f);
    _wr->stop();

    entities.push_back(_wr, _wr->getGeom());

    CarrierArtillery * _wl= new CarrierArtillery(BLUE_FACTION);
    _wl->init();
    _wl->embody(world, carrier_space_beluga);
    _wl->attachTo(world,_bg, +40.0, 27.0f+2, -230.0f);
    _wl->stop();

    entities.push_back(_wl, _wl->getGeom());

    CarrierLauncher * _cf= new CarrierLauncher(BLUE_FACTION);
    _cf->init();
    _cf->embody(world, carrier_space_beluga);
    _cf->attachTo(world,_bg, +40.0, 27.0f+2, 0.0);
    _cf->stop();

    entities.push_back(_cf, _cf->getGeom());

    Structure *t1 = islands[0]->addStructure(new CommandCenter(BLUE_FACTION, FACTORY_ISLAND)    ,       200.0f,    -100.0f,0,world);
    Structure *t2 = islands[0]->addStructure(new Launcher(BLUE_FACTION)           ,         0.0f,    -1700.0f,0,world);
    Structure *t3 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)      ,         0.0f,    650.0f,0,world);
    Structure *t4 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)        ,       100.0f,    -650.0f,0,world);
    Structure *t5 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)        ,        20.0f,    80.0f,0,world);
    Structure *t6 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)        ,         -60.0f,    -80.0f,0,world);
    Structure *t7 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)        ,         0.0f,    120.0f,0,world);
    Structure *t8 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)        ,         -230.0f,    230.0f,0,world);


    //Vec3f pos(0.0,1.32, - 3500);
    Vec3f pos(-10,1.32,10);
    Camera.setPos(pos);

    aiplayer = BLUE_AI;
    controller.faction = BOTH_FACTION;

}

int TestCase_108::check(unsigned long timertick)
{
    if (timertick == 200)
    {

    }

    if (timertick > 8000)
    {
        Vehicle* _b = findCarrier(GREEN_FACTION);

        isdone = false;
        haspassed = false;

        if (!_b)
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

int TestCase_108::number()
{
    return 107;

}

std::string TestCase_108::title()
{
    return std::string("Check the new weapons of Beluga.");
}


bool TestCase_108::done()
{
    return isdone;
}
bool TestCase_108::passed()
{
    return haspassed;
}
std::string TestCase_108::failedMessage()
{
    return message;
}


// -----------
TestCase *pickTestCase(int testcase)
{
    return new TestCase_108();
}


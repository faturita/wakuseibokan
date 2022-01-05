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

#include "testcase_107.h"


extern unsigned long timer;

extern  Controller controller;
extern container<Vehicle*> entities;
extern std::vector<BoxIsland*> islands;
extern dWorldID world;
extern dSpaceID space;
extern int testing;
extern  Camera Camera;
extern int  aiplayer;

TestCase_107::TestCase_107()
{

}

void TestCase_107::init()
{
    BoxIsland *nemesis = new BoxIsland(&entities);
    nemesis->setName("Atom");
    nemesis->setLocation(0.0f,-1.0,0.0f);
    nemesis->buildTerrainModel(space,"terrain/fulcrum.bmp");

    islands.push_back(nemesis);

    Balaenidae *_b = new Balaenidae(GREEN_FACTION);
    _b->init();
    dSpaceID carrier_space = _b->embody_in_space(world, space);
    _b->setPos(-5000.0,20.5f,-14000.0f);
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

int TestCase_107::check(unsigned long timertick)
{
    if (timertick == 200)
    {
        Vehicle* _b = findCarrier(GREEN_FACTION);

        Vec3f dest = Vec3f(-2000.0f, 20.5, -4000.0f);
        _b->goTo(dest);
        _b->enableAuto();
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

int TestCase_107::number()
{
    return 107;

}

std::string TestCase_107::title()
{
    return std::string("Checking torpedo firing from islands.");
}


bool TestCase_107::done()
{
    return isdone;
}
bool TestCase_107::passed()
{
    return haspassed;
}
std::string TestCase_107::failedMessage()
{
    return message;
}


// -----------
TestCase *pickTestCase(int testcase)
{
    return new TestCase_107();
}

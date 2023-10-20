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

#include "testcase_120.h"

extern unsigned long timer;

extern  Controller controller;
extern container<Vehicle*> entities;
extern std::vector<BoxIsland*> islands;
extern dWorldID world;
extern dSpaceID space;
extern int testing;
extern  Camera Camera;
extern int  aiplayer;

TestCase_120::TestCase_120()
{

}


Otter*  addOtter(Vec3f pos, float angle, int faction)
{
    // 6,3,12
    Otter *_otter = new Otter(faction);
    _otter->init();
    dSpaceID car_space = _otter->embody_in_space(world, space);

    _otter->setPos(pos);
    _otter->stop();
    _otter->setSignal(4);
    _otter->setNameByNumber(1);
    _otter->setStatus(SailingStatus::ROLLING);
    _otter->setIsland(islands[0]);

    Vec3f dimensions(5.0f,4.0f,10.0f);

    entities.push_back(_otter, _otter->getGeom());


    Wheel * _fr= new Wheel(faction, 0.001, 30.0);
    _fr->init();
    _fr->embody(world, car_space);
    _fr->attachTo(world,_otter,4.9f, -3.0, 5.8);
    _fr->stop();

    entities.push_back(_fr, _fr->getGeom());


    Wheel * _fl= new Wheel(faction, 0.001, 30.0);
    _fl->init();
    _fl->embody(world, car_space);
    _fl->attachTo(world,_otter, -4.9f, -3.0, 5.8);
    _fl->stop();

    entities.push_back(_fl, _fl->getGeom());


    Wheel * _br= new Wheel(faction, 0.001, 30.0);
    _br->init();
    _br->embody(world, car_space);
    _br->attachTo(world,_otter, 4.9f, -3.0, -5.8);
    _br->stop();

    entities.push_back(_br, _br->getGeom());


    Wheel * _bl= new Wheel(faction, 0.001, 30.0);
    _bl->init();
    _bl->embody(world, car_space);
    _bl->attachTo(world,_otter, -4.9f, -3.0, -5.8);
    _bl->stop();

    entities.push_back(_bl, _bl->getGeom());

    _otter->addWheels(_fl, _fr, _bl, _br);

    _fl->setSteering(true);
    _fr->setSteering(true);

    dMatrix3 Re2;
    dRSetIdentity(Re2);
    dRFromAxisAndAngle(Re2,0.0,1.0,0.0,angle);
    dBodySetRotation(_otter->getBodyID(),Re2);

    _otter->enableAuto();

    return _otter;


}

void TestCase_120::init()
{
    BoxIsland *nemesis = new BoxIsland(&entities);
    nemesis->setName("Nemesis");
    nemesis->setLocation(0.0f,-1.0,0.0f);
    nemesis->buildTerrainModel(space);

    islands.push_back(nemesis);


    BoxIsland *atom = new BoxIsland(&entities);
    atom->setName("Atom");
    atom->setLocation(0.0f,-1.0,6 kmf);
    atom->buildTerrainModel(space);

    islands.push_back(atom);

    Otter *_otter1 = addOtter(Vec3f(0,5,-3000.0), 0,GREEN_FACTION);


}

int TestCase_120::check(unsigned long timertick)
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

int TestCase_120::number()
{
    return 120;

}

std::string TestCase_120::title()
{
    return std::string("Write the test/scenario description here");
}


bool TestCase_120::done()
{
    return isdone;
}
bool TestCase_120::passed()
{
    return haspassed;
}
std::string TestCase_120::failedMessage()
{
    return message;
}


// -----------
TestCase *pickTestCase(int testcase)
{
    return new TestCase_120();
}


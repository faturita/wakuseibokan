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

#include "testcase_117.h"

extern unsigned long timer;

extern  Controller controller;
extern container<Vehicle*> entities;
extern std::vector<BoxIsland*> islands;
extern dWorldID world;
extern dSpaceID space;
extern int testing;
extern  Camera Camera;
extern int  aiplayer;

TestCase_117::TestCase_117()
{

}


void TestCase_117::init()
{
    BoxIsland *nemesis = new BoxIsland(&entities);
    nemesis->setName("Atom");
    nemesis->setLocation(0.0f,-1.0,0.0f);
    nemesis->buildTerrainModel(space,"terrain/track.bmp");

    islands.push_back(nemesis);


    {
        // 6,3,12
        Otter *_otter = new Otter(BLUE_FACTION);
        _otter->init();
        dSpaceID car_space = _otter->embody_in_space(world, space);

        _otter->setPos(getRandomCircularSpot(Vec3f(0.0,10,0.0),100.0f));
        _otter->setPos(130.0,10.0,-1344.0);
        _otter->stop();
        _otter->setSignal(4);
        _otter->setNameByNumber(1);
        _otter->setStatus(SailingStatus::ROLLING);
        _otter->setIsland(nemesis);

        Vec3f dimensions(5.0f,4.0f,10.0f);

        entities.push_back(_otter, _otter->getGeom());



        Wheel * _fr= new Wheel(BLUE_FACTION, 0.001, 30.0);
        _fr->init();
        _fr->embody(world, car_space);
        _fr->attachTo(world,_otter,4.9f, -3.0, 5.8);
        _fr->stop();

        entities.push_back(_fr, _fr->getGeom());


        Wheel * _fl= new Wheel(BLUE_FACTION, 0.001, 30.0);
        _fl->init();
        _fl->embody(world, car_space);
        _fl->attachTo(world,_otter, -4.9f, -3.0, 5.8);
        _fl->stop();

        entities.push_back(_fl, _fl->getGeom());


        Wheel * _br= new Wheel(BLUE_FACTION, 0.001, 30.0);
        _br->init();
        _br->embody(world, car_space);
        _br->attachTo(world,_otter, 4.9f, -3.0, -5.8);
        _br->stop();

        entities.push_back(_br, _br->getGeom());


        Wheel * _bl= new Wheel(BLUE_FACTION, 0.001, 30.0);
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
        dRFromAxisAndAngle(Re2,0.0,1.0,0.0,PI/2.0);
        //dRFromAxisAndAngle(Re2,0.0,1.0,0.0,getRandomInteger((int)-PI/2.0+PI,(int)PI/2.0)/10.0+PI);
        dBodySetRotation(_otter->getBodyID(),Re2);

    }



    //Structure *t1 = islands[0]->addStructure(new CommandCenter(BLUE_FACTION, FACTORY_ISLAND)    ,       200.0f,    -100.0f,0,world);


    //Vec3f pos(0.0,1.32, - 3500);
    Vec3f pos(-10,1.32,10);
    Camera.setPos(pos);

    aiplayer = BOTH_AI;
    controller.faction = BOTH_FACTION;

}

int TestCase_117::check(unsigned long timertick)
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

int TestCase_117::number()
{
    return 117;

}

std::string TestCase_117::title()
{
    return std::string("Let the Walrus drive.");
}


bool TestCase_117::done()
{
    return isdone;
}
bool TestCase_117::passed()
{
    return haspassed;
}
std::string TestCase_117::failedMessage()
{
    return message;
}


// -----------
TestCase *pickTestCase(int testcase)
{
    return new TestCase_117();
}

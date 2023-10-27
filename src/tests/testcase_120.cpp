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
#include "../units/Turtle.h"

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


Otter*  addOtter(Vec3f pos, float angle, int faction, int number)
{
    // 6,3,12
    Otter *_otter = new Otter(faction);
    _otter->init();
    dSpaceID car_space = _otter->embody_in_space(world, space);

    _otter->setPos(pos);
    _otter->stop();
    _otter->setSignal(4);
    _otter->setNameByNumber(number);
    _otter->setStatus(SailingStatus::ROLLING);
    _otter->setIsland(islands[0]);

    // The multibody entity is not added first here, is added by the spawner


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
    nemesis->buildRegularTerrainModel(space);

    islands.push_back(nemesis);

    Turtle *_turtle = spawnTurtle(Vec3f(0,100.0,0), PI, GREEN_FACTION,1,4,space,world);
    _turtle->setIsland(nemesis);

    entities.push_back(_turtle, _turtle->getGeom());

    Otter *_otter1 = addOtter(Vec3f(0,5,-3000.0), 0,BLUE_FACTION,1);
    entities.push_back(_otter1, _otter1->getGeom());

    Wheel *l,*r,*bl,*br;
    _turtle->setPos(Vec3f(0,100.0,0));
    _turtle->getWheels(l,r,bl,br);
    l->setPos(Vec3f(0,100.0,0));
    r->setPos(Vec3f(0,100.0,0));
    bl->setPos(Vec3f(0,100.0,0));
    br->setPos(Vec3f(0,100.0,0));

    Structure *t1 = islands[0]->addStructure(new CommandCenter(GREEN_FACTION, DEFENSE_ISLAND)    ,       200.0f,    -100.0f,0,world);
    Structure *t2 = islands[0]->addStructure(new Armory(GREEN_FACTION)           ,         0.0f,    -650.0f,0,world);
    Structure *t3 = islands[0]->addStructure(new Antenna(GREEN_FACTION)      ,         0.0f,    650.0f,0,world);
    Structure *t4 = islands[0]->addStructure(new Launcher(GREEN_FACTION)      ,       -650.0f,    0.0f,0,world);



    Vec3f pos(0,10,-4000);
    Camera.setPos(pos);
    Camera.dy = 0;
    Camera.dz = 0;
    Camera.xAngle = 0;
    Camera.yAngle = 0;
    controller.controllingid = CONTROLLING_NONE;

    aiplayer = BOTH_AI;
    controller.faction = BOTH_FACTION;

}

int TestCase_120::check(unsigned long timertick)
{

    if (timertick == 1)
    {
        BoxIsland *nemesis = (BoxIsland*) islands[0];
        Turtle *_turtle = (Turtle*) entities[1];

        Walrus *_otter = findWalrus(BLUE_FACTION);

        if (_otter && _turtle)
        {
            _otter->attack(_turtle->getPos());
        }

    }
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
    return std::string("Check the terrain island and the tank.");
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


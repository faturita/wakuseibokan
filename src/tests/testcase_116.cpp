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

#include "testcase_116.h"

extern unsigned long timer;

extern  Controller controller;
extern container<Vehicle*> entities;
extern std::vector<BoxIsland*> islands;
extern dWorldID world;
extern dSpaceID space;
extern int testing;
extern  Camera Camera;
extern int  aiplayer;

TestCase_116::TestCase_116()
{

}


void TestCase_116::init()
{
    BoxIsland *nemesis = new BoxIsland(&entities);
    nemesis->setName("Atom");
    nemesis->setLocation(0.0f,-1.0,0.0f);
    nemesis->buildTerrainModel(space,"terrain/fulcrum.bmp");

    islands.push_back(nemesis);

    Balaenidae *_b = new Balaenidae(GREEN_FACTION);
    _b->init();
    dSpaceID carrier_space = _b->embody_in_space(world, space);
    //b->setPos(0.0f + 0.0 kmf,20.5f,-4000.0f + 0.0 kmf);
    _b->setPos(0.0, 20.5f, - 4000.0f);
    _b->stop();

    entities.push_back(_b, _b->getGeom());

    CarrierTurret * _bo= new CarrierTurret(GREEN_FACTION);
    _bo->init();
    _bo->embody(world, carrier_space);
    _bo->attachTo(world,_b, -40.0f, 20.0f + 5, -210.0f);
    _bo->stop();

    _b->addWeapon(entities.push_back(_bo, _bo->getGeom()));


    CarrierArtillery * _w1= new CarrierArtillery(GREEN_FACTION);
    _w1->init();
    _w1->embody(world, carrier_space);
    _w1->attachTo(world,_b, -40.0, 27.0f, +210.0f);
    _w1->stop();

    _b->addWeapon(entities.push_back(_w1, _w1->getGeom()));


    {
        // 6,3,12
        Otter *_otter = new Otter(BLUE_FACTION);
        _otter->init();
        dSpaceID car_space = _otter->embody_in_space(world, space);
        _otter->setPos(400.0f,70.0f,-4400.0f);
        _otter->setPos(40.0f,30.0f,-0.0f);

        _otter->setPos(Vec3f(2000.0,20.0f,-4000.0+getRandomInteger(-600,600)));
        _otter->setPos(getRandomCircularSpot(Vec3f(0.0,10,-6000.0),2000.0f));
        _otter->stop();
        _otter->setSignal(4);
        _otter->setNameByNumber(1);
        _otter->setStatus(SailingStatus::SAILING);

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
        //dRFromAxisAndAngle(Re2,0.0,1.0,0.0,-PI/4.0);
        dRFromAxisAndAngle(Re2,0.0,1.0,0.0,getRandomInteger((int)-PI/2.0+PI,(int)PI/2.0)/10.0+PI);
        dBodySetRotation(_otter->getBodyID(),Re2);

        _otter->attack(_b->getPos());
        _otter->enableAuto();

    }



    Structure *t1 = islands[0]->addStructure(new CommandCenter(BLUE_FACTION, FACTORY_ISLAND)    ,       200.0f,    -100.0f,0,world);
    Structure *t2 = islands[0]->addStructureAtDesiredHeight(new Dock(BLUE_FACTION), world, 0);
    Structure *t3 = islands[0]->addStructure(new Antenna(BLUE_FACTION)      ,         0.0f,    650.0f,0,world);
    Structure *t4 = islands[0]->addStructure(new Runway(BLUE_FACTION)        ,       100.0f,    -650.0f,0,world);
    Structure *t5 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)        ,       290.0f,    80.0f,0,world);
    Structure *t6 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)        ,         -60.0f,    -80.0f,0,world);
    Structure *t7 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)        ,         0.0f,    120.0f,0,world);
    Structure *t8 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)        ,         -230.0f,    230.0f,0,world);




    //Vec3f pos(0.0,1.32, - 3500);
    Vec3f pos(-10,1.32,10);
    Camera.setPos(pos);

    aiplayer = BOTH_AI;
    controller.faction = BOTH_FACTION;

}

int TestCase_116::check(unsigned long timertick)
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

int TestCase_116::number()
{
    return 116;

}

std::string TestCase_116::title()
{
    return std::string("Check carrier defences.");
}


bool TestCase_116::done()
{
    return isdone;
}
bool TestCase_116::passed()
{
    return haspassed;
}
std::string TestCase_116::failedMessage()
{
    return message;
}


// -----------
TestCase *pickTestCase(int testcase)
{
    return new TestCase_116();
}


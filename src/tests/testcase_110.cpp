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


#include "testcase_110.h"


extern unsigned long timer;

extern  Controller controller;
extern container<Vehicle*> entities;
extern std::vector<BoxIsland*> islands;
extern dWorldID world;
extern dSpaceID space;
extern int testing;
extern  Camera Camera;
extern int  aiplayer;

TestCase_110::TestCase_110()
{

}

void TestCase_110::init()
{
    BoxIsland *nemesis = new BoxIsland(&entities);
    nemesis->setName("Atom");
    nemesis->setLocation(0.0f,-1.0,0.0f);
    nemesis->buildTerrainModel(space,"terrain/fulcrum.bmp");

    islands.push_back(nemesis);


    // 6,3,12
    Otter *_otter = new Otter(GREEN_FACTION);
    _otter->init();
    dSpaceID car_space = _otter->embody_in_space(world, space);
    _otter->setPos(400.0f,70.0f,-4400.0f);
    _otter->setPos(40.0f,30.0f,-0.0f);

    _otter->setPos(Vec3f(getRandomInteger(-120,120),30.0f,getRandomInteger(-60,60)));
    _otter->stop();
    _otter->setSignal(4);
    _otter->setNameByNumber(1);
    _otter->setStatus(SailingStatus::ROLLING);

    Vec3f dimensions(5.0f,4.0f,10.0f);

    entities.push_back(_otter, _otter->getGeom());



    Wheel * _fr= new Wheel(GREEN_FACTION, 0.001, 30.0);
    _fr->init();
    _fr->embody(world, car_space);
    _fr->attachTo(world,_otter,4.9f, -3.0, 5.8);
    _fr->stop();

    entities.push_back(_fr, _fr->getGeom());


    Wheel * _fl= new Wheel(GREEN_FACTION, 0.001, 30.0);
    _fl->init();
    _fl->embody(world, car_space);
    _fl->attachTo(world,_otter, -4.9f, -3.0, 5.8);
    _fl->stop();

    entities.push_back(_fl, _fl->getGeom());


    Wheel * _br= new Wheel(GREEN_FACTION, 0.001, 30.0);
    _br->init();
    _br->embody(world, car_space);
    _br->attachTo(world,_otter, 4.9f, -3.0, -5.8);
    _br->stop();

    entities.push_back(_br, _br->getGeom());


    Wheel * _bl= new Wheel(GREEN_FACTION, 0.001, 30.0);
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
    dRFromAxisAndAngle(Re2,0.0,1.0,0.0,getRandomInteger((int)-PI/4*10,(int)PI/4*10)/10.0);
    dBodySetRotation(_otter->getBodyID(),Re2);

    _otter->goTo(Vec3f(0,0,340));
    _otter->enableAuto();


    Structure *t1 = islands[0]->addStructure(new CommandCenter(GREEN_FACTION, FACTORY_ISLAND)    ,       200.0f,    -100.0f,0,world);
    //Structure *t6 = islands[0]->addStructure(new Warehouse(GREEN_FACTION)        ,         -0.0f,    80.0f,PI/2.0,world);

    Structure *t7 = islands[0]->addStructure(new Warehouse(GREEN_FACTION)        ,        45.0f,     340.0f,0,world);
    Structure *t8 = islands[0]->addStructure(new Warehouse(GREEN_FACTION)        ,       -45.0f,     340.0f,0,world);



    //Vec3f pos(0.0,1.32, - 3500);
    Vec3f pos(-10,1.32,10);
    Camera.setPos(pos);

    aiplayer = BLUE_AI;
    controller.faction = BOTH_FACTION;

}

int TestCase_110::check(unsigned long timertick)
{

    if (timertick > 100)
    {
        Vehicle* _b = findWalrus(GREEN_FACTION);

        Controller co;
        co.registers.thrust = 10;
        co.registers.roll = 1;

        //_b->doControl(co);

        isdone = false;
        haspassed = false;

        if (_b)
        {
            Vec3f pos = _b->getPos();

            dout << (pos-Vec3f(45,pos[1],340)).magnitude() <<  ":-: " << (pos-Vec3f(-45,pos[1],340)).magnitude() << std::endl;

            // This only happen between the two warehouses.
            if ( ((pos-Vec3f(45,pos[1],340)).magnitude()<60) &&
                ((pos-Vec3f(-45,pos[1],340)).magnitude()<60)  )
            {
                isdone = true;
                haspassed = true;
            }
        }
    }


    if (timertick > 5000)
    {
        isdone = true;
        haspassed = false;
        message = std::string("Timeout for parking....");
    }

    return 0;
}

int TestCase_110::number()
{
    return 110;

}

std::string TestCase_110::title()
{
    return std::string("Parking in the parking spot, between the two warehouses.");
}


bool TestCase_110::done()
{
    return isdone;
}
bool TestCase_110::passed()
{
    return haspassed;
}
std::string TestCase_110::failedMessage()
{
    return message;
}


// -----------
TestCase *pickTestCase(int testcase)
{
    return new TestCase_110();
}

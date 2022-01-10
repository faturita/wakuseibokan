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

#include "../engine.h"

#include "testcase_104.h"


extern unsigned long timer;

extern  Controller controller;
extern container<Vehicle*> entities;
extern std::vector<BoxIsland*> islands;
extern dWorldID world;
extern dSpaceID space;
extern int testing;
extern  Camera Camera;

TestCase_104::TestCase_104()
{

}

void TestCase_104::init()
{
    BoxIsland *nemesis = new BoxIsland(&entities);
    nemesis->setName("Nemesis");
    nemesis->setLocation(0.0f,-1.0,0.0f);
    nemesis->buildTerrainModel(space,"terrain/gaijin.bmp");

    islands.push_back(nemesis);

    // Entities will be added later in time.
    Balaenidae *_b = new Balaenidae(GREEN_FACTION);
    _b->init();
    _b->embody(world,space);
    _b->setPos(0.0f,20.5f,-4000.0f);
    _b->stop();

    entities.push_back(_b, _b->getGeom());


    // 6,3,12
    Otter *_otter = new Otter(BLUE_FACTION);
    _otter->init();
    dSpaceID car_space = _otter->embody_in_space(world, space);
    _otter->setPos(400.0f,70.0f,-4400.0f);
    _otter->setPos(0.0f,70.0f,-0.0f);
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


    Walrus *_walrus = new Walrus(GREEN_FACTION);

    _walrus->init();
    _walrus->embody(world, space);
    _walrus->setPos(0.0f,20.5f,2000);
    _walrus->setStatus(SailingStatus::SAILING);
    _walrus->setSignal(4);

    size_t idx = entities.push_back(_walrus, _walrus->getGeom());


    Vec3f pos(0.0,1.32, - 3500);
    Camera.setPos(pos);

    //aiplayer = BOTH_AI;
    controller.faction = BOTH_FACTION;

}

int TestCase_104::check(unsigned long timertick)
{
    Vehicle *_b = findCarrier(GREEN_FACTION);

    if (timertick == 100)
    {
        size_t idx = 0;
        spawnManta(space,world,_b,idx);
    }

    if (timertick == 320)
    {
        // launch
        launchManta(_b);
    }

    if (timertick == 400)
    {
        Walrus* _w = findWalrus(BLUE_FACTION);

        Manta *m = findManta(GREEN_FACTION);

        if (m)
        {
            if (_w)
            {
                m->enableAuto();
                m->dogfight(_w->getPos());


                _w->attack(m->getPos());
                _w->enableAuto();
            }
        }
    }

    if (timertick > 400)
    {
        Walrus* _w = findWalrus(BLUE_FACTION);

        Manta *m = findManta(GREEN_FACTION);

        if (m)
        {
            if (_w)
            {
                m->dogfight(_w->getPos());
                _w->attack(m->getPos());

            }
        }
    }

    if (timertick > 5000)
    {
        Walrus *_w = findWalrus(BLUE_FACTION);

        bool hasbeenhit = false;
        if (!_w || _w->getHealth()<1000)
            hasbeenhit = true;

        if (!hasbeenhit)
        {
            isdone=true;
            haspassed=false;
            message = std::string("For some reason, the new multibody entity has not been hit by a Manta.");
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

int TestCase_104::number()
{
    return 103;

}

std::string TestCase_104::title()
{
    return std::string("Carrier Multibody entity.");
}


bool TestCase_104::done()
{
    return isdone;
}
bool TestCase_104::passed()
{
    return haspassed;
}
std::string TestCase_104::failedMessage()
{
    return message;
}


// -----------
TestCase *pickTestCase(int testcase)
{
    return new TestCase_104();
}

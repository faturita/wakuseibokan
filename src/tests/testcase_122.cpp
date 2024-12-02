#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <chrono>

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

#include "testcase_122.h"

extern unsigned long timer;
extern float fps;

extern  Controller controller;
extern container<Vehicle*> entities;
extern std::vector<BoxIsland*> islands;
extern dWorldID world;
extern dSpaceID space;
extern int testing;
extern  Camera camera;
extern int  aiplayer;

TestCase_122::TestCase_122()
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
    //_otter->no_damping_on_bullets = true;
    //_otter->firepower = 100.0;

    return _otter;


}

void TestCase_122::init()
{
    BoxIsland *nemesis = new BoxIsland(&entities);
    nemesis->setName("Atom");
    nemesis->setLocation(0.0f,-1.0,0.0f);
    nemesis->buildTerrainModel(space,"terrain/thermopilae.bmp");

    islands.push_back(nemesis);

    Otter *_otter1 = addOtter(Vec3f(0,5,+500), 0,GREEN_FACTION);
    Otter *_otter2 = addOtter(Vec3f(0,5,-500), 0, BLUE_FACTION);

    Wheel* l;
    Wheel* r;
    Wheel* bl;
    Wheel* br;


    Vec3f pos(-1000,10,0);
    camera.setPos(pos);
    camera.dy = 0;
    camera.dz = 0;
    camera.xAngle = -90;
    camera.yAngle = 0;
    controller.controllingid = CONTROLLING_NONE;

    aiplayer = FREE_AI;
    controller.faction = BOTH_FACTION;

}


int TestCase_122::check(unsigned long timertick)
{
    static auto start = std::chrono::steady_clock::now();
    static unsigned long startick;
    Vehicle* _b = findWalrus(BLUE_FACTION);

    if (timertick == 200)
    {

        Controller co;
        co.registers.pitch = 71;

        _b->doControl(co);

        Vehicle *action = _b->fire(0,world, space);

        if (action != NULL)
        {
            entities.push_back(action, action->getGeom());
            gunshot(_b->getPos());
            start = std::chrono::steady_clock::now();
            startick = timertick;
        }
    }

    Vehicle* _w = findWalrus(GREEN_FACTION);

    if (!_w || _w->getHealth() < 1000.0)
    {
        isdone = true;
        haspassed = true;

        auto end = std::chrono::steady_clock::now();

        float elapsedtimemilliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        float elapsedtimeticksmilliseconds = ((timertick-startick)*1000.0)/(fps);
        float estimatedtimemilliseconds = 1000.0;   // @FIXME Need to calculate this with drag.
        float estimatedtimeticksmilliseconds = (timertick-startick)*0.05*1000.0 ;


        std::cout << "Elapsed time in milliseconds: "
             << elapsedtimemilliseconds
             << " ms" << std::endl;

        std::cout << "Elapsed time at "<< fps << " Hz from ticks: " << timertick-startick << " ticks " << elapsedtimeticksmilliseconds << " ms" << std::endl;

        std::cout << "Estimated time of fall for a projectile with Vo=100 m/s:" << estimatedtimemilliseconds << " ms" << std::endl;

        std::cout << "At simulation step 0.05, we have " << (1.0/0.05) << " ticks per second of simulation, so  " << timertick-startick << " ticks are:" << estimatedtimeticksmilliseconds << " ms" << std::endl;


        //assert( abs(elapsedtimemilliseconds-elapsedtimeticksmilliseconds) < 1000.0 || !"Elapsed Ticks and time do not match.");
        //assert( abs(estimatedtimemilliseconds-estimatedtimeticksmilliseconds) < 1000.0 || !"Estimated simulation time from ticks and theoretical time do not match.");

    }


    if (timertick > 15000)
    {
        isdone = true;
        haspassed = false;
        message = std::string("The timeout has occurred and nothing happened.");
    }

    return 0;
}

int TestCase_122::number()
{
    return 119;

}

std::string TestCase_122::title()
{
    return std::string("Consistency of target aiming with drag (like 111).");
}


bool TestCase_122::done()
{
    return isdone;
}
bool TestCase_122::passed()
{
    return haspassed;
}
std::string TestCase_122::failedMessage()
{
    return message;
}


// -----------
TestCase *pickTestCase(int testcase)
{
    return new TestCase_122();
}


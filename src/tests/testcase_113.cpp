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


#include "testcase_113.h"


extern unsigned long timer;

extern  Controller controller;
extern container<Vehicle*> entities;
extern std::vector<BoxIsland*> islands;
extern dWorldID world;
extern dSpaceID space;
extern int testing;
extern  Camera Camera;
extern int  aiplayer;

TestCase_113::TestCase_113()
{

}

void TestCase_113::init()
{
    // Strategy Game
    Balaenidae *_b = new Balaenidae(GREEN_FACTION);
    _b->init();
    //dSpaceID carrier_space = space;
    //_b->embody(world, space);
    dSpaceID carrier_space = _b->embody_in_space(world, space);
    //_b->setPos(0.0f + 0.0 kmf,20.5f,-4000.0f + 0.0 kmf);
    _b->setPos(0 kmf, 0, 0 kmf - 14000.0f);
    _b->stop();

    entities.push_back(_b, _b->getGeom());


    BoxIsland *nemesis = new BoxIsland(&entities);
    nemesis->setName("Atom");
    nemesis->setLocation(0.0f,-1.0,0.0f);
    nemesis->buildTerrainModel(space,"terrain/fulcrum.bmp");

    islands.push_back(nemesis);


    Structure *t1 = islands[0]->addStructure(new CommandCenter(BLUE_FACTION, FACTORY_ISLAND)    ,       200.0f,    -100.0f,0,world);
    //Structure *t6 = islands[0]->addStructure(new Warehouse(GREEN_FACTION)        ,         -0.0f,    80.0f,PI/2.0,world);

    Structure *t7 = islands[0]->addStructure(new Warehouse(BLUE_FACTION)        ,        450.0f,     340.0f,0,world);
    Structure *t8 = islands[0]->addStructure(new Antenna(BLUE_FACTION)        ,       -450.0f,     340.0f,0,world);



    //Vec3f pos(0.0,1.32, - 3500);
    Vec3f pos(-10,1.32,10);
    Camera.setPos(pos);

    aiplayer = BLUE_AI;
    controller.faction = BOTH_FACTION;

}

int TestCase_113::check(unsigned long timertick)
{

    if (timertick == 100)
    {
        Balaenidae *_b = (Balaenidae*) entities[0];

        Vehicle *action = _b->fire(0,world,space);

        if (action != NULL)
        {
            size_t index = entities.push_back(action, action->getGeom());
            gunshot();
            switchControl(index);

            missile = index;
        }
    }

    if (timertick > 300 && timertick < 600)
    {
        Vehicle *action = entities[missile];
        action->stop();

    }



    if (timertick == 350)
    {
        controller.controllingid = CONTROLLING_NONE;
        Vec3f pos = islands[0]->getPos();

        pos += Vec3f(200.0f,10,-100.0-500.0);

        Camera.setPos(pos);
        Camera.fw = Vec3f(0.0f,0.0f,1.0f);

    }


    if (timertick == 450)
    {
        controller.controllingid = CONTROLLING_NONE;
        Vec3f pos = islands[0]->getPos();

        pos += Vec3f(450.0f,10,340.0-500.0);

        Camera.setPos(pos);
        Camera.fw = Vec3f(0.0f,0.0f,1.0f);

    }


    if (timertick == 550)
    {
        controller.controllingid = CONTROLLING_NONE;
        Vec3f pos = islands[0]->getPos();

        pos += Vec3f(-450.0f,10,340.0-500.0);

        Camera.setPos(pos);
        Camera.fw = Vec3f(0.0f,0.0f,1.0f);

    }

    if (timertick == 600)
    {
        CommandCenter *c = (CommandCenter*)islands[0]->getCommandCenter();

        Missile *a = (Missile*)entities[missile];

        if (c && a)
        {
            a->goTo(c->getPos());
            a->enableAuto();
        }

        switchControl(missile);
    }


    if (timertick > 5000)
    {
        isdone = true;
        haspassed = true;
        message = std::string("Completed....");
    }

    return 0;
}

int TestCase_113::number()
{
    return 113;

}

std::string TestCase_113::title()
{
    return std::string("Picking up targets.");
}


bool TestCase_113::done()
{
    return isdone;
}
bool TestCase_113::passed()
{
    return haspassed;
}
std::string TestCase_113::failedMessage()
{
    return message;
}


// -----------
TestCase *pickTestCase(int testcase)
{
    return new TestCase_113();
}

/* ============================================================================
**
** Scenario 111
**
** This is 3D port of Java RoboCode platform.
**
** There are two (it could be more) walrus tanks.  They are configured
** to transmit telemetry information to whoever is subscribed to receive them
**
** ========================================================================= */

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


#include "testcase_111.h"


extern unsigned long timer;

extern  Controller controller;
extern container<Vehicle*> entities;
extern std::vector<BoxIsland*> islands;
extern dWorldID world;
extern dSpaceID space;
extern int testing;
extern  Camera Camera;
extern int  aiplayer;

typedef struct sockaddr SA;

TestCase_111::TestCase_111()
{

}

void TestCase_111::init()
{

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    fcntl(sockfd, F_SETFL, O_NONBLOCK);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(4501);

    bind(sockfd, (SA *) &servaddr, sizeof(servaddr));

    /**
    socklen_t len;
    ControlStructure mesg;

    SA pcliaddr;

    socklen_t clilen=sizeof(cliaddr);
    int n;

    for (;;) {
        len = clilen;
        n = recvfrom(sockfd, &mesg, sizeof(mesg), 0, &pcliaddr, &len);

        if (n!=-1)
        {
            sendto(sockfd, &mesg, n, 0, &pcliaddr, len);

            printf("%d:roll %10.2f thrust %10.2f\n", mesg.id, mesg.roll, mesg.thrust);
        }
    }
    **/



    BoxIsland *nemesis = new BoxIsland(&entities);
    nemesis->setName("Atom");
    nemesis->setLocation(0.0f,-1.0,0.0f);
    nemesis->buildTerrainModel(space,"terrain/thermopilae.bmp");

    islands.push_back(nemesis);

    {
        // 6,3,12
        Otter *_otter = new Otter(GREEN_FACTION);
        _otter->init();
        dSpaceID car_space = _otter->embody_in_space(world, space);
        _otter->setPos(400.0f,70.0f,-4400.0f);
        _otter->setPos(40.0f,30.0f,-0.0f);

        _otter->setPos(Vec3f(getRandomInteger(-1200,1200),30.0f,getRandomInteger(-600,600)));
        _otter->stop();
        _otter->setSignal(4);
        _otter->setNameByNumber(1);
        _otter->setStatus(SailingStatus::SAILING);
        _otter->setIsland(nemesis);

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
        dRFromAxisAndAngle(Re2,0.0,1.0,0.0,getRandomInteger((int)-PI/2.0+PI,(int)PI/2.0)/10.0+PI);
        dBodySetRotation(_otter->getBodyID(),Re2);

        //_otter->goTo(Vec3f(0,0,340));
        _otter->enableAuto();
        _otter->enableTelemetry();
    }


    {
        // 6,3,12
        Otter *_otter = new Otter(BLUE_FACTION);
        _otter->init();
        dSpaceID car_space = _otter->embody_in_space(world, space);
        _otter->setPos(400.0f,70.0f,-4400.0f);
        _otter->setPos(40.0f,30.0f,-0.0f);

        _otter->setPos(Vec3f(getRandomInteger(-1200,1200),30.0f,getRandomInteger(-600,600)));
        _otter->stop();
        _otter->setSignal(4);
        _otter->setNameByNumber(2);
        _otter->setStatus(SailingStatus::SAILING);
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
        //dRFromAxisAndAngle(Re2,0.0,1.0,0.0,-PI/4.0);
        dRFromAxisAndAngle(Re2,0.0,1.0,0.0,getRandomInteger((int)-PI/4*10,(int)PI/4*10)/10.0);
        dBodySetRotation(_otter->getBodyID(),Re2);

        //_otter->goTo(Vec3f(0,0,340));
        _otter->enableAuto();
        _otter->enableTelemetry();
    }

//    BoxVehicle * _bo= new BoxVehicle();
//    _bo->init();
//    _bo->embody(world, space);
//    _bo->setPos(0.0,320.0f,-1900.0f);
//    _bo->stop();

//    entities.push_back(_bo, _bo->getGeom());


    //Vec3f pos(0.0,1.32, - 3500);
    Vec3f pos(-10,1.32,10);
    Camera.setPos(pos);

    aiplayer = FREE_AI;
    controller.faction = BOTH_FACTION;

    endtimer = 0;

}

int TestCase_111::check(unsigned long timertick)
{
    if (timertick ==5)
    {
        Vehicle* _b1 = findWalrus(GREEN_FACTION);
        _b1->enableAuto();
        _b1->setStatus(ROLLING);
        Vehicle* _b2 = findWalrus(BLUE_FACTION);
        _b2->enableAuto();
        _b2->setStatus(ROLLING);
    }

    if (timertick > 20)
    {

        socklen_t len;
        ControlStructure mesg;

        SA pcliaddr;

        socklen_t clilen=sizeof(cliaddr);
        int n;

        len = clilen;
        n = recvfrom(sockfd, &mesg, sizeof(mesg), 0, &pcliaddr, &len);

        if (n!=-1)
        {
            //sendto(sockfd, &mesg, n, 0, &pcliaddr, len);

            printf("Delay %lu %d:roll %10.2f thrust %10.2f\n", (timer-mesg.sourcetimer),mesg.controllingid, mesg.registers.roll, mesg.registers.thrust);

            Vehicle *_b=NULL;

            if (mesg.controllingid == 1)
                _b = findWalrus(GREEN_FACTION);
            else if (mesg.controllingid == 2)
                _b = findWalrus(BLUE_FACTION);

            if ((timer-mesg.sourcetimer)>30000)
            {
                printf("Message IGNORED !!!");
            }
            else if (_b)
            {
                Controller co;
                co.controllingid = mesg.controllingid;
                co.faction = mesg.faction;
                co.registers = mesg.registers;
                //co.push(mesg.order);

                _b->doControl(co);


                if (mesg.order.command == Command::FireOrder && _b->getPower()>0)
                {
                    Vehicle *action = _b->fire(0,world, space);

                    if (action != NULL)
                    {
                        entities.push_back(action, action->getGeom());
                        gunshot();
                    }
                    _b->setPower(_b->getPower()-1);
                }

            }
        }

        Vehicle *_b1 = findWalrus(GREEN_FACTION);

        Vehicle *_b2 = findWalrus(BLUE_FACTION);

        if (_b1 && (_b1->getStatus() == SailingStatus::SAILING || _b1->getStatus() == SailingStatus::OFFSHORING))
        {
            _b1->damage(1);
        }

        if (_b2 && (_b2->getStatus() == SailingStatus::SAILING || _b2->getStatus() == SailingStatus::OFFSHORING))
        {
            _b2->damage(1);

        }

        if (!_b1 && endtimer==0)
        {
            endtimer = timertick + 300;
            whowon=2;
        }

        if (!_b2 && endtimer==0)
        {
            endtimer = timertick + 300;
            whowon = 1;
        }


    }

    if (timertick > endtimer && endtimer!=0)
    {
        isdone = true;
        haspassed = true;
        printf("Walrus %d won.", whowon);
    }

    if (timertick > 6000)
    {
        isdone = true;
        haspassed = false;
        message = std::string("Match is over.");

        Vehicle *_b1 = findWalrus(GREEN_FACTION);

        Vehicle *_b2 = findWalrus(BLUE_FACTION);

        if (_b1)
        {
            printf("GREEN 1 : Health:%d\n", _b1->getHealth());
        }

        if (_b2)
        {
            printf("BLUE 2 : Health:%d\n", _b2->getHealth());
        }

    }

    return 0;
}

int TestCase_111::number()
{
    return 111;

}

std::string TestCase_111::title()
{
    return std::string("Combat. Two walruses fighting each other.");
}


bool TestCase_111::done()
{
    return isdone;
}
bool TestCase_111::passed()
{
    return haspassed;
}
std::string TestCase_111::failedMessage()
{
    return message;
}


// -----------
TestCase *pickTestCase(int testcase)
{
    return new TestCase_111();
}

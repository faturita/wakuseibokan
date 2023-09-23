/* ============================================================================
**
** Scenario 111
**
** This is 3D port of Java RoboCode platform.
**
** There are two (there could be more) walrus tanks.  They are configured
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

    sockfd1 = socket(AF_INET, SOCK_DGRAM, 0);
    fcntl(sockfd1, F_SETFL, O_NONBLOCK);

    bzero(&servaddr1, sizeof(servaddr1));
    servaddr1.sin_family = AF_INET;
    servaddr1.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr1.sin_port = htons(4501);

    bind(sockfd1, (SA *) &servaddr1, sizeof(servaddr1));


    sockfd2 = socket(AF_INET, SOCK_DGRAM, 0);
    fcntl(sockfd2, F_SETFL, O_NONBLOCK);

    bzero(&servaddr2, sizeof(servaddr2));
    servaddr2.sin_family = AF_INET;
    servaddr2.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr2.sin_port = htons(4502);

    bind(sockfd2, (SA *) &servaddr2, sizeof(servaddr2));

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

        _otter->setPos(Vec3f(getRandomInteger(-1400,1400),30.0f,getRandomInteger(-1400,1400)));
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

        dRFromAxisAndAngle(Re2,0.0,1.0,0.0,(float)getRandomInteger(  ( (int)-PI/2.0+PI )*100 , ( (int)PI/2.0)/10.0+PI )/100.0    );

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

        _otter->setPos(Vec3f(getRandomInteger(-1400,1400),30.0f,getRandomInteger(-1400,1400)));
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
        dRFromAxisAndAngle(Re2,0.0,1.0,0.0,(float)getRandomInteger(  ( (int)-PI/2.0 )*100 , ( (int)PI/2.0)/10.0 )/100.0    );
        dBodySetRotation(_otter->getBodyID(),Re2);

        //_otter->goTo(Vec3f(0,0,340));
        _otter->enableAuto();
        _otter->enableTelemetry();
    }


    Vec3f pos(-10,1.32,10);
    Camera.setPos(pos);

    aiplayer = FREE_AI;
    controller.faction = BOTH_FACTION;

    endtimer = 0;

}

float distancegreen, distanceblue;

int TestCase_111::check(unsigned long timertick)
{
    if (timertick ==1)
    {
        Vehicle* _b1 = findWalrus(GREEN_FACTION);
        _b1->enableAuto();
        _b1->setStatus(ROLLING);
        Vehicle* _b2 = findWalrus(BLUE_FACTION);
        _b2->enableAuto();
        _b2->setStatus(ROLLING);
    }

    //usleep(1000000.0);

    if (timertick >= 1)
    {

        socklen_t len;
        ControlStructure mesg1, mesg2;
        SA pcliaddr1, pcliaddr2;

        socklen_t clilen1=sizeof(cliaddr1);
        int n1, n2;

        len = clilen1;
        n1 = recvfrom(sockfd1, &mesg1, sizeof(mesg1), 0, &pcliaddr1, &len);


        socklen_t clilen2=sizeof(cliaddr2);

        len = clilen2;
        n2 = recvfrom(sockfd2, &mesg2, sizeof(mesg2), 0, &pcliaddr2, &len);

        if (n1!=-1)
        {
            //printf("[%d] Delay %lu - roll:  %10.2f thrust %10.2f\n",mesg1.controllingid, (timer-mesg1.sourcetimer), mesg1.registers.roll, mesg1.registers.thrust);

            Vehicle *_b=NULL;

            if (mesg1.controllingid == 1)
                _b = findWalrus(GREEN_FACTION);
            else if (mesg1.controllingid == 2)
                _b = findWalrus(BLUE_FACTION);

            if ((timer-mesg1.sourcetimer)>30000)
            {
                printf("Message IGNORED !!!");
            }
            else if (_b)
            {
                Controller co;
                co.controllingid = mesg1.controllingid;
                co.faction = mesg1.faction;
                co.registers = mesg1.registers;
                //co.push(mesg.order);

                _b->doControl(co);


                if (mesg1.order.command == Command::FireOrder && _b->getPower()>0)
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

        if (n2!=-1)
        {
            //sendto(sockfd, &mesg, n, 0, &pcliaddr, len);

            //printf("[%d] Delay %lu - roll:  %10.2f thrust %10.2f\n",mesg2.controllingid, (timer-mesg2.sourcetimer), mesg2.registers.roll, mesg2.registers.thrust);

            Vehicle *_b=NULL;

            if (mesg2.controllingid == 1)
                _b = findWalrus(GREEN_FACTION);
            else if (mesg2.controllingid == 2)
                _b = findWalrus(BLUE_FACTION);

            if ((timer-mesg2.sourcetimer)>30000)
            {
                printf("Message IGNORED !!!");
            }
            else if (_b)
            {
                Controller co;
                co.controllingid = mesg2.controllingid;
                co.faction = mesg2.faction;
                co.registers = mesg2.registers;
                //co.push(mesg.order);

                _b->doControl(co);


                if (mesg2.order.command == Command::FireOrder && _b->getPower()>0)
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

        if (_b1)
        {
            distancegreen += _b1->getSpeed();
        }

        if (_b2)
        {
            distanceblue += _b2->getSpeed();
        }



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
        printf("Walrus %d WON.\n", whowon);
    }

    float fps = getFPS();

    if (fps == 0)  fps=60.0;

    // Only 5 mimutes duration
    if (timertick > 10000)
    {
        isdone = true;
        haspassed = false;
        message = std::string("Match is over.");

        Vehicle *_b1 = findWalrus(GREEN_FACTION);

        Vehicle *_b2 = findWalrus(BLUE_FACTION);

        if (_b1)
        {
            printf("GREEN 1 : Health:%d, Efficiency: %f, Travelled Distance: %f\n", _b1->getHealth(), ((float)_b1->getPower()/(float)_b2->getHealth()), distancegreen/timertick);
        }

        if (_b2)
        {
            printf("BLUE 2 : Health:%d, Efficiency: %f, Travelled Distance: %f\n", _b2->getHealth(), ((float)_b2->getPower()/(float)_b1->getHealth()), distanceblue/timertick);
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

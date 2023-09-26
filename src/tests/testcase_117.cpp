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

typedef struct sockaddr SA;

TestCase_117::TestCase_117()
{

}


void TestCase_117::init()
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

    BoxIsland *nemesis = new BoxIsland(&entities);
    nemesis->setName("Atom");
    nemesis->setLocation(0.0f,-1.0,0.0f);
    nemesis->buildTerrainModel(space,"terrain/track.bmp");

    islands.push_back(nemesis);


    {
        // 6,3,12
        Otter *_otter = new Otter(GREEN_FACTION);
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
        dRFromAxisAndAngle(Re2,0.0,1.0,0.0,PI/2.0);
        //dRFromAxisAndAngle(Re2,0.0,1.0,0.0,getRandomInteger((int)-PI/2.0+PI,(int)PI/2.0)/10.0+PI);
        dBodySetRotation(_otter->getBodyID(),Re2);

        _otter->enableAuto();
        _otter->enableTelemetry();

    }



    //Structure *t1 = islands[0]->addStructure(new CommandCenter(BLUE_FACTION, FACTORY_ISLAND)    ,       200.0f,    -100.0f,0,world);


    //Vec3f pos(0.0,1.32, - 3500);
    Vec3f pos(-10,1.32,10);
    Camera.setPos(pos);

    aiplayer = FREE_AI;
    controller.faction = BOTH_FACTION;

}

float distancegreen, distanceblue;

int TestCase_117::check(unsigned long timertick)
{
    if (timertick ==1)
    {
        Vehicle* _b1 = findWalrus(GREEN_FACTION);
        _b1->enableAuto();
        _b1->setStatus(ROLLING);

    }

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
    }

    Vehicle *_b1 = findWalrus(GREEN_FACTION);

    if (_b1)
    {
        distancegreen += _b1->getSpeed();
    }

    if (_b1 && (_b1->getStatus() == SailingStatus::SAILING || _b1->getStatus() == SailingStatus::OFFSHORING))
    {
        _b1->damage(1);
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

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
#include <sstream>


#include "../container.h"
#include "../networking/telemetry.h"

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
extern  Camera camera;
extern int  aiplayer;
extern int controlmap[];
extern bool episodesmode;

extern std::unordered_map<std::string, GLuint> textures;

typedef struct sockaddr SA;

TestCase_111::TestCase_111()
{
    //std::cout << "sizeof(ControlStructure2): " << sizeof(ControlStructure2) << std::endl;

    //std::cout << "Offsets:\n";
    //std::cout << "controllingid: " << offsetof(ControlStructure2, controllingid) << "\n";
    //std::cout << "registers: " << offsetof(ControlStructure2, registers) << "\n";
    //std::cout << "faction: " << offsetof(ControlStructure2, faction) << "\n";
    //std::cout << "sourcetimer: " << offsetof(ControlStructure2, sourcetimer) << "\n";

}

size_t TestCase_111::addTank(BoxIsland *nemesis, int faction, int walusnumber,GLuint textureId)
{
    // 6,3,12
    Otter *_otter = new Otter(faction);
    _otter->init();
    dSpaceID car_space = _otter->embody_in_space(world, space);

    // Check if the file initlocations.txt exists, if it does open it and read the locations for this particular walrusnumber
    // If it does not exist, create a random location for the walrus

    std::ifstream initlocations("conf/initlocations.txt");
    std::string line;
    
    int wnumber = 0;
    int x = getRandomInteger(-1400,1400);
    int z = getRandomInteger(-1400,1400);
    float ibearing = (float) -getRandomInteger(0, 360)*PI/180.0;

    if (initlocations.is_open())
    {
        while (std::getline(initlocations, line))
        {
            int xx,zz;
            float ibb;
            std::istringstream iss(line);
            iss >> wnumber >> xx  >> zz >> ibb;
            if (wnumber == walusnumber)
            {
                x = xx;
                z = zz;
                ibearing = -ibb*PI/180.0;
                break;
            }
        }
    }
    initlocations.close();


    Vec3f p = Vec3f(x,10.0f,z);
    _otter->setPos(p);
    _otter->stop();
    _otter->setSignal(4);
    _otter->setNameByNumber(walusnumber);
    _otter->setStatus(SailingStatus::SAILING);
    _otter->setIsland(nemesis);
    _otter->setTexture(textureId);
    _otter->enable_heatup = true;
    //_otter->no_damping_on_bullets = true;
    //_otter->firepower = 100.0;

    Vec3f dimensions(5.0f,4.0f,10.0f);

    size_t id = entities.push_back(_otter, _otter->getGeom());



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
    //dRFromAxisAndAngle(Re2,0.0,1.0,0.0,-PI/4.0);

    //dRFromAxisAndAngle(Re2,0.0,1.0,0.0,(float)getRandomInteger(  ( (int)-PI/2.0+PI )*100 , ( (int)PI/2.0)/10.0+PI )/100.0    );

    dRFromAxisAndAngle(Re2,0.0,1.0,0.0,(float) ibearing);

    dBodySetRotation(_otter->getBodyID(),Re2);

    //_otter->goTo(Vec3f(0,0,340));
    _otter->enableAuto();
    _otter->enableTelemetry();

    Wheel *l,*r,*bl,*br;
    _otter->setPos(p);
    _otter->getWheels(l,r,bl,br);
    l->setPos(p);
    r->setPos(p);
    bl->setPos(p);
    br->setPos(p);

    return id;
}


void TestCase_111::reset(BoxIsland *nemesis)
{
    GLuint textureId[] = { textures["sky"], textures["metal"], textures["sea"], textures["land"], textures["road"] };
    size_t val ;

    assert(iendpoints <= 5 && iendpoints > 0 || !"Do not support more than 5 walruses.");

    for(int i=0;i<iendpoints;i++)
    {
        val= addTank(nemesis, i+1, i+1, textureId[i]);controlmap[i] = val;
        healthes.push_back(1000);
        powers.push_back(1000);
        distances.push_back(0);
    }

    controller.controllingid = val;
}

void TestCase_111::init()
{
    iendpoints=pickendpoint();

    for(int i=0;i<iendpoints;i++)
    {
        sockadders.push_back(socketaddr());
        sockadders[i].sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        fcntl(sockadders[i].sockfd, F_SETFL, O_NONBLOCK);

        bzero(&sockadders[i].servaddr, sizeof(sockadders[i].servaddr));
        sockadders[i].servaddr.sin_family = AF_INET;
        sockadders[i].servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
        sockadders[i].servaddr.sin_port = htons(4501+i);

        bind(sockadders[i].sockfd, (SA *) &sockadders[i].servaddr, sizeof(sockadders[i].servaddr));
    }

    BoxIsland *nemesis = new BoxIsland(&entities);
    nemesis->setName("Atom");
    nemesis->setLocation(0.0f,-1.0,0.0f);
    nemesis->buildTerrainModel(space,"terrain/thermopilae.bmp");

    islands.push_back(nemesis);

    reset(nemesis);

    // Set up the camera view

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

void TestCase_111::cleanall()
{
    for(int i=0;i<iendpoints;i++)
    {
        Vehicle *_b=NULL;
        size_t p;

        _b = findWalrusByFactionAndNumber(p, i+1, i+1);

        if (_b)
        {
            _b->damage(100000);
        }
        distances[i] = 0;
    }
    timer = 0;
    dout << "Cleaning up sceneario to start it over again." << std::endl;
    BoxIsland *nemesis = findNearestIsland(Vec3f(0,0,0));
    reset(nemesis);
}

void TestCase_111::checkBeforeDone(unsigned long timertick)
{
    for(int i=0;i<iendpoints;i++)    
    {
        float distance = distances[i];
        float health = healthes[i];
        float power = powers[i];

        // ODE Simulation wraps one real time step in 20 ticks.
        printf("Faction: %d, Walrus %d : Health:%8.2f, Power: %8.2f, Travelled Distance: %8.2f\n", i+1, i+1, health, power, distance/20.0);
    }   
}

int TestCase_111::check(unsigned long timertick)
{
    if (timertick ==1)
    {
        for(int i=0;i<iendpoints;i++)    
        {
            size_t p;
            Vehicle* _b = findWalrusByFactionAndNumber(p, i+1, i+1);
            if (_b)
            {
                _b->enableAuto();
                _b->setStatus(ROLLING);
            }
        }
    }

//    //usleep(1000000.0);

    if (timertick >= 1)
    {

        for(int i=0;i<iendpoints;i++)
        {
            socklen_t len;
            ControlStructure2 mesg;
            SA pcliaddr;
            struct sockaddr_in cliaddr;

            socklen_t clilen=sizeof(cliaddr);
            int n;
            len = clilen;
            n = recvfrom(sockadders[i].sockfd, &mesg, sizeof(ControlStructure2), 0, &pcliaddr, &len);

            //printf("Received %d %d bytes from %s:%d\n",sizeof(ControlStructure2), n, inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));



            if (n!=-1)
            {
                //printf("Delay %ld %d\n",timer-mesg.sourcetimer, mesg.command);

                Vehicle *_b=NULL;
                size_t p;

                _b = findWalrusByFactionAndNumber(p, mesg.faction, mesg.controllingid);

                //printf("Received Command Message from %d:%d\n", mesg.faction, mesg.controllingid);

                if ((timer-mesg.sourcetimer)>30000)
                {
                    //printf("Received Command Message is too old, IGNORED !!!\n");
                } 
                else if (_b)
                {
                    Controller co;
                    co.controllingid = mesg.controllingid;
                    co.faction = mesg.faction;

                    co.registers.thrust = mesg.registers.thrust;
                    co.registers.roll = mesg.registers.roll;
                    co.registers.pitch = mesg.registers.pitch;
                    co.registers.yaw = mesg.registers.yaw;
                    co.registers.precesion = mesg.registers.precesion;
                    co.registers.bank = mesg.registers.bank;

                    //co.push(mesg.order);

                    _b->doControl(co);


                    if (mesg.command == 11 && _b->getPower()>0)
                    {
                        Vehicle *action = _b->fire(0,world, space);

                        if (action != NULL)
                        {
                            entities.push_back(action, action->getGeom());
                            gunshot(_b->getPos());
                            _b->setPower(_b->getPower()-1);
                        }
                    }   
                }
            } 
        }

        for(int i=0;i<iendpoints;i++)    
        {
            size_t p;
            Vehicle* _b = findWalrusByFactionAndNumber(p, i+1, i+1);
            if (_b)
            {
                distances[i] += _b->getSpeed();
                healthes[i] = clipped(_b->getHealth(),1,1000);
                powers[i] = _b->getPower();

                if (_b && (_b->getStatus() == SailingStatus::SAILING || _b->getStatus() == SailingStatus::OFFSHORING))
                {
                    _b->damage(1);
                }
            }
            else
            {
                if (endtimer == 0 || endtimer == DEFAULT_MATCH_DURATION)
                    endtimer = timertick + 300;
            }
        }
    }

    if (timertick > endtimer && endtimer!=0)
    {
        checkBeforeDone(timertick);
        if (!episodesmode)
        {
            isdone = true;
            haspassed = true;

            if (endtimer == DEFAULT_MATCH_DURATION)
            {
                message = std::string("Match is over.");
                haspassed = false;
            }
        }
        else
        {
            cleanall();
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
    return std::string("Combat.  Walruses fighting each other.");
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

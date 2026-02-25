#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "../container.h"
#include "../terrain/Terrain.h"
#include "../units/Vehicle.h"
#include "../units/Seal.h"
#include "../usercontrols.h"
#include "../camera.h"
#include "../engine.h"

#include "testcase_130.h"

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

// Seal starts heading north (+Z), destination is 90° to the right (+X).
// This forces the PID heading controller to make a clean 90° turn.
static Vec3f sealStart      (   0.0f, 1.0f,    0.0f);
static Vec3f sealDestination(3000.0f, 0.0f,    0.0f);

static size_t sealIdx = 0;

// For oscillation counting
static float  prevErrorSign = 0.0f;
static int    zeroCrossings = 0;
static float  maxOvershoot  = 0.0f;   // peak |error| after first crossing

// ---------------------------------------------------------------------
TestCase_130::TestCase_130() {}

void TestCase_130::init()
{
    sealIdx       = 0;
    prevErrorSign = 0.0f;
    zeroCrossings = 0;
    maxOvershoot  = 0.0f;

    // No island — pure heading / PID test

    Seal *seal = new Seal(GREEN_FACTION);
    seal->init();
    seal->embody(world, space);
    seal->setPos(sealStart[0], sealStart[1], sealStart[2]);
    seal->setNameByNumber(1);
    seal->setSignal(4);
    seal->setStatus(SailingStatus::SAILING);
    seal->stop();

    sealIdx = entities.push_back(seal, seal->getGeom());

    printf("  [init] Seal at (%.0f,%.0f,%.0f)  dest (%.0f,%.0f,%.0f)\n",
           sealStart[0], sealStart[1], sealStart[2],
           sealDestination[0], sealDestination[1], sealDestination[2]);

    // Top-down camera
    Vec3f pos(0.0f, 5000.0f, 0.0f);
    camera.setPos(pos);
    camera.xAngle = -90;  camera.yAngle = 0;
    camera.dy = 0;        camera.dz = 0;
    controller.controllingid = CONTROLLING_NONE;
}

// ---------------------------------------------------------------------
int TestCase_130::check(unsigned long timertick)
{
    Seal *seal = NULL;
    if (entities.hasMore(sealIdx))
    {
        Vehicle *v = entities[sealIdx];
        if (v && v->getType() == WALRUS)
            seal = (Seal*)v;
    }

    if (!seal)
    {
        isdone    = true;
        haspassed = false;
        message   = std::string("Seal was destroyed or not found.");
        return 0;
    }

    // Kick off navigation at tick 10
    if (timertick == 10)
    {
        seal->goTo(sealDestination);
        seal->enableAuto();
        printf("  [tick 10] Navigation started to (%.0f,%.0f,%.0f)\n",
               sealDestination[0], sealDestination[1], sealDestination[2]);
    }

    Vec3f pos        = seal->getPos();
    Vec3f fwd        = seal->getForward();
    Vec3f toTarget   = (sealDestination - pos);
    float distToDest = toTarget.magnitude();

    // Heading error in degrees (signed)
    float headingError = 0.0f;
    if (distToDest > 10.0f)
    {
        Vec3f T = toTarget; T[1] = 0.0f; T = T.normalize();
        Vec3f F = fwd;      F[1] = 0.0f; F = F.normalize();
        float cosA   = T.dot(F);
        if (cosA >  1.0f) cosA =  1.0f;
        if (cosA < -1.0f) cosA = -1.0f;
        float angle  = acos(cosA) * 180.0f / M_PI;
        float signA  = T.cross(F)[1];
        headingError = angle * (signA > 0 ? +1.0f : -1.0f);
    }

    // Count zero-crossings of heading error (oscillation metric)
    if (timertick > 10)
    {
        float curSign = (headingError >= 0.0f) ? +1.0f : -1.0f;
        if (prevErrorSign != 0.0f && curSign != prevErrorSign)
        {
            zeroCrossings++;
            if (zeroCrossings >= 2)   // after the initial turn
                if (fabs(headingError) > maxOvershoot)
                    maxOvershoot = fabs(headingError);
        }
        prevErrorSign = curSign;
    }

    // Periodic log: position, heading error, zero crossings
    if (timertick % 100 == 0 && timertick >= 10)
    {
        printf("  [tick %4lu] pos=(%6.0f,%4.0f,%6.0f)  dist=%6.0f  err=%+7.2f°  xings=%d\n",
               timertick, pos[0], pos[1], pos[2],
               distToDest, headingError, zeroCrossings);
    }

    // Success: reached destination
    if (distToDest < 400.0f)
    {
        printf("PASS: Seal reached destination at tick %lu. Oscillations(zero-crossings)=%d  maxOvershoot=%.2f°\n",
               timertick, zeroCrossings, maxOvershoot);
        isdone    = true;
        haspassed = true;
        return 0;
    }

    // Timeout
    if (timertick > 5000)
    {
        printf("FAIL: timeout at tick %lu. pos=(%.0f,%.0f,%.0f) dist=%.0f  err=%.2f°  xings=%d\n",
               timertick, pos[0], pos[1], pos[2], distToDest, headingError, zeroCrossings);
        isdone    = true;
        haspassed = false;
        message   = std::string("Timeout: Seal did not reach the destination.");
    }

    return 0;
}

int TestCase_130::number()               { return 130; }
std::string TestCase_130::title()        { return std::string("Seal PID heading controller — 90-degree turn, no island."); }
bool TestCase_130::done()                { return isdone; }
bool TestCase_130::passed()              { return haspassed; }
std::string TestCase_130::failedMessage(){ return message; }

// -----------
TestCase *pickTestCase(int testcase)
{
    return new TestCase_130();
}

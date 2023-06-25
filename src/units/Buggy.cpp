//
//  Buggy.cpp
//  wakuseibokan
//
//  Created by Rodrigo Ramele on 6/26/15.
//

#include "Buggy.h"
#include "Wheel.h"
#include "../profiling.h"
#include "../md2model.h"

Buggy::Buggy(int faction)
{
    setFaction(faction);
}

void Buggy::init()
{
    width=6.0f;
    height=3.0f;
    length=12.0f;
    
    setForward(0,0,1);

    
}

int Buggy::getType()
{
    return WALRUS;
}


EntityTypeId Buggy::getTypeId()
{
    return EntityTypeId::TBuggy;
}

void Buggy::drawModel(float yRot, float xRot, float x, float y, float z)
{
    float f[3];
    f[0] = 0; f[1] = 0; f[2] = 0;
    
    glPushMatrix();
    glTranslatef(x, y, z);

    doTransform(f, R);

    drawRectangularBox(width, height, length);

    glPopMatrix();

}

void Buggy::drawModel()
{
    drawModel(0,0,pos[0],pos[1],pos[2]);
}

void Buggy::doControl(Controller controller)
{
    // @TODO: Add a safeguard for nullpointer.
    left->setThrottle(controller.registers.thrust);
    right->setThrottle(controller.registers.thrust);
    backright->setThrottle(controller.registers.thrust);
    backleft->setThrottle(controller.registers.thrust);

    if (controller.registers.thrust>0)
    {

        left->setAzimuth(controller.registers.roll/controller.registers.thrust);
        right->setAzimuth(controller.registers.roll/controller.registers.thrust);
    }

    backleft->setAzimuth(0);
    backright->setAzimuth(0);
}


void Buggy::doControl()
{

}


void Buggy::doControl(struct controlregister conts)
{

}


void Buggy::doDynamics()
{
    doDynamics(getBodyID());
}




void Buggy::doDynamics(dBodyID body)
{
    wrapDynamics(me);
    
}

void Buggy::embody(dWorldID world, dSpaceID space)
{
    me = dBodyCreate(world);
    embody(me);
    geom = dCreateBox( space, width, height, length);
    dGeomSetBody(geom, me);
}

void Buggy::embody(dBodyID myBodySelf)
{
    dMass m;

    float myMass = 1.0f;

    dBodySetPosition(myBodySelf, pos[0], pos[1], pos[2]);
    dMassSetBox(&m,1,width, height, length);
    dMassAdjust(&m, myMass*1.0f);
    dBodySetMass(myBodySelf,&m);

    me = myBodySelf;
}

void Buggy::addWheels(Wheel *left, Wheel *right, Wheel *backleft, Wheel *backright)
{
    Buggy::left = left;
    Buggy::right = right;

    Buggy::backleft = backleft;
    Buggy::backright = backright;
}


#include <unordered_map>
#include "../ThreeMaxLoader.h"
#include "Otter.h"

extern std::unordered_map<std::string, GLuint> textures;
extern container<Vehicle*> entities;

Otter::Otter(int newfaction) : AdvancedWalrus(newfaction)
{

}

void Otter::init()
{
    AdvancedWalrus::init();
    width=6.0f;
    height=3.0f;
    length=12.0f;

    setForward(0,0,1);


}

void Otter::embody(dWorldID world, dSpaceID space)
{
    me = dBodyCreate(world);
    embody(me);
    geom = dCreateBox( space, width, height, length);
    dGeomSetBody(geom, me);
}

void Otter::embody(dBodyID myBodySelf)
{
    dMass m;

    float myMass = 1.0f;

    dBodySetPosition(myBodySelf, pos[0], pos[1], pos[2]);
    dMassSetBox(&m,1,width, height, length);
    dMassAdjust(&m, myMass*1.0f);
    dBodySetMass(myBodySelf,&m);

    me = myBodySelf;
}

dSpaceID Otter::embody_in_space(dWorldID world, dSpaceID space)
{
    body_space = dSimpleSpaceCreate (space);
    dSpaceSetCleanup (body_space,0);

    embody(world, body_space);

    return body_space;
}

void Otter::addWheels(Wheel *left, Wheel *right, Wheel *backleft, Wheel *backright)
{
    Otter::left = left;
    Otter::right = right;

    Otter::backleft = backleft;
    Otter::backright = backright;
}


void Otter::doControl(Controller controller)
{
    if (status == SailingStatus::ROLLING)
    {
        azimuth = controller.registers.precesion;
        elevation = controller.registers.pitch;

        setAim(toVectorInFixedSystem(0,0,1,azimuth, -elevation));

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
    } else {
        AdvancedWalrus::doControl(controller);
    }
}

void Otter::doDynamics()
{
    doDynamics(getBodyID());
}




void Otter::doDynamics(dBodyID body)
{
    if (status == SailingStatus::ROLLING)
        wrapDynamics(me);
    else
        AdvancedWalrus::doDynamics(body);

}

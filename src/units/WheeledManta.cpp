#include <unordered_map>
#include "../ThreeMaxLoader.h"
#include "WheeledManta.h"

extern std::unordered_map<std::string, GLuint> textures;

WheeledManta::WheeledManta(int newfaction) : AdvancedManta(newfaction)
{

}


void WheeledManta::addWheels(Wheel *left, Wheel *right, Wheel *backleft, Wheel *backright)
{
    WheeledManta::left = left;
    WheeledManta::right = right;

    WheeledManta::backleft = backleft;
    WheeledManta::backright = backright;
}


void WheeledManta::doControl(Controller controller)
{
        setThrottle(controller.registers.thrust);
    if (!fly)
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
    } else

    AdvancedManta::doControl(controller);
}

void WheeledManta::doDynamics()
{
    doDynamics(getBodyID());
}

EntityTypeId WheeledManta::getTypeId()
{
    return EntityTypeId::TWheeledManta;
}


void WheeledManta::doDynamics(dBodyID body)
{
    if (fly || speed > 30)
    {AdvancedManta::doDynamics(me);fly=true;}
    else
        wrapDynamics(me);
}



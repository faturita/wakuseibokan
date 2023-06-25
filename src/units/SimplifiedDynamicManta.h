#ifndef SIMPLIFIEDDYNAMICMANTA_H
#define SIMPLIFIEDDYNAMICMANTA_H

#include "Manta.h"

#include "../container.h"

class SimplifiedDynamicManta : public Manta
{
protected:
    Vec3f waypoint;
    int flyingstate;

    std::vector<std::vector<float> > errserie;
    float et1=0,et2=0, et3=0;
    float ett1=0, ett2=0, ett3=0;
    float r1=0, r2=0, r3=0;
    float rt1=0, rt2=0, rt3=0;
    float midpointpitch = -5;

public:
    float precission = 200;
    SimplifiedDynamicManta(int newfaction);
    float angularPos[3] = {0,0,0};
    void doDynamics(dBodyID body);
    void doControl(Controller controller);
    void doControl(struct controlregister cons);
    void doControl();
    void doControlControl(Vec3f target, float thrust);
    void doControlForced(Vec3f target);
    void doControlFlipping(Vec3f target, float thust);
    void doControlControl2(Vec3f target, float thrust, float sp_height, float threshold=1000);
    void doControlWaypoint(float threshold=1000);
    void doHold(Vec3f target, float thrust);

    void doControlDestination();
    void doControlLanding();
    void doControlAttack();
    void doControlDogFight();

    void embody(dBodyID myBodySelf);
    void embody(dWorldID world, dSpaceID space);

    void release(Vec3f orientation);

    void setOrientation(Vec3f orientation);
    void virtual enableAuto();


    Vehicle* fire(int weapon, dWorldID world, dSpaceID space);
    Vehicle* fireAmmo(dWorldID world, dSpaceID space);
    Vehicle* fireBomb(dWorldID world, dSpaceID space);
    Vehicle* fireMissile(dWorldID world, dSpaceID space);
    Vehicle* fireAAM(dWorldID world, dSpaceID space);
    Vehicle* fireTorpedo(dWorldID world, dSpaceID space);
    void flyingCoefficients(float &Cd, float &CL, float &Cm, float &Cl, float &Cy, float &Cn);
    void rotateBody(dBodyID);

    void land(Vec3f landplace, Vec3f landattitude);
    void attack(Vec3f target);
    void dogfight(Vec3f target);

    void hold();

    virtual EntityTypeId getTypeId();
};

#endif // SIMPLIFIEDMANTA_H

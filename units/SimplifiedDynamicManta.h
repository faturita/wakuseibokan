#ifndef SIMPLIFIEDDYNAMICMANTA_H
#define SIMPLIFIEDDYNAMICMANTA_H

#include "Manta.h"

class SimplifiedDynamicManta : public Manta
{
public:
    float precission = 200;
    SimplifiedDynamicManta(int newfaction);
    float angularPos[3] = {0,0,0};
    void doDynamics(dBodyID body);
    void doControl(Controller controller);
    void doControl(struct controlregister cons);
    void doControl();
    void doControlControl(Vec3f target);
    void doControlForced(Vec3f target);

    void doControlDestination();
    void doControlLanding();
    void doControlAttack();

    void embody(dBodyID myBodySelf);
    void embody(dWorldID world, dSpaceID space);

    void release(Vec3f orientation);

    void setOrientation(Vec3f orientation);



    Vehicle* fire(dWorldID world, dSpaceID space);
    void flyingCoefficients(float &Cd, float &CL, float &Cm, float &Cl, float &Cy, float &Cn);
    void rotateBody(dBodyID);

    void land();
    void attack(Vec3f target);
};

#endif // SIMPLIFIEDMANTA_H

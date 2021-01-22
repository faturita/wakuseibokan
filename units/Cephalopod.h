#ifndef CEPHALOPOD_H
#define CEPHALOPOD_H

#include "SimplifiedDynamicManta.h"

class Cephalopod : public SimplifiedDynamicManta
{
protected:
    float pan;
    float tilt;
    float getVerticalAttitude();
    float getRollAngle();
    float getPitchAngle();
public:
    Cephalopod(int newfaction);

    void  init();

    int virtual getSubType();

    void drawModel(float yRot, float xRot, float x, float y, float z);

    void embody(dBodyID myBodySelf);

    void embody(dWorldID world, dSpaceID space);

    void getViewPort(Vec3f &Up, Vec3f &position, Vec3f &forward);

    void doDynamics();

    void doDynamics(dBodyID body);

    void doControl(controlregister regs);
    Vehicle *fire(dWorldID world, dSpaceID space);
    void doControl();
    void hoover(float sp3);
    void doControlDestination(Vec3f target, float threshold);
    void doControlAttack();
    void doControlLanding();
};

#endif // CEPHALOPOD_H

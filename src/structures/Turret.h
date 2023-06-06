#ifndef TURRET_H
#define TURRET_H

#include "Structure.h"

class Turret : public Structure
{
protected:
    Vec3f firingpos;
public:
    GLfloat modelview[16];

    float zoom;

    Turret(int faction);
    void init();
    void drawModel(float yRot, float xRot, float x, float y, float z);

    void getViewPort(Vec3f &Up, Vec3f &position, Vec3f &forward);
    Vec3f getForward();
    void setForward(float x, float y, float z);
    void setForward(Vec3f);

    Vec3f getFiringPort();

    Vehicle* fire(int weapon, dWorldID world, dSpaceID space);

    void virtual doControl(Controller);
    void virtual doControl();

    int virtual getSubType();
    EntityTypeId virtual getTypeId();
};

#endif // TURRET_H

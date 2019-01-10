#ifndef TURRET_H
#define TURRET_H

#include "Structure.h"

class Turret : public Structure
{
public:
    GLfloat modelview[16];

    float zoom;

    Turret();
    void init();
    void drawModel(float yRot, float xRot, float x, float y, float z);

    void getViewPort(Vec3f &Up, Vec3f &position, Vec3f &forward);
    Vec3f getForward();

    Vehicle* fire(dWorldID world, dSpaceID space);

    void doControl(Controller);
};

#endif // TURRET_H

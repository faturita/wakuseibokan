#ifndef ARTILLERY_H
#define ARTILLERY_H

#include "Structure.h"

class Artillery : public Structure
{
public:
    GLfloat modelview[16];

    float zoom;

    Artillery(int faction);
    void init();
    void drawModel(float yRot, float xRot, float x, float y, float z);

    void getViewPort(Vec3f &Up, Vec3f &position, Vec3f &forward);
    Vec3f getForward();

    Vehicle* fire(dWorldID world, dSpaceID space);

    void virtual doControl(Controller);
    void virtual doControl();
};

#endif // ARTILLERY_H

#ifndef ARTILLERY_H
#define ARTILLERY_H

#include "Structure.h"

class Artillery : public Structure
{
protected:
    Vec3f firingpos;
public:
    GLfloat modelview[16];

    float zoom;

    Artillery(int faction);
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

    int getSubType() override;
    EntityTypeId virtual getTypeId();
};

#endif // ARTILLERY_H

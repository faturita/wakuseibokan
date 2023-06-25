#ifndef LASERRAY_H
#define LASERRAY_H

#include "../units/Vehicle.h"

class LaserRay : public Vehicle
{
public:
    LaserRay();
    ~LaserRay();
    void init();
    void drawModel(float yRot, float xRot, float x, float y, float z);
    void drawModel();
    void doDynamics(dBodyID body);

    void embody(dWorldID world, dSpaceID space);
    void embody(dBodyID myBodySelf);

    void setPos(float x, float y, float z);
    void setPos(const Vec3f &newpos);

    int getType();
    EntityTypeId virtual getTypeId();

    void disable();
};

#endif // LASERRAY_H

#ifndef STINGRAY_H
#define STINGRAY_H

#include "SimplifiedDynamicManta.h"

class Stingray : public SimplifiedDynamicManta
{

public:
    Stingray(int newfaction);

    void  init();

    int virtual getSubType();
    EntityTypeId virtual getTypeId();

    void drawModel(float yRot, float xRot, float x, float y, float z);

    void embody(dWorldID world, dSpaceID space);
    void embody(dBodyID myBodySelf);
    void getViewPort(Vec3f &Up, Vec3f &position, Vec3f &forward);

    void setNameByNumber(int number);
};

#endif // STINGRAY_H

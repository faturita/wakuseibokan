#ifndef ADVANCEDMANTA_H
#define ADVANCEDMANTA_H

#include "SimplifiedDynamicManta.h"

class AdvancedManta : public SimplifiedDynamicManta
{

public:
    AdvancedManta(int newfaction);

    void  init();

    virtual int getSubType();
    virtual EntityTypeId getTypeId();

    void drawModel(float yRot, float xRot, float x, float y, float z);

    void embody(dWorldID world, dSpaceID space);
    void embody(dBodyID myBodySelf);
    void getViewPort(Vec3f &Up, Vec3f &position, Vec3f &forward);
};

#endif // ADVANCEDMANTA_H

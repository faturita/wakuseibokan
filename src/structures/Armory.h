#ifndef ARMORY_H
#define ARMORY_H

#include "Structure.h"

class Armory : public Structure
{
public:
    Armory(int faction);
    void init();
    void drawModel(float yRot, float xRot, float x, float y, float z);

    int getSubType() override;
    EntityTypeId virtual getTypeId();

    bool checkHeightOffset(int heightOffset) override;
    void getViewPort(Vec3f &Up, Vec3f &position, Vec3f &fwd);
    Vehicle* spawn(dWorldID  world,dSpaceID space,int type, int number);
};

#endif // ARMORY_H

#ifndef DOCK_H
#define DOCK_H

#include "Structure.h"

class Dock : public Structure
{
public:
    Dock(int faction);
    void init();
    void drawModel(float yRot, float xRot, float x, float y, float z);

    int getSubType() override;
    EntityTypeId virtual getTypeId();

    bool checkHeightOffset(int heightOffset) override;
    void getViewPort(Vec3f &Up, Vec3f &position, Vec3f &fwd);
    Vehicle* spawn(dWorldID  world,dSpaceID space,int type, int number);
};

#endif // DOCK_H

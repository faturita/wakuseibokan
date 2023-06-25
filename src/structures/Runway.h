#ifndef RUNWAY_H
#define RUNWAY_H

#include <vector>

#include "Structure.h"
#include "../units/Manta.h"


class Runway : public Structure
{
public:
    Runway(int faction);
    void init();
    void drawModel(float yRot, float xRot, float x, float y, float z);

    int getType();
    int getSubType() override;
    EntityTypeId virtual getTypeId();

    void taxi(Manta *m);
    void launch(Manta* m);
    Vehicle* spawn(dWorldID  world,dSpaceID space,int type, int number);

    void getViewPort(Vec3f &Up, Vec3f &position, Vec3f &forward);
};

#endif // RUNWAY_H

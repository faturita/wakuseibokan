#ifndef BELUGA_H
#define BELUGA_H

#include "../weapons/Weapon.h"
#include "Balaenidae.h"

class Beluga : public Balaenidae
{

public:
    Beluga(int faction);

    void drawModel(float yRot, float xRot, float x, float y, float z);
    void init();
    void embody(dWorldID world, dSpaceID space);
    void embody(dBodyID myBodySelf);

    int virtual getSubType();
    EntityTypeId virtual getTypeId();

    Vehicle* spawn(dWorldID  world,dSpaceID space,int type, int number);
};

#endif // BELUGA_H

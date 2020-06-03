#ifndef ARTILLERYAMMO_H
#define ARTILLERYAMMO_H

#include "Gunshot.h"

class ArtilleryAmmo : public Gunshot
{
protected:
    float range;
public:
    ArtilleryAmmo();
    ~ArtilleryAmmo();
    void init();
    void drawModel(float yRot, float xRot, float x, float y, float z);
    void drawModel();
    void doDynamics(dBodyID body);

    void embody(dWorldID world, dSpaceID space);
    void embody(dBodyID myBodySelf);

    int getType();
};

#endif // ARTILLERYAMMO_H

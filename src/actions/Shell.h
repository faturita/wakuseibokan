#ifndef SHELL_H
#define SHELL_H

#include "ArtilleryAmmo.h"

class Shell : public Gunshot
{
public:
    Shell();
    ~Shell();
    void init();
    void drawModel(float yRot, float xRot, float x, float y, float z);
    void drawModel();
    void doDynamics(dBodyID body);

    void embody(dWorldID world, dSpaceID space);
    void embody(dBodyID myBodySelf);

    int getType();
    EntityTypeId virtual getTypeId();
};

#endif // SHELL_H

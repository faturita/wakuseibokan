#ifndef GUNSHOT_H
#define GUNSHOT_H

#include "../units/Vehicle.h"

class Gunshot : public Vehicle
{
protected:
    float height;
    float width;
    float length;
public:
    Gunshot();
    ~Gunshot();
    void init();
    void drawModel(float yRot, float xRot, float x, float y, float z);
    void drawModel();
    void doDynamics(dBodyID body);

    void embody(dWorldID world, dSpaceID space);
    void embody(dBodyID myBodySelf);

    int getType();

};

#endif // GUNSHOT_H

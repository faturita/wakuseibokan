#ifndef STRUCTURE_H
#define STRUCTURE_H

#include "../units/Vehicle.h"
#include "../ThreeMaxLoader.h"

class Structure : public Vehicle
{
protected:
    dGeomID geom;
    float height;
    float width;
    float length;
public:
    Structure();
    void init();
    void drawModel();
    void drawModel(float yRot, float xRot, float x, float y, float z);

    void embody(dWorldID world, dSpaceID space);

    void embody(dBodyID myBodySelf);
};

#endif // STRUCTURE_H

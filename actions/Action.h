#ifndef ACTION_H
#define ACTION_H

#include "../units/Vehicle.h"

class Action : public Vehicle
{
protected:
    dGeomID geom;
    float height;
    float width;
    float length;
public:
    Action();
    void init();
    void drawModel(float yRot, float xRot, float x, float y, float z);

};

#endif // ACTION_H

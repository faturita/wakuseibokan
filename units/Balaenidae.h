#ifndef BALAENIDAE_H
#define BALAENIDAE_H

#include "Vehicle.h"

class Balaenidae : public Vehicle
{
protected:
    dGeomID geom;
    float rudder;
public:
    Balaenidae();
    void drawModel(float yRot, float xRot, float x, float y, float z);
    void init();
    int getType();
    void embody(dWorldID world, dSpaceID space);
    void embody(dBodyID myBodySelf);

    void doControl(Controller controller);

    void getViewPort(Vec3f &Up, Vec3f &position, Vec3f &forward);
    void doDynamics();


    void doDynamics(dBodyID body);

    void drawModel();
};

#endif // BALAENIDAE_H

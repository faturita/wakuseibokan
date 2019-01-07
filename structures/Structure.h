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

    float azimuth;
    float inclination;

    GLuint texture;
public:
    Structure();
    ~Structure();

    dGeomID getGeom();

    void init();
    void drawModel();
    void drawModel(float yRot, float xRot, float x, float y, float z);

    void embody(dWorldID world, dSpaceID space);

    void embody(dBodyID myBodySelf);


    // Camera
    void doControl(Controller controller);

    void getViewPort(Vec3f &Up, Vec3f &position, Vec3f &forward);

    Vec3f getForward();

    void setTexture(GLuint texture);
};

#endif // STRUCTURE_H

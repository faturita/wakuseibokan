#ifndef STRUCTURE_H
#define STRUCTURE_H

#include "../units/Vehicle.h"
#include "../ThreeMaxLoader.h"
#include "../terrain/island.h"

class Structure : public Vehicle
{
protected:
    float height;
    float width;
    float length;



public:
    float azimuth; // @FIXME
    float inclination;
    Island *island;

    Structure();
    ~Structure();

    void init();
    void drawModel();
    void drawModel(float yRot, float xRot, float x, float y, float z);

    void embody(dWorldID world, dSpaceID space);

    void embody(dBodyID myBodySelf);

    void virtual setPos(const Vec3f &newpos);
    void virtual setPos(float x, float y, float z);

    void  doDynamics(dBodyID);
    void  doDynamics();

    int getType();

    void onIsland(Island* island);

    // Camera
    void doControl(Controller controller);

    void getViewPort(Vec3f &Up, Vec3f &position, Vec3f &forward);

    Vec3f getForward();

    void setTexture(GLuint texture);
};

#endif // STRUCTURE_H

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
    float azimuth;                      // x-y plane angle, starting from north clockwise towards east. in deg
    float elevation;                    // angle from x-y plane positive towards the cenith at z. in deg
    Island *island;

    Structure();
    Structure(int faction);
    ~Structure();

    void init();
    void drawModel();
    void drawModel(float yRot, float xRot, float x, float y, float z);

    void embody(dWorldID world, dSpaceID space);

    void embody(dBodyID myBodySelf);

    void rotate(float yawangle);

    void virtual setPos(const Vec3f &newpos);
    void virtual setPos(float x, float y, float z);

    void  doDynamics(dBodyID);
    void  doDynamics();

    int getType();
    int virtual getSubType();
    EntityTypeId virtual getTypeId();

    void onIsland(Island* island);

    // Camera
    void doControl(Controller controller);

    void getViewPort(Vec3f &Up, Vec3f &position, Vec3f &forward);

    Vec3f getForward();

    void setTexture(GLuint texture);

    bool virtual checkHeightOffset(int heightOffset);

    TickRecord serialize();
};

#endif // STRUCTURE_H

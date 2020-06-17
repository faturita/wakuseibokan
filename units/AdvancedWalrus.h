#ifndef ADVANCEDWALRUS_H
#define ADVANCEDWALRUS_H


#include "Vehicle.h"
#include "Walrus.h"
#include "../terrain/Terrain.h"

class AdvancedWalrus : public Walrus
{
protected:
    Vec3f firingpos;
    Vec3f aim;
public:

    float azimuth = 0;
    float elevation = 0;

    AdvancedWalrus(int faction);

    void virtual init();

    void virtual drawModel(float yRot, float xRot, float x, float y, float z);
    void virtual doDynamics(dBodyID);
    void doDynamics();
    void embody(dWorldID world, dSpaceID space);
    void embody(dBodyID myBodySelf);

    void doMaterial();

    void doControl(Controller controller);
    void doControl();
    void doControl(struct controlregister cons);

    Vec3f getFiringPort();
    void  getViewPort(Vec3f &Up, Vec3f &position, Vec3f &forward);

    void setAim(Vec3f aim);
    Vec3f getAim();

    Vehicle* fire(dWorldID world, dSpaceID space);


};


#endif // ADVANCEDWALRUS_H

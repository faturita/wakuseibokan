#ifndef CARRIERLAUNCHER_H
#define CARRIERLAUNCHER_H

#include "Weapon.h"

class CarrierLauncher : public Weapon
{
    Vec3f firingpos;

    int zoom;

public:
    CarrierLauncher(int faction);
    void  init();

    void  drawModel(float yRot, float xRot, float x, float y, float z);

    void embody(dWorldID world, dSpaceID space);
    void embody(dBodyID myself);
    void  doControl(Controller);
    void doControl();
    void doControl(struct controlregister conts);

    int getSubType();
    EntityTypeId virtual getTypeId();

    Vehicle* fire(int weapon, dWorldID world, dSpaceID space);
    Vehicle* fire(int weapon, dWorldID world, dSpaceID space, int shellloadingtime);
    Vehicle* fireAir(dWorldID world, dSpaceID space);
    Vehicle* fireGround(dWorldID world, dSpaceID space);
    Vehicle* fireWater(dWorldID world, dSpaceID space);

    void ground();
    void air();
    void water();

    //Vehicle* aimAndFire(dWorldID world, dSpaceID space, Vec3f target);

    void setForward(Vec3f forw);
    void setForward(float x, float y, float z);
    Vec3f getForward();

    void getViewPort(Vec3f &Up, Vec3f &position, Vec3f &fw);
    Vec3f getFiringPort();
};

#endif // CARRIERLAUNCHER_H

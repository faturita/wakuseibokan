#ifndef CARRIERTURRET_H
#define CARRIERTURRET_H

#include <iostream>

#include "../units/Vehicle.h"
#include "Weapon.h"

class CarrierTurret : public Weapon
{
    Vec3f firingpos;

    int zoom;
    
public:
    CarrierTurret(int faction);
	void  init();

	void  drawModel(float yRot, float xRot, float x, float y, float z);

    
    void  embody(dBodyID);
    void  embody(dWorldID world, dSpaceID space);
    void  doControl(Controller);
    void  doControl(struct controlregister conts);
    void doControl();

    int getSubType();
    EntityTypeId virtual getTypeId();

    Vehicle* fire(int weapon, dWorldID world, dSpaceID space);
    Vehicle* fire(int weapon, dWorldID world, dSpaceID space, int shellloadingtime);
    Vehicle* aimAndFire(dWorldID world, dSpaceID space, Vec3f target);

    void setForward(Vec3f forw);
    void setForward(float x, float y, float z);
    Vec3f getForward();

    void getViewPort(Vec3f &Up, Vec3f &position, Vec3f &fw);
    Vec3f getFiringPort();
};


#endif // CARRIERTURRET_H

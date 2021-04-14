//
//  Weapon.h
//
//

#ifndef CARRIERTURRET_H
#define CARRIERTURRET_H

#include <iostream>

#include "../units/Vehicle.h"
#include "Weapon.h"

class CarrierTurret : public Weapon
{
    Vec3f firingpos;
    Vec3f aim;

    int zoom;
    
public:
    CarrierTurret(int faction);
	void  init();

	void  drawModel(float yRot, float xRot, float x, float y, float z);

    
    void  embody(dBodyID);
    void  embody(dWorldID world, dSpaceID space);
    void  doControl(Controller);

    void doControl();

    int   getType();
    int getSubType();

    Vehicle* fire(dWorldID world, dSpaceID space);

    void setForward(Vec3f forw);
    void setForward(float x, float y, float z);
    Vec3f getForward();

    void getViewPort(Vec3f &Up, Vec3f &position, Vec3f &fw);
    Vec3f getFiringPort();

    void setAim(Vec3f aim);
    Vec3f getAim();
};


#endif // CARRIERTURRET_H

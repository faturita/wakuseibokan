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
    GLuint _textureBox;
    dGeomID geom;

    dJointID joint;

    Vec3f firingpos;

    int zoom;
    
public:
    CarrierTurret(int faction);
	void  init();
    int   getType();
	void  drawModel(float yRot, float xRot, float x, float y, float z);

    
	void  doDynamics(dBodyID);
    void  doDynamics();
    void attachTo(dWorldID world, Vehicle *attacher, float x, float y, float z);
    void  embody(dBodyID);
    void  embody(dWorldID world, dSpaceID space);
    void  doControl(Controller);
    dBodyID  getBodyID();
    void  stop();

    void doControl();
    int getSubType();

    Vehicle* fire(dWorldID world, dSpaceID space);

    void setForward(Vec3f forw);
    void setForward(float x, float y, float z);
    Vec3f getForward();

    void getViewPort(Vec3f &Up, Vec3f &position, Vec3f &fw);
    Vec3f getFiringPort();
};


#endif // CARRIERTURRET_H

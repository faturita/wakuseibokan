//
//  Weapon.h
//
//

#ifndef WEAPON_H
#define WEAPON_H

#include <iostream>

#include "../units/Vehicle.h"

class Weapon : public Vehicle
{
    GLuint _textureBox;
    dGeomID geom;

    dJointID joint;

protected:
    float height;
    float length;
    float width;

    float azimuth=0;
    float elevation=0;
    
public:
    Weapon(int faction);
	void  init();
    int   getType();
	void  drawModel(float yRot, float xRot, float x, float y, float z);
    
	void drawModel();
    
	void  drawDirectModel();
	void doMaterial();
	void  doDynamics(dBodyID);
    void  doDynamics();
    void attachTo(dWorldID world, Vehicle *attacher, float x, float y, float z);
    void  embody(dBodyID);
    void  embody(dWorldID world, dSpaceID space);
    void  doControl(Controller);
    dBodyID  getBodyID();
    void  stop();
};


#endif // WEAPON_H

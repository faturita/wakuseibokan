#ifndef WEAPON_H
#define WEAPON_H

#include <iostream>

#include "../units/Vehicle.h"

class Weapon : public Vehicle
{
    GLuint _textureBox;

    dJointID joint;

protected:
    float height;
    float length;
    float width;

    float azimuth=0;
    float elevation=0;
    
public:
    Weapon(int faction);
    ~Weapon();
    void init();
    void drawModel(float yRot, float xRot, float x, float y, float z);
	void drawModel();

    void virtual setPos(const Vec3f &newpos);
    void virtual setPos(float x, float y, float z);
    
	void  doDynamics(dBodyID);
    void  doDynamics();

    int getType();
    int virtual getSubType();
    EntityTypeId virtual getTypeId();

    void  attachTo(dWorldID world, Vehicle *attacher, float x, float y, float z);
    void  embody(dBodyID);
    void  embody(dWorldID world, dSpaceID space);

    void  doControl(Controller);

    void getViewPort(Vec3f &Up, Vec3f &position, Vec3f &forward);

    Vec3f virtual getForward();

    dBodyID  getBodyID();
    void  stop();
};


#endif // WEAPON_H

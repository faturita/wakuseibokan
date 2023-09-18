#ifndef WHEEL_H
#define WHEEL_H

#include <iostream>

#include "../units/Vehicle.h"

enum WheelLocations { FRONT_RIGHT=1, FRONT_LEFT=2, BACK_RIGHT = 3, BACK_LEFT = 4};

class Wheel : public Vehicle
{
    GLuint _textureBox;

    dJointID joint;

protected:
    bool steeringwheel = false;

    float height;
    float length;
    float width;

    float azimuth=0;
    float elevation=0;

    float cfm = 0.8;
    float maxtorque = 5.5;

    float odometry = 0;


public:
    Wheel(int faction);
    Wheel(int faction, float cfm, float maxtorque);
    ~Wheel();
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

    void  attachTo(dWorldID world, Vehicle *attacher, Vec3f dimensions, WheelLocations loc);
    void  attachTo(dWorldID world, Vehicle *attacher, float x, float y, float z);
    void  embody(dBodyID);
    void  embody(dWorldID world, dSpaceID space);

    void  doControl(Controller);
    void  doControl();
    void  doControl(struct controlregister conts);

    void getViewPort(Vec3f &Up, Vec3f &position, Vec3f &forward);

    dBodyID  getBodyID();
    void  stop();

    void setSteering(bool steer);

    void setAzimuth(float value);

    void resetOdometry();
    float getOdometry();
};



#endif // WHEEL_H

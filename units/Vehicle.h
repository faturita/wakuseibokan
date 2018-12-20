/*
 * Vehicle.h
 *
 *  Created on: Jan 16, 2011
 *      Author: faturita
 */

#ifndef VEHICLE_H_
#define VEHICLE_H_


#include <stdio.h>
#include "../math/vec3f.h"
#include "../math/yamathutil.h"
#include "../md2model.h"
#include "../openglutils.h"
#include "../odeutils.h"
#include "../observable.h"
#include "../usercontrols.h"


class Vehicle : Observable
{
protected:
	Vec3f pos;
	MD2Model* _model;
	float speed;
	float R[12];
	Vec3f forward;
	float throttle;
    float engine[3];
    float steering;
    
    dBodyID me;
    
public:
	float xRotAngle;
	float yRotAngle;
	Vehicle();
    
    int virtual getType();
    
	void virtual init();
	void setSpeed(float speed);
	float getSpeed();
	void setXRotAngle(float xRotAngle);
	void setYRotAngle(float yRotAngle);
	void setPos(const Vec3f &newpos);
	void setPos(float x, float y, float z);
	Vec3f getPos();
	void setForward(float x, float y, float z);
	Vec3f getForward();
	void virtual drawModel(float yRot, float xRot, float x, float y, float z);
    void virtual drawModel() { assert( 0 || !"Should not be executed."); }
    
	void virtual getViewPort(Vec3f &Up, Vec3f &position, Vec3f &forward);
	void setLocation(float fPos[3], float R[12]);

	void virtual setThrottle(float thorttle);
	float virtual getThrottle();

	void virtual doDynamics(dBodyID);
    void virtual doDynamics();

    void virtual doControl(Controller);
    
    void virtual embody(dBodyID myBodySelf);
    
    dBodyID virtual getBodyID();

    void setVector(float* V, Vec3f v);
    void setVector(float *V, dVector3 v);

    Vec3f dBodyGetLinearVelInBody(dBodyID body);
    Vec3f dBodyGetAngularVelInBody(dBodyID body);
    Vec3f dBodyGetLinearVelVec(dBodyID body);

};

#endif /* VEHICLE_H_ */

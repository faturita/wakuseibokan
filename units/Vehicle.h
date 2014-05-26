/*
 * Vehicle.h
 *
 *  Created on: Jan 16, 2011
 *      Author: faturita
 */

#ifndef VEHICLE_H_
#define VEHICLE_H_


#include <stdio.h>
#include "../carrier/vec3f.h"
#include "../carrier/yamathutil.h"
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
public:
	float xRotAngle;
	float yRotAngle;
	Vehicle();
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
	void virtual drawModel();
	void virtual getViewPort(Vec3f &Up, Vec3f &position, Vec3f &forward);
	void setLocation(float fPos[3], float R[12]);

	void virtual setThrottle(float thorttle);
	float virtual getThrottle();

	void virtual doDynamics(dBodyID);



};

#endif /* VEHICLE_H_ */

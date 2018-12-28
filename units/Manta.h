/*
 * Manta.h
 *
 *  Created on: Jan 16, 2011
 *      Author: faturita
 */

#ifndef MANTA_H_
#define MANTA_H_

#include "Vehicle.h"


class Manta : public Vehicle
{
    dGeomID geom;
    
public:
	float rudder;
	float elevator;
    float aileron;
    float flaps;
    float spoiler;
    float ih;

    float param[10];
    float alpha;
    float beta;

	float S[3];
	float V[3];
    float addd;
	void virtual init();
    int  virtual getType();
	void virtual getViewPort(Vec3f &Up, Vec3f &position, Vec3f &forward);
	void virtual drawModel(float yRot, float xRot, float x, float y, float z);
	void virtual drawModel();
	void virtual drawDirectModel();
	void virtual doDynamics(dBodyID);
    void wrapDynamics(dBodyID body);
    void doDynamics();
    void doControl(Controller controller);
    
    void airspeddrarestoration();
    
    void embody(dWorldID world, dSpaceID space);
    void embody(dBodyID myBodySelf);
};

#endif /* MANTA_H_ */

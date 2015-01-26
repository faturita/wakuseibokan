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
public:
	float rudder;
	float elevator;
	float S[3];
	float V[3];
	void virtual init();
	void virtual getViewPort(Vec3f &Up, Vec3f &position, Vec3f &forward);
	void virtual drawModel(float yRot, float xRot, float x, float y, float z);
	void virtual drawModel();
	void virtual drawDirectModel();
	void virtual doDynamics(dBodyID);
    void doDynamics();
    void doControl(Controller controller);
};

#endif /* MANTA_H_ */

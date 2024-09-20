/*
 * Walrus.h
 *
 *  Created on: Jan 16, 2011
 *      Author: faturita
 */

#ifndef SEAL_H
#define SEAL_H

#include "Vehicle.h"
#include "Walrus.h"
#include "../terrain/Terrain.h"

class Seal : public Walrus
{
protected:

public:
    Seal(int faction);

	void virtual init();

    int  virtual getSubType();
    EntityTypeId virtual getTypeId();
	void virtual drawModel(float yRot, float xRot, float x, float y, float z);

    void  getViewPort(Vec3f &Up, Vec3f &position, Vec3f &forward);

};

#endif /* SEAL_H */

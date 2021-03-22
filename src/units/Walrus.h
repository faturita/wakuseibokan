/*
 * Walrus.h
 *
 *  Created on: Jan 16, 2011
 *      Author: faturita
 */

#ifndef WALRUS_H_
#define WALRUS_H_

#include "Vehicle.h"
#include "../terrain/Terrain.h"

enum SailingStatus { SAILING=0, ROLLING, OFFSHORING, INSHORING};

class Walrus : public Vehicle
{
protected:
    float height;
    float width;
    float length;
    int number;

    BoxIsland *island;

public:
    Walrus(int faction);

	void virtual init();
    int  virtual getType();
    int  virtual getSubType();
	void virtual drawModel(float yRot, float xRot, float x, float y, float z);
	void virtual drawModel();
	void virtual drawDirectModel();
	void doMaterial();
	void virtual doDynamics(dBodyID);
    void doDynamics();

    void virtual doControl(Controller controller);
    void virtual doControl();
    void virtual doControl(struct controlregister cons);

    void virtual doControlAttack();
    void virtual doControlDestination();
    
    void embody(dWorldID world, dSpaceID space);
    void embody(dBodyID myBodySelf);

    Vehicle* fire(dWorldID world, dSpaceID space);
    int getNumber() const;
    void setNumber(int value);

    BoxIsland *getIsland() const;
    void setIsland(BoxIsland *value);

    void virtual setStatus(int status);

    void attack(Vec3f target);
};

#endif /* WALRUS_H_ */

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

    BoxIsland *island=NULL;

public:
    Walrus(int faction);

	void virtual init();
    int  virtual getType();
    int  virtual getSubType();
    EntityTypeId virtual getTypeId();
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

    Vehicle* fire(int weapon, dWorldID world, dSpaceID space);

    BoxIsland *getIsland() const;
    void setIsland(BoxIsland *value);

    void virtual setStatus(int status);

    void attack(Vec3f target);

    void setNameByNumber(int number);

    void  getViewPort(Vec3f &Up, Vec3f &position, Vec3f &forward);
};

#endif /* WALRUS_H_ */

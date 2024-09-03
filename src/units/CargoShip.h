/*
 * CargoShip.h
 *
 *  Created on: Sep 02, 2024
 *      Author: faturita
 */

#ifndef CARGOSHIP_H
#define CARGOSHIP_H

#include "Vehicle.h"
#include "Walrus.h"
#include "../terrain/Terrain.h"

class CargoShip : public Vehicle
{
protected:

    BoxIsland *island=NULL;

public:
    CargoShip(int faction);

	void virtual init();
    int  virtual getType();
    int  virtual getSubType();
    EntityTypeId virtual getTypeId();
	void virtual drawModel(float yRot, float xRot, float x, float y, float z);
	void virtual drawDirectModel();
	void doMaterial();
	void virtual doDynamics(dBodyID);
    void doDynamics();

    void virtual doControl(Controller controller);
    void virtual doControl();
    void virtual doControl(struct controlregister cons);

    void virtual doControlAttack();
    void virtual doControlDestination();
    void virtual doControlDocking();
    
    void embody(dWorldID world, dSpaceID space);
    void embody(dBodyID myBodySelf);

    Vehicle* fire(int weapon, dWorldID world, dSpaceID space);

    void virtual setStatus(int status);
    void attack(Vec3f target);
    void virtual setNameByNumber(int number);

    void  getViewPort(Vec3f &Up, Vec3f &position, Vec3f &forward);
};

#endif /* CARGOSHIP_H */

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

class Walrus : public Vehicle
{
protected:

    BoxIsland *island=NULL;

    // PID state for heading control in doControlDestination()
    float _pid_integral  = 0.0f;
    float _pid_prev_error = 0.0f;

    // When true: approach and beach on the island (invasion mode).
    // When false: navigate around the island using potential fields (avoidance mode).
    bool allowIslandLanding = false;

public:
    Walrus(int faction);

	void virtual init();
    int  virtual getType();
    int  virtual getSubType();
    EntityTypeId virtual getTypeId();
	void virtual drawModel(float yRot, float xRot, float x, float y, float z);
	void virtual drawDirectModel();
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

    bool getLandingMode() const { return allowIslandLanding; }
    void setLandingMode(bool value) { allowIslandLanding = value; }

    void virtual setNameByNumber(int number);

    void  getViewPort(Vec3f &Up, Vec3f &position, Vec3f &forward);
};

#endif /* WALRUS_H_ */

/*
 * Manta.h
 *
 * Remember Mobula Ray
 *
 * Created on: Jan 16, 2011
 *      Author: faturita
 */

#ifndef MANTA_H_
#define MANTA_H_

#include "Vehicle.h"

enum FlyingStatus {IN_CARRIER=0, ON_DECK, LANDED, TACKINGOFF, FLYING, HOLDING, DOCKING};

class Manta : public Vehicle
{

public:
    Manta(int newfaction);

    bool antigravity=false;

    float rudder=0;
    float elevator=0;
    float aileron=0;
    float flaps=0;
    float spoiler=0;
    float ih=0;

    float param[10];
    float alpha=0;
    float beta=0;

    float S[3] = {0.0f, 0.0f, 0.0f};
    float V[3] = {0.0f, 0.0f, 0.0f};
    float addd=0;
	void virtual init();
    int  virtual getType();
    int  virtual getSubType();
	void virtual getViewPort(Vec3f &Up, Vec3f &position, Vec3f &forward);
	void virtual drawModel(float yRot, float xRot, float x, float y, float z);
	void virtual drawModel();
	void virtual drawDirectModel();
	void virtual doDynamics(dBodyID);
    void virtual release(Vec3f orientation);
    void virtual land(Vec3f landplace, Vec3f placeattitude);
    void virtual doHold(Vec3f target, float thrust);
    void virtual attack(Vec3f target);
    void virtual dogfight(Vec3f target);
    void doDynamics();
    void doControl();


    void doControl(Controller controller);


    void doControl(struct controlregister regs);

    
    void airspeddrarestoration();
    
    void embody(dWorldID world, dSpaceID space);
    void embody(dBodyID myBodySelf);

    Vehicle* fire(int weapon, dWorldID world, dSpaceID space);

    void setNameByNumber(int number);

    virtual EntityTypeId getTypeId();

};

#endif /* MANTA_H_ */

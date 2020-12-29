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
#include "../model.h"
#include "../openglutils.h"
#include "../odeutils.h"
#include "../observable.h"
#include "../usercontrols.h"

/**
 * @brief Structures are identified if they type number is greater than or equal to COLLISIONABLE.
 */
enum VehicleTypes { RAY=1, WALRUS=2, MANTA=3, CARRIER=4, ACTION=5, CONTROLABLEACTION = 6 ,COLLISIONABLE=7, LANDINGABLE = 8, CONTROL=9};

enum VehicleSubTypes { BALAENIDAE = 1, BELUGA = 2, SIMPLEWALRUS = 3, ADVANCEDWALRUS = 4, SIMPLEMANTA = 5, MEDUSA = 6, ARTILLERY = 10, COMMANDCENTER = 11, HANGAR = 12, WAREHOUSE = 13, RUNWAY = 14, LASERTURRET = 15, TURRET = 16, LAUNCHER = 17, FACTORY = 18, DOCK = 19, ANTENNA = 20, RADAR = 21, STRUCTURE = 22 };

enum AISTATUS { FREE, DESTINATION, LANDING, ATTACK, DOGFIGHT };


enum ORDERS { ATTACK_ISLAND=1, DEFEND_CARRIER, DEFEND_ISLAND };

#define NUMBERING(m) (m + 1)
#define FACTION(m) ( m == GREEN_FACTION ? "Balaenidae" : "Beluga")

class Vehicle : Observable
{
private:
    int ttl=-1;  //Live for ever.

    int health = 1000;
    int power = 1000;

    int faction=-1;

protected:
    Vec3f pos;
    Model* _model;
    Model* _topModel;
	float speed;
	float R[12];
	Vec3f forward;
    float throttle=0;
    float engine[3];
    float steering;
    Vec3f destination;
    Vec3f attitude;         // This is the set point for forward.

    int signal=3;

    // @FIXME: This is super ugly.
    bool reached=false;
    
    dBodyID me=NULL;
    dGeomID geom=NULL;

    struct controlregister myCopy;

    bool aienable = false;
    int aistatus = FREE;

    int status=0;
    int order=0;

    GLuint texture;

    void setTtl(int ttlvalue);

    void setFaction(int newfaction);
    
public:
    bool inert=false;
    float xRotAngle=0;
    float yRotAngle=0;
	Vehicle();
    ~Vehicle();
    
    int virtual getType();
    int virtual getSubType();
    
	void virtual init();
	void setSpeed(float speed);
	float getSpeed();
	void setXRotAngle(float xRotAngle);
	void setYRotAngle(float yRotAngle);
    void virtual setPos(const Vec3f &newpos);
    void virtual setPos(float x, float y, float z);
	Vec3f getPos();
	void setForward(float x, float y, float z);
    void setForward(Vec3f);
	Vec3f getForward();

    void getR(float retR[12]);
    void setR(float newR[12]);
    void alignToMe(dBodyID fBodyID);

	void virtual drawModel(float yRot, float xRot, float x, float y, float z);
    void virtual drawModel();
    
	void virtual getViewPort(Vec3f &Up, Vec3f &position, Vec3f &forward);
	void setLocation(float fPos[3], float R[12]);

	void virtual setThrottle(float thorttle);
	float virtual getThrottle();
    void virtual upThrottle(float throttle);

    void virtual stop();
    void virtual stop(dBodyID who);

	void virtual doDynamics(dBodyID);
    void virtual doDynamics();

    void virtual doControl(Controller);
    void virtual doControl(struct controlregister);
    void virtual doControl();
    
    void virtual embody(dBodyID myBodySelf);
    void antigravity(dBodyID myBodySelf);

    void wrapDynamics(dBodyID body);
    
    dBodyID virtual getBodyID();
    dGeomID virtual getGeom();

    void setVector(float* V, Vec3f v);
    void setVector(float *V, dVector3 v);

    Vec3f dBodyGetLinearVelInBody(dBodyID body);
    Vec3f dBodyGetAngularVelInBody(dBodyID body);
    Vec3f dBodyGetLinearVelVec(dBodyID body);

    struct controlregister getControlRegisters();
    void setControlRegisters(struct controlregister);

    float getBearing();
    void setDestination(Vec3f target);
    Vec3f getDestination() const;

    void virtual attack(Vec3f target);

    void setAttitude(Vec3f attit);
    Vec3f getAttitude();

    virtual Vehicle* fire(dWorldID world, dSpaceID space);

    virtual int getTtl();
    virtual void tick();

    bool isAuto();
    void enableAuto();
    void disableAuto();

    bool arrived();

    virtual Vehicle* spawn(dWorldID world, dSpaceID space,int type, int number);

    int getStatus() const;
    void setStatus(int value);
    int getHealth() const;
    void damage(int d);
    void setTexture(const GLuint &value);

    const Vec3f map(Vec3f);

    bool VERIFY(Vec3f newpos, dBodyID who);

    int getFaction();
    int getPower() const;
    void setPower(int value);

    Vec3f toWorld(dBodyID body,Vec3f fw);
    Vec3f toBody(dBodyID body, Vec3f fw);
    int getSignal() const;
    void setSignal(int value);
    int getOrder() const;
    void setOrder(int value);
    int getAistatus() const;

    std::string subTypeText(int code);
};

#endif /* VEHICLE_H_ */

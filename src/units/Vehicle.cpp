/*
 * Vehicle.cpp
 *
 * This is the base class of all the entities in this game.
 * C++ is great for game programming because the inherence structure provided by the loosely Object Oriented paradigm that is imprented
 * on this language, is very handy and appropriate for the natural problem of modelling objects.
 *
 *
 *
 *  Created on: Jan 16, 2011
 *      Author: faturita
 */


#include "Vehicle.h"
#include <assert.h>
#include "../profiling.h"
#include "../md2model.h"
#include "../networking/telemetry.h"



std::string Vehicle::subTypeText(int code)
{
    switch (code) {
    case VehicleSubTypes::DOCK:return std::string("Dock");break;
    case VehicleSubTypes::RADAR:return std::string("Radar");break;
    case VehicleSubTypes::BELUGA:return std::string("Beluga Carrier");break;
    case VehicleSubTypes::HANGAR:return std::string("Hangar");break;
    case VehicleSubTypes::MEDUSA:return std::string("Medusa Fighter");break;
    case VehicleSubTypes::RUNWAY:return std::string("Runway");break;
    case VehicleSubTypes::TURRET:return std::string("Gunlet Turret");break;
    case VehicleSubTypes::ANTENNA:return std::string("Comm Antenna");break;
    case VehicleSubTypes::FACTORY:return std::string("Factory");break;
    case VehicleSubTypes::LAUNCHER:return std::string("Missile Launcher");break;
    case VehicleSubTypes::ARTILLERY:return std::string("Artillery");break;
    case VehicleSubTypes::STRUCTURE:return std::string("Basic Structure Building");break;
    case VehicleSubTypes::WAREHOUSE:return std::string("Warehouse");break;
    case VehicleSubTypes::BALAENIDAE:return std::string("Balaenidae Carrier");break;
    case VehicleSubTypes::LASERTURRET:return std::string("Laser Turret");break;
    case VehicleSubTypes::COMMANDCENTER:return std::string("CommandCenter");break;
    default:return std::string("Unit");break;
    }
}

int Vehicle::getStatus() const
{
    return status;
}

void Vehicle::setStatus(int value)
{
    status = value;
}

Vehicle::Vehicle()
{
    for(int i=0;i<12;i++)
    {
        R[i]=0;
	}
	R[0]=R[5]=R[10]=1;

    memset(&registers,0,sizeof(struct controlregister));
    Vehicle::speed = 0;

    pos = Vec3f(0.0f,0.0f,0.0f);

    _model = NULL;
    _topModel = NULL;
    speed = 0.0f;

    forward = Vec3f(0.0f,0.0f,1.0f);
    throttle=0.0f;

    steering=0.0f;

    destination = Vec3f(0.0f, 0.0f, 0.0f);
}

Vehicle::~Vehicle()
{
    // This is the only destructor of the entire entity tree.
    if (me) dBodyDestroy(me);
    if (geom) dGeomDestroy(geom);
    //CLog::Write(CLog::Debug,"Vehicle: Destructor.\n");

    // @FIXME: Risky
    if (_model != NULL) delete _model;
    if (_topModel != NULL) delete _topModel;


}

void Vehicle::getR(float retR[12])
{
    memcpy(retR,R,sizeof(float)*12);
}
void Vehicle::setR(float newR[12])
{
    memcpy(R,newR,sizeof(float)*12);
}

void Vehicle::setXRotAngle(float xRotAngle)
{
	Vehicle::xRotAngle = xRotAngle;
}
void Vehicle::setYRotAngle(float yRotAngle)
{
	Vehicle::yRotAngle = yRotAngle;
}
float Vehicle::getSpeed()
{
	return speed;
}
void Vehicle::setSpeed(float speed)
{
	Vehicle::speed=speed;
}
void Vehicle::setPos(const Vec3f &newpos)
{
	pos[0] = newpos[0];
	pos[1] = newpos[1];
	pos[2] = newpos[2];

    assert( me != NULL || !"Setting position without setting the body!");
    if (me) dBodySetPosition(me, pos[0], pos[1], pos[2]);
}
void Vehicle::setPos(float x, float y, float z)
{
	pos[0] = x;
	pos[1] = y;
	pos[2] = z;

    assert( me != NULL || !"Setting position without setting the body!");
    if (me) dBodySetPosition(me, pos[0], pos[1], pos[2]);
}


void Vehicle::setVelocity(float vx, float vy, float vz)
{
    Vehicle::vel[0] = vx;
    Vehicle::vel[1] = vy;
    Vehicle::vel[2] = vz;
}

void Vehicle::setRotation(float *newR)
{
    setR(newR);

    assert( me != NULL || !"Setting position without setting the body!");
    if (me) dBodySetRotation(me, newR);

    dVector3 result;
    dBodyVectorToWorld(me, 0,0,1,result);       //@NOTE: Every vehicle forward position in their own coordinate system is 0,0,1.
    setForward(result[0],result[1],result[2]);
}


Vec3f Vehicle::getPos()
{
	return pos;
}

void Vehicle::setForward(float x, float y, float z)
{
	forward[0]=x;
	forward[1]=y;
	forward[2]=z;
}
void Vehicle::setForward(Vec3f p)
{
    forward = p;
}

Vec3f Vehicle::getForward()
{
	return forward;
}

void Vehicle::setLocation(float fPos[3], float R[12])
{
	pos[0] = fPos[0];
	pos[1] = fPos[1];
	pos[2] = fPos[2];

	for(int i=0; i<12; i++)
	{
        Vehicle::R[i] =  R[i];
	}
}


bool Vehicle::isAuto()
{
    return Vehicle::aienable;
}
void Vehicle::enableAuto()
{
    Vehicle::aienable = true;
}

void Vehicle::disableAuto()
{
    Vehicle::aienable = false;
}

Vec3f Vehicle::getVelocity()
{
    return vel;
}


Vec3f Vehicle::dBodyGetLinearVelVec(dBodyID body)
{
    dReal *v = (dReal *)dBodyGetLinearVel(body);

    Vec3f vec3fV;
    vec3fV[0]= v[0];vec3fV[1] = v[1]; vec3fV[2] = v[2];

    return vec3fV;
}

Vec3f Vehicle::dBodyGetLinearVelInBody(dBodyID body)
{
    dReal *v = (dReal *)dBodyGetLinearVel(body);

    Vec3f vec3fV;
    vec3fV[0]= v[0];vec3fV[1] = v[1]; vec3fV[2] = v[2];

    dVector3 velResult2;
    dBodyVectorFromWorld (body, vec3fV[0], vec3fV[1], vec3fV[2], velResult2);

    vec3fV[0]= velResult2[0];vec3fV[1] = velResult2[1]; vec3fV[2] = velResult2[2];

    return vec3fV;
}

Vec3f Vehicle::dBodyGetAngularVelInBody(dBodyID body)
{
    const dReal* velResultWorld = dBodyGetAngularVel(body);

    dVector3 velResultBody;
    dBodyVectorFromWorld (body, velResultWorld[0], velResultWorld[1], velResultWorld[2], velResultBody);

    Vec3f vec3fV;
    vec3fV[0]= velResultBody[0];vec3fV[1] = velResultBody[1]; vec3fV[2] = velResultBody[2];

    return vec3fV;
}

void Vehicle::init()
{
	//Load the model
    _model = MD2Model::loadModel("units/walrusgood.md2");
    if (_model != NULL)
        _model->setAnimation("run");
}

void Vehicle::clean()
{
    // For Vehicle the cleanup is performed in the destructor (is the only class).

    // This works for all multiobject objects




}

dSpaceID Vehicle::myspace()
{
    return body_space;
}

int Vehicle::getType()
{
    return 0;
}

int Vehicle::getSubType()
{
    return 0;
}

EntityTypeId Vehicle::getTypeId()
{
    assert(0 || !"Vehicle should not be instantiated.\n");
    return EntityTypeId::TWeapon;
}

void Vehicle::setVector(float* V, dVector3 v)
{
    V[0] = v[0];
    V[1] = v[1];
    V[2] = v[2];
}

void Vehicle::setVector(float* V, Vec3f v)
{
    V[0] = v[0];
    V[1] = v[1];
    V[2] = v[2];
}

void  Vehicle::getViewPort(Vec3f &Up, Vec3f &position, Vec3f &forward)
{
	position = getPos();
	forward = getForward();
	Up = toVectorInFixedSystem(0.0f, 1.0f, 0.0f,0,0);

	Vec3f orig;


	forward = forward.normalize();
	orig = position;
	Up[0]=Up[2]=0;Up[1]=4;
	position = position - 20*forward + Up;
	forward = orig-position;
}

void Vehicle::drawModel(float yRot, float xRot, float x, float y, float z)
{
    //Draw the saved model
    if (_model != NULL)
    {
        glPushMatrix();
        glTranslatef(x, y, z);
		glRotatef(-180.0f, 1.0f, 0.0f, 0.0f);
		glRotatef(-180.0f, 0.0f, 0.0f, 1.0f);

		glRotatef(yRot, 0.0f, 0.0f, 1.0f);

        glRotatef(xRot, 1.0f, 0.0f, 0.0f);

        _model->draw();
        drawArrow();
        glPopMatrix();
    }
    else
    {
    	printf ("model is null\n");
    }
}

void Vehicle::drawModel()
{
    drawModel(0,0,pos[0],pos[1],pos[2]);
}

/**
void Vehicle::drawModel()
{
    assert(0 || !"Draw model should not be executed.\n");
    
    float modX=0.0f, modY=0.0f, modZ=0.0f;

    modX = pos[0];
    modY = pos[1];
    modZ = pos[2];

    Vec3f forward = toVectorInFixedSystem(0, 0, -0.1,-xRotAngle,yRotAngle);

    modX+=speed*forward[0]; modY+=speed*forward[1];modZ+=speed*forward[2];

    pos += speed * forward;

    drawModel(xRotAngle, yRotAngle, modX, modY, modZ);
}**/

void Vehicle::setThrottle(float throttle)
{
    Vehicle::throttle = throttle;
}

void Vehicle::upThrottle(float throttle)
{
    Vehicle::throttle += throttle;
}

float Vehicle::getThrottle()
{
	return Vehicle::throttle;
}

void Vehicle::stop()
{
    stop(me);
}

void Vehicle::stop(dBodyID who)
{
    dBodySetLinearVel(who,0.0f,0.0f,0.0f);
    dBodySetAngularVel(who,0.0f,0.0f,0.0f);
}

void Vehicle::doControl(Controller controller)
{
    //engine[0] = controller.pitch;
    //engine[1] = controller.yaw;
    //engine[2] = controller.roll;
}

void Vehicle::doControl()
{
    assert(0 || !"This method is not implemented.");
}

void Vehicle::doControl(struct controlregister)
{
    assert(0 || !"This method is not implemented.");
}

void Vehicle::setControlRegisters(struct controlregister reg)
{
    Vehicle::registers = reg;
}

void  Vehicle::doDynamics(dBodyID) {
    assert( 0 || !"This should not be executed.");
}
void  Vehicle::doDynamics()
{
    doDynamics(getBodyID());
}

void Vehicle::antigravity(dBodyID myBodySelf)
{
    dBodyAddForce(myBodySelf, 0,9.81f,0);
}

//@FIXME swithc to getAzimuth in yamathutil.
float Vehicle::getBearing()
{
    Vec3f f = (getForward().normalize())*30;

    float val = atan2(f[2], f[0])*180.0/PI;

    if (val>=90) val -= 90;
    else val += 270;

    return val;
}

Vehicle* Vehicle::fire(int weapon, dWorldID world, dSpaceID space)
{
    assert(0 || !"This should not be executed.");
}

Vehicle* Vehicle::spawn(dWorldID world, dSpaceID space,int type, int number)
{
    assert(0 || !"This should not be executed.");
}


void  Vehicle::embody(dBodyID myBodySelf)
{
    dMass m;
    
    float myMass = 1.0f;
    float radius = 2.64f;
    float boxlength = 7.0f;
    
    dBodySetPosition(myBodySelf, pos[0], pos[1], pos[2]);
    //dMassSetBox(&m,1,boxlength,boxlength,boxlength);
    dMassSetSphere(&m,1,radius);
    dMassAdjust(&m, myMass*1.0f);
    dBodySetMass(myBodySelf,&m);
    
    me = myBodySelf;

    assert(0 || !"This should not be executed.");
}

dBodyID Vehicle::getBodyID()
{
    return me;
}

dGeomID Vehicle::getGeom()
{
    return geom;
}

int Vehicle::getTtl()
{
    return Vehicle::ttl;
}

int Vehicle::getHealth() const
{
    return health;
}

void Vehicle::damage(int amount)
{
    Vehicle::health-=amount;
}

void Vehicle::destroy()
{
    damage(getHealth()+1);
}

int Vehicle::getPower() const
{
    return power;
}

void Vehicle::setPower(int value)
{
    power = value;
}

int Vehicle::getSignal() const
{
    return signal;
}

void Vehicle::setSignal(int value)
{
    signal = value;
}

int Vehicle::getOrder() const
{
    return order;
}

void Vehicle::setOrder(int value)
{
    order = value;
}

AutoStatus Vehicle::getAutoStatus() const
{
    return autostatus;
}

void Vehicle::setAutoStatus(AutoStatus au)
{
    autostatus = au;
}

void Vehicle::setTtl(int ttlvalue)
{
    Vehicle::ttl = ttlvalue;
}

void Vehicle::tick()
{
    Vehicle::ttl--;
}

struct controlregister Vehicle::getControlRegisters()
{
    return registers;
}

/**
 * This is the key binding between ODE and OpenGL to make this engine work.
 * Get the speed.
 * Get the forward direction in global coordinates (setForward is in global coordinates).
 * Get the position and rotation from ODE, and set the position with setPos and with setLocation, it updates the R matrix of rotation
 *  that is used by OpenGL to reorient the model.
 *
 * Additionally, the stability of ODE is verified and somehow hacked trying to shift the position or orientation a little bit to avoid the unstability issues.
 *
 * @brief Vehicle::wrapDynamics
 * @param body
 */
void Vehicle::wrapDynamics(dBodyID body)
{
    Vec3f linearVel = dBodyGetLinearVelVec(body);

    // Get speed, alpha and beta.
    speed = linearVel.magnitude();

    dVector3 result;

    dBodyVectorToWorld(body, 0,0,1,result);
    setForward(result[0],result[1],result[2]);

    const dReal *dBodyPosition = dBodyGetPosition(body);
    const dReal *dBodyRotation = dBodyGetRotation(body);

    Vec3f newpos(dBodyPosition[0], dBodyPosition[1], dBodyPosition[2]);

    /**
    if (getType()==WALRUS)
        CLog::Write(CLog::Debug,"Walrus   %p - %10.2f,%10.2f,%10.2f\n", getBodyID(), dBodyPosition[0],dBodyPosition[1],dBodyPosition[2]);

    if (getType()==MANTA)
        CLog::Write(CLog::Debug,"Manta    %p - %10.2f,%10.2f,%10.2f\n", getBodyID(), dBodyPosition[0],dBodyPosition[1],dBodyPosition[2]);

    if (getType()==CARRIER)
        CLog::Write(CLog::Debug,"Carrier  %p - %10.2f,%10.2f,%10.2f\n", getBodyID(), dBodyPosition[0],dBodyPosition[1],dBodyPosition[2]);
    **/

    // @FIXME: This should not be really here.
    if (dotelemetry)
    {

        float *fPos = (float *)dBodyPosition;
        float *R = (float *)dBodyRotation;

//        for (int i=0;i<4;i++)
//        {
//            printf("|");
//            for (int j=0; j<3;j++)
//            {
//                printf ("%10.5f", R[i*4+j]);
//            }
//            printf("|\n");
//        }

//        Vec3f s = Vec3f(fPos[0], fPos[1], fPos[2]);
//        dout << s << std::endl;

        telemetryme(number, health, power, getBearing(), (float *)dBodyPosition, (float *)dBodyRotation);
    }


    // @NOTE:  Keep the following line if you want the game to be more stable.
    if (VERIFY(newpos, body)) {
        setPos(dBodyPosition[0],dBodyPosition[1],dBodyPosition[2]);
        setLocation((float *)dBodyPosition, (float *)dBodyRotation);
        setVelocity(linearVel[0],linearVel[1],linearVel[2]);
    }
}
void Vehicle::alignToMyBody(dBodyID fBodyID)
{
    const dReal *dBodyRotation = dBodyGetRotation(me);
    dBodySetRotation(fBodyID,dBodyRotation);
}

void Vehicle::alignToMe(dBodyID fBodyID)
{
    Vec3f position = getPos();

    //position[1] += 19.0f; // Move upwards to the center of the real rotation.

    forward = getForward();
    Vec3f Up = toVectorInFixedSystem(0.0f, 1.0f, 0.0f,0,0);
    Vec3f orig;


    forward = forward.normalize();
    orig = position;
    position = position + 40*forward;
    forward = -orig+position;


    Vec3f f1(0.0,0.0,1.0);
    Vec3f f2 = forward.cross(f1);
    f2 = f2.normalize();
    float alpha = _acos( forward.dot(f1)/(f1.magnitude()*forward.magnitude()));

    dMatrix3 Re;
    dRSetIdentity(Re);
    dRFromAxisAndAngle(Re,f2[0],f2[1],f2[2],-alpha);

    dBodySetRotation(fBodyID,Re);
}

void Vehicle::setFaction(int newfaction)
{
    faction = newfaction;
}

int Vehicle::getFaction()
{
    return faction;
}

void Vehicle::goTo(Vec3f dest)
{
    Vehicle::destination = dest;
    dst_status = DestinationStatus::TRAVELLING;
    autostatus = AutoStatus::DESTINATION;
}


void Vehicle::setDestination(Vec3f dest)
{
    Vehicle::destination = dest;
}

void Vehicle::goWaypoints()
{
    dst_status = DestinationStatus::STILL;
    autostatus = AutoStatus::WAYPOINT;
}

void Vehicle::addWaypoint(Vec3f waypoint)
{
    waypoints.push(waypoint);
}

void Vehicle::clearWaypoints()
{
    while (!waypoints.empty())
        waypoints.pop();
}

Vec3f Vehicle::getDestination() const
{
    return Vehicle::destination;
}

void Vehicle::attack(Vec3f target)
{
    assert(!"Not implemented.");
}

void Vehicle::setAttitude(Vec3f attit)
{
    Vehicle::attitude = attit;

}

Vec3f Vehicle::getAttitude()
{
    return Vehicle::attitude;
}

/**
 * Check model consistencies.
 *
 * ODE sometimes just blow away.  Try to catch empirically those situations here.
 *
 * @brief Vehicle::VERIFY
 * @param speed
 * @param who
 */
bool Vehicle::VERIFY(Vec3f newpos, dBodyID who)
{
    //if (speed>1000.0f && getType()!= ACTION)
    //    stop(who);
    if ((newpos-pos).magnitude()>1000.0f && (getType() != ACTION && getType()!=CONTROLABLEACTION && getType()!=EXPLOTABLEACTION))
    {
        //assert(!"position System is unstable.");   // This does not work with bullets.
        setPos(pos[0]+(rand() % 10 -5 +1)*0.1,pos[1],pos[2]+(rand() % 10 -5 +1)*0.1);
        //stop();
        //dBodyAddRelForce (who,0, 0,0);
        //dBodyAddRelTorque( who, 0, 0,0 );
        //dGeomDisable(geom);
        return false;
    }

    Vec3f angularVel = dBodyGetAngularVelInBody(who);
    float angspeed = angularVel.magnitude();

    if ((speed>10000.0f || isnan(speed)) && (getType() != ACTION && getType()!=CONTROLABLEACTION && getType()!=EXPLOTABLEACTION))
    {
        //assert(!"High speed:System is unstable.");   // This does not work with bullets.
        setPos(pos[0]+(rand() % 10 -5 +1)*0.1,pos[1],pos[2]+(rand() % 10 -5 +1)*0.1);
        //stop();
        //dBodyAddRelForce (who,0, 0,0);
        //dBodyAddRelTorque( who, 0, 0,0 );
        //dGeomDisable(geom);

        return false;
    }


    if ((isnan(newpos[0]) || isnan(newpos[1] || isnan(newpos[2]))))
    {
        //assert(!"NAN System is unstable.");   // This does not work with bullets.
        setPos(pos[0]+(rand() % 10 -5 +1)*0.1,pos[1],pos[2]+(rand() % 10 -5 +1)*0.1);
        //stop();
        //dBodyAddRelForce (who,0, 0,0);
        //dBodyAddRelTorque( who, 0, 0,0 );
        //dGeomDisable(geom);
        return false;
    }


    return true;
}

// Returns a 2D map version of position using current height of the object.
const Vec3f Vehicle::map(Vec3f position)
{
    Vec3f loc(position[0], getPos()[1], position[2]);

    return loc;
}

Vec3f Vehicle::toWorld(dBodyID body,Vec3f fw)
{
    dVector3 result;
    dBodyVectorToWorld(body, fw[0],fw[1],fw[2],result);
    return Vec3f(result[0],result[1],result[2]);
}

Vec3f Vehicle::toBody(dBodyID body,Vec3f fw)
{
    dVector3 result;
    dBodyVectorFromWorld(body, fw[0],fw[1],fw[2],result);
    return Vec3f(result[0],result[1],result[2]);
}

bool Vehicle::arrived()
{
    return dst_status == DestinationStatus::REACHED;
}

std::string Vehicle::getName()
{
    return name;
}

void Vehicle::setName(std::string newname)
{
    name = newname;
}

void Vehicle::setName(char name[256], int number)
{
    char msg[256];
    sprintf(msg, "%s %d", name, number);
    setName(std::string(msg));
}

void Vehicle::setNameByNumber(int number)
{
    assert(!"Vehicle numbering should not be called.");
}

int Vehicle::getNumber()
{
    return number;
}

void Vehicle::setNumber(int number)
{
    this->number = number;
}

void Vehicle::enableTelemetry()
{
    dotelemetry = true;
}

void Vehicle::disableTelemetry()
{
    dotelemetry = false;
}

Vec3f Vehicle::getDimensions()
{
    return Vec3f(width, height, length);
}

void Vehicle::updateScreenLocation()
{
    float winX=0;
    float winY=0;
    float winZ=0;

    getScreenLocation(winX, winY, winZ, getPos()[0], getPos()[1], getPos()[2]);
    //dout << winX << "," << winY << std::endl;

    onScreen[0] = winX;
    onScreen[1] = winZ; // @NOTE: Checkout here (they are rearranged)
    onScreen[2] = winY;
}

Vec3f Vehicle::screenLocation()
{
    return onScreen;
}

void Vehicle::setTheOrientation(Vec3f orientation)
{
    dMatrix3 R1,R2;
    dRSetIdentity(R1);
    dRSetIdentity(R2);

    dRFromAxisAndAngle(R1,0,1,0,getAzimuthRadians(orientation));

    dRFromAxisAndAngle(R2,1,0,0,getDeclination(orientation)*PI/180.0f);

    dQuaternion q1,q2,q3;
    dQfromR(q1,R1);

    dQfromR(q2,R2);

    dQMultiply0(q3,q2,q1);

    if (!Vehicle::inert)
        dBodySetQuaternion(me,q3);
}

TickRecord Vehicle::serialize()
{
    TickRecord tickrecord;

    float dBodyRotation[12];

    getR(dBodyRotation);

    Vec3f dBodyPosition = getPos();

    tickrecord.typeId = (int)getTypeId();
    tickrecord.type = getType();
    tickrecord.subtype = getSubType();

    tickrecord.faction = getFaction();
    tickrecord.health = getHealth();
    tickrecord.power = getPower();

    tickrecord.status = getStatus();

    tickrecord.ttl = getTtl();

    tickrecord.number = getNumber();

    tickrecord.location.pos1 = dBodyPosition[0];
    tickrecord.location.pos2 = dBodyPosition[1];
    tickrecord.location.pos3 = dBodyPosition[2];
    tickrecord.location.r1 = dBodyRotation[0];
    tickrecord.location.r2 = dBodyRotation[1];
    tickrecord.location.r3 = dBodyRotation[2];
    tickrecord.location.r4 = dBodyRotation[3];
    tickrecord.location.r5 = dBodyRotation[4];
    tickrecord.location.r6 = dBodyRotation[5];
    tickrecord.location.r7 = dBodyRotation[6];
    tickrecord.location.r8 = dBodyRotation[7];
    tickrecord.location.r9 = dBodyRotation[8];
    tickrecord.location.r10 = dBodyRotation[9];
    tickrecord.location.r11 = dBodyRotation[10];
    tickrecord.location.r12 = dBodyRotation[11];

    tickrecord.location.vel1 = getVelocity()[0];
    tickrecord.location.vel2 = getVelocity()[1];
    tickrecord.location.vel3 = getVelocity()[2];

    return tickrecord;
}

void Vehicle::deserialize(TickRecord record)
{
    float dBodyRotation[12];

    setPos(Vec3f(record.location.pos1,record.location.pos2, record.location.pos3));

    dBodyRotation[0] = record.location.r1;
    dBodyRotation[1] = record.location.r2;
    dBodyRotation[2] = record.location.r3;
    dBodyRotation[3] = record.location.r4;
    dBodyRotation[4] = record.location.r5;
    dBodyRotation[5] = record.location.r6;
    dBodyRotation[6] = record.location.r7;
    dBodyRotation[7] = record.location.r8;
    dBodyRotation[8] = record.location.r9;
    dBodyRotation[9] = record.location.r10;
    dBodyRotation[10] = record.location.r11;
    dBodyRotation[11] = record.location.r12;

    if (me != NULL) setRotation(dBodyRotation);  // Not for structures that do not contain a body

    setVelocity(record.location.vel1, record.location.vel2, record.location.vel3);

    setPower(record.power);
    health = record.health;
    setFaction(record.faction);
    setNumber(record.number);

    setStatus(record.status);

    setTtl(record.ttl);

    //if (record.type == MANTA)
    //{
    //    ((Manta*)this)->release(getForward());
    //}

}

#include "Wheel.h"
#include "../profiling.h"

Wheel::Wheel(int faction)
{
    Wheel(faction, 0.8, 5.5);
}
Wheel::Wheel(int faction, float cfm, float maxtorque)
{
    setFaction(faction);

    Wheel::cfm = cfm;
    Wheel::maxtorque = maxtorque;

}

Wheel::~Wheel()
{
    if (joint)
    {
        dJointDisable(joint);
        dJointDestroy(joint);
    }
}

void Wheel::init()
{

    Wheel::width = 0.2;
    Wheel::height = 1.0;
    Wheel::length = 1.0;

    setName("Wheel");

    setForward(0,0,1);

}

EntityTypeId Wheel::getTypeId()
{
    return EntityTypeId::TWheel;
}

int Wheel::getType()
{
    return WEAPON;
}

int Wheel::getSubType()
{
    return STRUCTURE;
}

void Wheel::setPos(const Vec3f &newpos)
{
    pos[0] = newpos[0];
    pos[1] = newpos[1];
    pos[2] = newpos[2];

    dGeomSetPosition(geom, pos[0], pos[1], pos[2]);
}
void Wheel::setPos(float x, float y, float z)
{
    pos[0] = x;
    pos[1] = y;
    pos[2] = z;

    dGeomSetPosition(geom, pos[0], pos[1], pos[2]);
}

void Wheel::doDynamics(dBodyID body)
{
    me = body;
    doDynamics();
}

void Wheel::doDynamics()
{
    dJointSetHinge2Param (joint,dParamVel2,-getThrottle());
    dJointSetHinge2Param (joint,dParamFMax2,maxtorque);

    Vec3f vav = dBodyGetAngularVelInBody(me);

    //@NOTE: For some unknown reason, the following line generates a segfault when the ODE world is terminated.
    //odometry += vav.magnitude();

    if (steeringwheel)
    {

        dReal v = dJointGetHinge2Angle1 (joint) - azimuth;
        if (v > 0.1) v = 0.1;
        if (v < -0.1) v = -0.1;
        v *= -5.0;


        dJointSetHinge2Param (joint,dParamVel,v);
        dJointSetHinge2Param (joint,dParamFMax,100);
        dJointSetHinge2Param (joint,dParamLoStop,-0.75);
        dJointSetHinge2Param (joint,dParamHiStop,0.75);
        dJointSetHinge2Param (joint,dParamFudgeFactor,0.1);
    }

    wrapDynamics(me);
}

void Wheel::stop()
{
    // @NOTE: ODE is extraordinary, but if you do not use it exactly as they suggest (using motors to drive forces) there could appear
    //    many issues.  I've found that I need to provide a stop function for the Wheels (attached to other units) because if there happen
    //    to be some problem with the unit, ODE halts and the simulation is stopped.
    Vehicle::stop();
}



void Wheel::embody(dWorldID world, dSpaceID space)
{
    me = dBodyCreate(world);
    dQuaternion q;
    dQFromAxisAndAngle (q,0,0,1,M_PI*0.5);
    // dQFromAxisAndAngle (q,0,0,1,M_PI*0.5);  // Use me for cylinders
    dBodySetQuaternion (me,q);
    embody(me);

    // @NOTE: Sphere are better because they are submerged in the sea floor when the walrus becomes a boat.
    geom = dCreateSphere(space, length);
    //geom = dCreateCylinder(space,length,width);
    dGeomSetBody(geom, me);
}


void Wheel::embody(dBodyID myBodySelf)
{
    dMass m;

    float myMass = 0.2f;

    dBodySetPosition(myBodySelf, pos[0], pos[1], pos[2]);
    dMassSetSphere(&m,1,length);
    dMassAdjust(&m, myMass*1.0f);
    dBodySetMass(myBodySelf,&m);

    me = myBodySelf;

}

static const dVector3  axison1 = { 0, 1, 0 },axison2 = { 1, 0, 0 };

void Wheel::attachTo(dWorldID world, Vehicle *attacher, Vec3f dimensions, WheelLocations loc)
{
    switch (loc) {
        case 1:attachTo(world, attacher, (+1)*dimensions[0]/2-0.1, (-1)*dimensions[1], (+1)*(dimensions[2]/2-0.2));break;
        case 2:attachTo(world, attacher, (-1)*dimensions[0]/2-0.1, (-1)*dimensions[1], (+1)*(dimensions[2]/2-0.2));break;
        case 3:attachTo(world, attacher, (+1)*dimensions[0]/2-0.1, (-1)*dimensions[1], (-1)*(dimensions[2]/2-0.2));break;
        case 4:attachTo(world, attacher, (-1)*dimensions[0]/2-0.1, (-1)*dimensions[1], (-1)*(dimensions[2]/2-0.2));break;
    default: assert(!"No option provided on object model construction.");
    }



}

void Wheel::attachTo(dWorldID world, Vehicle *attacher, float x, float y, float z)
{
    // 6,3,12
    setPos(attacher->getPos()[0]+x, attacher->getPos()[1]+y, attacher->getPos()[2]+z);

    joint = dJointCreateHinge2 (world,0);
    dJointAttach (joint,attacher->getBodyID(),getBodyID() );

    dJointSetHinge2Anchor (joint, getPos()[0], getPos()[1], getPos()[2]);
    dJointSetHinge2Axes (joint, axison1, axison2);
    dJointSetHinge2Param (joint,dParamSuspensionERP,0.4);  //0.4
    dJointSetHinge2Param (joint,dParamSuspensionCFM,cfm);    //0.08

    dJointSetHinge2Param (joint,dParamLoStop,0);
    dJointSetHinge2Param (joint,dParamHiStop,0);

}

dBodyID Wheel::getBodyID()
{
    return me;
}


void Wheel::doControl(Controller controller)
{
    Wheel::elevation = controller.registers.pitch;
    Wheel::azimuth = controller.registers.roll;

    setThrottle(controller.registers.thrust);


}

void Wheel::doControl()
{

}


void Wheel::doControl(struct controlregister conts)
{

}

void Wheel::getViewPort(Vec3f &Up, Vec3f &position, Vec3f &fwd)
{
    position = getPos();
    fwd = toVectorInFixedSystem(0, 0, 1,Wheel::azimuth,Wheel::elevation);
    Up = toVectorInFixedSystem(0.0f, 1.0f, 0.0f,0,0);

    Vec3f orig;

    fwd = fwd.normalize();
    orig = position;
    Up[0]=Up[2]=0;Up[1]=4;// 4 is a good value to be just above the shoulders, like watching somebodys cellphone on the train
    position = position - 40*fwd + Up;
    fwd = orig-position;
}

void Wheel::drawModel()
{
    drawModel(0,0,pos[0],pos[1],pos[2]);
}

void  Wheel::drawModel(float yRot, float xRot, float x, float y, float z)

{
    //glLoadIdentity();
    glPushMatrix();
    glTranslatef(x,y,z);

    // This will Rotate according to the R quaternion (which is a variable in Vehicle).
    float f[3];
    f[0] = 0; f[1] = 0; f[2] = 0;

    doTransform(f, R);

    //glRotatef(0, 0.0f, 0.0f, 1.0f);

    // @FIXME: Create a nice wheel.
    drawRectangularBox(width, height, length);

    glPopMatrix();

}

void Wheel::setSteering(bool option)
{
    Wheel::steeringwheel = option;
}

void Wheel::setAzimuth(float value)
{
    Wheel::azimuth = value;
}

void Wheel::resetOdometry()
{
    odometry = 0.0;
}


float Wheel::getOdometry()
{
    return odometry;
}

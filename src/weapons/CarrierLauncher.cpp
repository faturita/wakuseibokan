#include <unordered_map>
#include "../ThreeMaxLoader.h"
#include "../actions/Gunshot.h"
#include "../profiling.h"

#include "../actions/AAM.h"
#include "../actions/Missile.h"
#include "../actions/Torpedo.h"

#include "CarrierLauncher.h"

extern std::unordered_map<std::string, GLuint> textures;

CarrierLauncher::CarrierLauncher(int faction) : Weapon(faction)
{
    CarrierLauncher::zoom = 20.0f;
    CarrierLauncher::firingpos = Vec3f(0.0f,40.0f,0.0f);
}

void CarrierLauncher::init()
{
    //Load the model
    _model = (Model*)T3DSModel::loadModel("structures/missilelauncherbase.3ds",0.0f,0.0,0.0f,6,6,6,textures["metal"]);
    if (_model != NULL)
    {
        _topModel = (Model*) T3DSModel::loadModel("structures/launchertop.3ds",0,0,0,4,4,4,0);
    }

    Weapon::height=27.97;
    Weapon::length=11.68;
    Weapon::width=11.68;

    setName("Launcher");

    autostatus = AutoStatus::WATER;

    setForward(Vec3f(0,0,1));
}

void CarrierLauncher::drawModel(float yRot, float xRot, float x, float y, float z)
{
    float f[3];
    f[0] = 0; f[1] = 0; f[2] = 0;

    //Draw the saved model
    if (_model != NULL)
    {
        glPushMatrix();
        glTranslatef(x, y, z);

        doTransform(f, R);

        //glScalef(4.0f,4.0f,4.0f);
        //glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
        _model->draw(textures["metal"]);
        //drawRectangularBox(Structure::width, Structure::height, Structure::length);

        glTranslatef(0.0f,16.0f,0.0f);

        //glRotatef(270.0f, 0.0f, 1.0f, 0.0f);
        glRotatef(-Weapon::azimuth,0.0f,1.0f,0.0f);
        glRotatef(-Weapon::elevation,0.0f,0.0f,1.0f);

        _topModel->setTexture(textures["metal"]);
        _topModel->draw();

        glPopMatrix();

    }
    else
    {
        printf ("model is null\n");
    }
}

Vec3f CarrierLauncher::getForward()
{
    Vec3f forward = toVectorInFixedSystem(0, 0, 1,Weapon::azimuth,-Weapon::elevation);
    return forward;

    return forward;
}

void CarrierLauncher::setForward(float x, float y, float z)
{
    setForward(Vec3f(x,y,z));
}

void CarrierLauncher::setForward(Vec3f forw)
{
    Weapon::elevation = getDeclination(forw);
    Weapon::azimuth = getAzimuth(forw);

    Weapon::setForward(forw);

}

Vec3f CarrierLauncher::getFiringPort()
{
    //return Vec3f(getPos()[0],20.1765f, getPos()[2]);
    return Vec3f(getPos()[0],getPos()[1]+firingpos[1],getPos()[2]);
}

void CarrierLauncher::getViewPort(Vec3f &Up, Vec3f &position, Vec3f &fw)
{
    position = getPos();
    position[1] += firingpos[1];
    fw = toWorld(me, toVectorInFixedSystem(0,0,1,azimuth, -elevation));
    Up = toVectorInFixedSystem(0.0f, 1.0f, 0.0f,0,0);

    //dout << "Forward:" << forward << std::endl;

    Vec3f orig;


    fw = fw.normalize();
    orig = position;
    Up[0]=Up[2]=0;Up[1]=4;// poner en 4 si queres que este un toque arriba desde atras.
    position = position + (abs(zoom))*forward;

    //forward = -orig+position;
}


Vehicle* CarrierLauncher::fireAir(dWorldID world, dSpaceID space)
{
    if (getTtl()>0)
        return NULL;

    AAM *action = new AAM(getFaction());
    // Need axis conversion.
    action->init();
    action->setOrigin(me);

    Vec3f position = getPos();
    position[1] += .5f; // Move upwards to the center of the real rotation.
    Vec3f fw = toWorld(me, toVectorInFixedSystem(0,0,1,azimuth, -elevation));
    Vec3f Up = toVectorInFixedSystem(0.0f, 1.0f, 0.0f,0,0);

    Vec3f orig;

    fw = fw.normalize();
    orig = position;
    position = position + 60.0f*fw;
    fw = -orig+position;


    position = orig;
    position[1] += firingpos[1];
    action->embody(world,space);
    action->setPos(position[0],position[1],position[2]);


    dMatrix3 R,R2;
    dRSetIdentity(R);
    dRFromEulerAngles (R, 0,0,
                      0);

    dQuaternion q1,q2,q3;
    dQfromR(q1,R);
    dRFromAxisAndAngle(R2,0,1,0,getAzimuthRadians(fw));

    dQfromR(q2,R2);


    dQMultiply0(q3,q2,q1);


    Vec3f Ft=fw + Vec3f(0,20,0);
    Ft=Ft*1;

    dBodyAddForce(action->getBodyID(), Ft[0],Ft[1],Ft[2]);
    dBodySetQuaternion(action->getBodyID(),q3);

    setTtl(1000);

    // I can set power or something here.
    return (Vehicle*)action;
}

Vehicle* CarrierLauncher::fire(int weapon, dWorldID world, dSpaceID space)
{

    // @FIXME Check here if it is not better to use weapon instaed of the status.
    switch (autostatus) {
    case AutoStatus::GROUND:return fireGround(world,space);
        break;
    case AutoStatus::AIR:return fireAir(world, space);
        break;
    case AutoStatus::WATER:return fireWater(world, space);
        break;
    }

    return NULL;
}

void CarrierLauncher::embody(dWorldID world, dSpaceID space)
{
    me = dBodyCreate(world);
    embody(me);
    geom = dCreateBox(space, Weapon::width, Weapon::height, Weapon::length);
    dGeomSetBody(geom, me);
    dGeomSetPosition(geom, pos[0], pos[1], pos[2]);
}


void CarrierLauncher::embody(dBodyID myBodySelf)
{
    dMass m;

    float myMass = 0.001f;                  // @NOTE: I want it to do not disrupt too much the structure of the carrier.

    dBodySetPosition(myBodySelf, pos[0], pos[1], pos[2]);
    dMassSetBox(&m,1,Weapon::width, Weapon::height, Weapon::length);
    dMassAdjust(&m, myMass*1.0f);
    dBodySetMass(myBodySelf,&m);

    me = myBodySelf;

}

Vehicle* CarrierLauncher::fireWater(dWorldID world, dSpaceID space)
{
    if (getTtl()>0)
        return NULL;

    Torpedo *action = new Torpedo(getFaction());
    // Need axis conversion.
    action->init();
    action->setOrigin(me);

    Vec3f position = getPos();
    position[1] += .5f; // Move upwards to the center of the real rotation.
    Vec3f fw = toWorld(me, toVectorInFixedSystem(0,0,1,azimuth, -elevation));
    Vec3f Up = toVectorInFixedSystem(0.0f, 1.0f, 0.0f,0,0);

    Vec3f orig;

    fw = fw.normalize();
    orig = position;
    position = position + 50.0f*fw;
    fw = -orig+position;

    position[1] =1.0;
    action->embody(world,space);
    action->setPos(position[0],position[1],position[2]);


    dMatrix3 R,R2;
    dRSetIdentity(R);
    dRFromEulerAngles (R, 0,0,0);

    dQuaternion q1,q2,q3;
    dQfromR(q1,R);
    dRFromAxisAndAngle(R2,0,1,0,getAzimuthRadians(fw));

    dQfromR(q2,R2);


    dQMultiply0(q3,q2,q1);


    //Vec3f Ft=fw + Vec3f(0,20,0);
    //Ft=Ft*600;


    //dBodyAddForce(action->getBodyID(), Ft[0],Ft[1],Ft[2]);

    dBodySetQuaternion(action->getBodyID(),q3);
    dBodyAddRelForce(action->getBodyID(),0,0,60);

    setTtl(1000);

    // I can set power or something here.
    return (Vehicle*)action;
}

Vehicle* CarrierLauncher::fireGround(dWorldID world, dSpaceID space)
{
    if (getTtl()>0)
        return NULL;

    Missile *action = new Missile(getFaction());
    // Need axis conversion.
    action->init();
    action->setOrigin(me);

    Vec3f position = getPos();
    position[1] += .5f; // Move upwards to the center of the real rotation.
    Vec3f fw = toWorld(me, toVectorInFixedSystem(0,0,1,azimuth, -elevation));
    Vec3f Up = toVectorInFixedSystem(0.0f, 1.0f, 0.0f,0,0);

    Vec3f orig;

    fw = fw.normalize();
    orig = position;
    position = position + 60.0f*fw;
    fw = -orig+position;


    position = orig;
    position[1] += firingpos[1];
    action->embody(world,space);
    action->setPos(position[0],position[1],position[2]);


    dMatrix3 R,R2;
    dRSetIdentity(R);
    dRFromEulerAngles (R, 0,0,
                      0);

    dQuaternion q1,q2,q3;
    dQfromR(q1,R);
    dRFromAxisAndAngle(R2,0,1,0,getAzimuthRadians(fw));

    dQfromR(q2,R2);


    dQMultiply0(q3,q2,q1);


    Vec3f Ft=fw + Vec3f(0,20,0);
    Ft=Ft*250;

    dBodyAddForce(action->getBodyID(), Ft[0],Ft[1],Ft[2]);
    dBodySetQuaternion(action->getBodyID(),q3);

    setTtl(1000);

    // I can set power or something here.
    return (Vehicle*)action;
}

void CarrierLauncher::doControl()
{
    Controller c;

    c.registers = registers;

    CarrierLauncher::doControl(c);
}

void CarrierLauncher::doControl(Controller controller)
{
    doControl(controller.registers);
}

void CarrierLauncher::doControl(struct controlregister conts)
{
    zoom = 20.0f + conts.precesion*100;

    elevation -= conts.pitch * (20.0f/abs(zoom)) ;
    azimuth += conts.roll * (20.0f/abs(zoom)) ;

    setForward(toVectorInFixedSystem(0,0,1,azimuth, -elevation));
}


void CarrierLauncher::ground()
{
    autostatus = AutoStatus::GROUND;
}

void CarrierLauncher::air()
{
    autostatus = AutoStatus::AIR;
}

void CarrierLauncher::water()
{
    autostatus = AutoStatus::WATER;
}

int CarrierLauncher::getSubType()
{
    return LAUNCHER;
}

EntityTypeId CarrierLauncher::getTypeId()
{
    return EntityTypeId::TCarrierLauncher;
}

#include <unordered_map>
#include "Launcher.h"
#include "../actions/Missile.h"
#include "../actions/AAM.h"
#include "../actions/Torpedo.h"

#include "../sounds/sounds.h"

extern std::unordered_map<std::string, GLuint> textures;

Launcher::Launcher(int faction)
{
    Launcher::zoom = 20.0f;
    setFaction(faction);
}

void Launcher::init()
{
    //Load the model
    _model = (Model*)T3DSModel::loadModel("structures/missilelauncherbase.3ds",0.0f,0.0,0.0f,6,6,6,textures["metal"]);
    if (_model != NULL)
    {
        _topModel = (Model*) T3DSModel::loadModel("structures/launchertop.3ds",0,0,0,4,4,4,0);
    }

    Structure::height=27.97;
    Structure::length=11.68;
    Structure::width=11.68;

    Launcher::firingpos = Vec3f(0.0f,16.0f,0.0f);

    autostatus = AutoStatus::GROUND;

    setName("Launcher");

    setForward(Vec3f(0,0,1));
}

void Launcher::drawModel(float yRot, float xRot, float x, float y, float z)
{
    float f[3];
    f[0] = 0; f[1] = 0; f[2] = 0;

    //Draw the saved model
    if (_model != NULL)
    {
        glPushMatrix();
        glTranslatef(x, y, z);

        //glScalef(4.0f,4.0f,4.0f);
        //glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
        _model->draw(textures["metal"]);
        //drawRectangularBox(Structure::width, Structure::height, Structure::length);

        glTranslatef(0.0f,16.0f,0.0f);

        //glRotatef(270.0f, 0.0f, 1.0f, 0.0f);
        glRotatef(-Structure::azimuth,0.0f,1.0f,0.0f);
        glRotatef(-Structure::elevation,0.0f,0.0f,1.0f);

        _topModel->setTexture(textures["metal"]);
        _topModel->draw();

        glPopMatrix();

    }
    else
    {
        printf ("model is null\n");
    }
}

Vec3f Launcher::getForward()
{
    //Vec3f forward = toVectorInFixedSystem(0, 0, 1,Structure::azimuth,-Structure::inclination);
    //return forward;

    return forward;
}

void Launcher::setForward(float x, float y, float z)
{
    Launcher::setForward(Vec3f(x,y,z));
}

void Launcher::setForward(Vec3f forw)
{
    Structure::elevation = getDeclination(forw);
    Structure::azimuth = getAzimuth(forw);

    Structure::setForward(forw);

}

Vec3f Launcher::getFiringPort()
{
    //return Vec3f(getPos()[0],20.1765f, getPos()[2]);
    return Vec3f(getPos()[0],getPos()[1]+firingpos[1],getPos()[2]);
}

void Launcher::getViewPort(Vec3f &Up, Vec3f &position, Vec3f &forward)
{
    position = getPos();
    position[1] += 40.0f;
    forward = getForward();
    Up = toVectorInFixedSystem(0.0f, 1.0f, 0.0f,0,0);

    //dout << "Forward:" << forward << std::endl;

    Vec3f orig;


    forward = forward.normalize();
    orig = position;
    Up[0]=Up[2]=0;Up[1]=4;// poner en 4 si queres que este un toque arriba desde atras.
    position = position + (abs(zoom))*forward;

    //forward = -orig+position;
}


Vehicle* Launcher::fireAir(dWorldID world, dSpaceID space)
{
    if (getTtl()>0)
        return NULL;

    AAM *action = new AAM(getFaction());
    // Need axis conversion.
    action->init();
    action->setOrigin(me);

    Vec3f position = getPos();
    position[1] += .5f; // Move upwards to the center of the real rotation.
    Vec3f fw = getForward();
    Vec3f Up = toVectorInFixedSystem(0.0f, 1.0f, 0.0f,0,0);

    Vec3f orig;

    fw = fw.normalize();
    orig = position;
    position = position + 60.0f*fw;
    fw = -orig+position;


    position = orig;
    position[1] += 40;
    action->embody(world,space);
    action->setPos(position[0],position[1],position[2]);


    dMatrix3 R,R2;
    dRSetIdentity(R);
    dRFromEulerAngles (R, 0,0,
                      0);

    dQuaternion q1,q2,q3;
    dQfromR(q1,R);
    dRFromAxisAndAngle(R2,0,1,0,getAzimuthRadians(getForward()));

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

Vehicle* Launcher::fire(int weapon, dWorldID world, dSpaceID space)
{

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

Vehicle* Launcher::fireWater(dWorldID world, dSpaceID space)
{
    if (getTtl()>0)
        return NULL;

    Torpedo *action = new Torpedo(getFaction());
    // Need axis conversion.
    action->init();
    action->setOrigin(me);

    Vec3f position = getPos();
    position[1] += .5f; // Move upwards to the center of the real rotation.
    Vec3f fw = getForward();
    Vec3f Up = toVectorInFixedSystem(0.0f, 1.0f, 0.0f,0,0);

    Vec3f orig;

    fw = fw.normalize();
    orig = position;
    position = position + 80.0f*fw;
    fw = -orig+position;

    position[1] =1.0;
    action->embody(world,space);
    action->setPos(position[0],position[1],position[2]);


    dMatrix3 R,R2;
    dRSetIdentity(R);
    dRFromEulerAngles (R, 0,0,0);

    dQuaternion q1,q2,q3;
    dQfromR(q1,R);
    dRFromAxisAndAngle(R2,0,1,0,getAzimuthRadians(getForward()));

    dQfromR(q2,R2);


    dQMultiply0(q3,q2,q1);


    Vec3f Ft=fw + Vec3f(0,20,0);
    Ft=Ft*60;

    //dBodyAddForce(action->getBodyID(), Ft[0],Ft[1],Ft[2]);
    dBodyAddRelForce(action->getBodyID(),0,0,60);
    dBodySetQuaternion(action->getBodyID(),q3);

    setTtl(1000);

    // I can set power or something here.
    return (Vehicle*)action;
}

Vehicle* Launcher::fireGround(dWorldID world, dSpaceID space)
{
    if (getTtl()>0)
        return NULL;

    Missile *action = new Missile(getFaction());
    // Need axis conversion.
    action->init();
    action->setOrigin(me);

    Vec3f position = getPos();
    position[1] += .5f; // Move upwards to the center of the real rotation.
    Vec3f fw = getForward();
    Vec3f Up = toVectorInFixedSystem(0.0f, 1.0f, 0.0f,0,0);

    Vec3f orig;

    fw = fw.normalize();
    orig = position;
    position = position + 60.0f*fw;
    fw = -orig+position;


    position = orig;
    position[1] += 40;
    action->embody(world,space);
    action->setPos(position[0],position[1],position[2]);


    dMatrix3 R,R2;
    dRSetIdentity(R);
    dRFromEulerAngles (R, 0,0,
                      0);

    dQuaternion q1,q2,q3;
    dQfromR(q1,R);
    dRFromAxisAndAngle(R2,0,1,0,getAzimuthRadians(getForward()));

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

void Launcher::doControl()
{
    Controller c;

    c.registers = registers;

    Launcher::doControl(c);
}

/**
 * The values are modified from the rc
 * @brief Turret::doControl
 * @param controller
 */
void Launcher::doControl(Controller controller)
{
    zoom = 20.0f + controller.registers.precesion*100;

    elevation -= controller.registers.pitch * (20.0f/abs(zoom)) ;
    azimuth += controller.registers.roll * (20.0f/abs(zoom)) ;

    setForward(toVectorInFixedSystem(0,0,1,azimuth, -elevation));

}

void Launcher::ground()
{
    autostatus = AutoStatus::GROUND;
}

void Launcher::air()
{
    autostatus = AutoStatus::AIR;
}

void Launcher::water()
{
    autostatus = AutoStatus::WATER;
}

int Launcher::getSubType()
{
    return LAUNCHER;
}

EntityTypeId Launcher::getTypeId()
{
    return EntityTypeId::TLauncher;
}

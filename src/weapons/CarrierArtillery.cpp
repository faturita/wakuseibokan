#include <unordered_map>
#include "../actions/Shell.h"
#include "../ThreeMaxLoader.h"
#include "../actions/Gunshot.h"
#include "../profiling.h"
#include "../sounds/sounds.h"

#include "CarrierArtillery.h"

extern std::unordered_map<std::string, GLuint> textures;

CarrierArtillery::CarrierArtillery(int faction) : Weapon(faction)
{
    CarrierArtillery::zoom = 20.0f;
    CarrierArtillery::firingpos = Vec3f(0.0f,19.0f,0.0f);
}

void CarrierArtillery::init()
{
    //Load the model
    _model = (Model*)T3DSModel::loadModel("structures/turretbase.3ds",0.0f,-8.14f,0.0f,1,1,1,textures["sky"]);
    if (_model != NULL)
    {
        _topModel = (Model*) T3DSModel::loadModel("structures/turrettop.3ds",0,0,0,1,1,1,0);

    }

    Weapon::height=11.68;
    Weapon::length=11.68;
    Weapon::width=11.68;

    setForward(Vec3f(0,0,1));

    setName("Artillery");

    Weapon::azimuth = 0;
    Weapon::elevation = 0;
}

int CarrierArtillery::getSubType()
{
    return ARTILLERY;
}

EntityTypeId CarrierArtillery::getTypeId()
{
    return EntityTypeId::TCarrierArtillery;
}


void CarrierArtillery::embody(dWorldID world, dSpaceID space)
{
    me = dBodyCreate(world);
    embody(me);
    geom = dCreateBox(space, Weapon::width, Weapon::height, Weapon::length);
    dGeomSetBody(geom, me);
    dGeomSetPosition(geom, pos[0], pos[1], pos[2]);
}


void CarrierArtillery::embody(dBodyID myBodySelf)
{
    dMass m;

    float myMass = 0.001f;                  // @NOTE: I want it to do not disrupt too much the structure of the carrier.

    dBodySetPosition(myBodySelf, pos[0], pos[1], pos[2]);
    dMassSetBox(&m,1,Weapon::width, Weapon::height, Weapon::length);
    dMassAdjust(&m, myMass*1.0f);
    dBodySetMass(myBodySelf,&m);

    me = myBodySelf;

}

void CarrierArtillery::drawModel(float yRot, float xRot, float x, float y, float z)
{
    float f[3];
    f[0] = 0; f[1] = 0; f[2] = 0;

    //Draw the saved model
    if (_model != NULL)
    {
        glPushMatrix();
        glTranslatef(x, y, z);

        doTransform(f, R);

        glScalef(1.0f,1.0f,1.0f);
        //glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
        //drawRectangularBox(Weapon::width, Weapon::height, Weapon::length);
        glRotatef(270.0f, 0.0f, 1.0f, 0.0f);
        glRotatef(-Weapon::azimuth,0.0f,1.0f,0.0f);
        glRotatef(-Weapon::elevation,0.0f,0.0f,1.0f);

        _topModel->setTexture(textures["sky"]);
        _topModel->draw();


        glPopMatrix();

    }
    else
    {
        printf ("model is null\n");
    }
}

Vec3f CarrierArtillery::getForward()
{
    Vec3f forward = toVectorInFixedSystem(0, 0, 1,Weapon::azimuth,-Weapon::elevation);
    return forward;
}

void CarrierArtillery::setForward(float x, float y, float z)
{
    CarrierArtillery::setForward(Vec3f(x,y,z));
}

void CarrierArtillery::setForward(Vec3f forw)
{
    Weapon::elevation = getDeclination(forw);
    Weapon::azimuth = getAzimuth(forw);

    Weapon::setForward(forw);

}

Vec3f CarrierArtillery::getFiringPort()
{
    return Vec3f(getPos()[0],getPos()[1]+firingpos[1],getPos()[2]);
}

void CarrierArtillery::getViewPort(Vec3f &Up, Vec3f &position, Vec3f &fw)
{
    position = getPos();
    position[1] += 2.0f;
    fw = toWorld(me, toVectorInFixedSystem(0,0,1,azimuth, -elevation));
    Up = toVectorInFixedSystem(0.0f, 1.0f, 0.0f,0,0);

    //dout << "Forward:" << forward << std::endl;

    Vec3f orig;


    fw = fw.normalize();
    orig = position;
    Up[0]=Up[2]=0;Up[1]=4;// poner en 4 si queres que este un toque arriba desde atras.
    position = position + (abs(zoom))*fw;

    //forward = -orig+position;

}

Vehicle* CarrierArtillery::aimAndFire(dWorldID world, dSpaceID space, Vec3f target)
{
    Vec3f aimTarget = target - getPos();
    Vec3f aim = toBody(me,aimTarget);

    setForward((aim));
    Weapon::azimuth = getAzimuth(aim);
    Weapon::elevation = getDeclination(aim);


    Vehicle *action = fire(0,world,space,100);

    return action;
}

Vehicle* CarrierArtillery::fire(int weapon, dWorldID world, dSpaceID space)
{
    return fire(0,world, space, 1);
}

Vehicle* CarrierArtillery::fire(int weapon, dWorldID world, dSpaceID space, int shellloadingtime)
{
    if (getTtl()>0)
        return NULL;

    Shell *action = new Shell();
    // Need axis conversion.
    action->init();


    Vec3f position = getPos();
    forward = toWorld(me, toVectorInFixedSystem(0,0,1,azimuth, -elevation));
    Vec3f Up = toVectorInFixedSystem(0.0f, 1.0f, 0.0f,0,0);

    Vec3f orig;


    forward = forward.normalize();
    orig = position;
    position = position + 40*forward;
    forward = -orig+position;

    Vec3f Ft = forward*15;

    Vec3f f1(0.0,0.0,1.0);
    Vec3f f2 = forward.cross(f1);
    f2 = f2.normalize();
    float alpha = _acos( forward.dot(f1)/(f1.magnitude()*forward.magnitude()));

    dMatrix3 Re;
    dRSetIdentity(Re);
    dRFromAxisAndAngle(Re,f2[0],f2[1],f2[2],-alpha);

    action->embody(world,space);
    action->setPos(position[0],position[1],position[2]);

    Vec3f d = action->getPos() - getPos();

    //dout << d << std::endl;

    dout << "Elevation:" << elevation << " Azimuth:" << azimuth << std::endl;


    dBodySetLinearVel(action->getBodyID(),Ft[0],Ft[1],Ft[2]);
    dBodySetRotation(action->getBodyID(),Re);

    artilleryshot();
    setTtl(shellloadingtime);

    // I can set power or something here.
    return (Vehicle*)action;
}

void CarrierArtillery::doControl()
{
    Controller c;

    c.registers = registers;

    CarrierArtillery::doControl(c);
}

void CarrierArtillery::doControl(Controller controller)
{
    doControl(controller.registers);
}


void CarrierArtillery::doControl(struct controlregister conts)
{
    zoom = 20.0f + conts.precesion*100;

    elevation -= conts.pitch * (20.0f/abs(zoom)) ;
    azimuth += conts.roll * (20.0f/abs(zoom)) ;

    //dout << "Azimuth: " << azimuth << " Inclination: " << elevation << std::endl;

    setForward(toVectorInFixedSystem(0,0,1,azimuth, -elevation));
}






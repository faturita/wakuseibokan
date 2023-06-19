#include <unordered_map>
#include "../ThreeMaxLoader.h"
#include "../actions/Gunshot.h"
#include "../profiling.h"
#include "Weapon.h"
#include "CarrierTurret.h"

extern std::unordered_map<std::string, GLuint> textures;


CarrierTurret::CarrierTurret(int faction) : Weapon(faction)
{
    CarrierTurret::zoom = 20.0f;
    CarrierTurret::firingpos = Vec3f(0.0f,14.0f,0.0f);
}

void CarrierTurret::init()
{
    //Load the model
    _model = (Model*)T3DSModel::loadModel("structures/turretbase.3ds",0.0f,-8.14f,0.0f,1,1,1,textures["metal"]);
    if (_model != NULL)
    {
        _topModel = (Model*) T3DSModel::loadModel("structures/turrettop.3ds",0,0,0,1,1,1,0);
    }

    Weapon::height=4.68;
    Weapon::length=11.68;
    Weapon::width=11.68;

    setName("Turret");

    setForward(0,0,1);

    Weapon::azimuth = 0;
    Weapon::elevation = 0;
    
}

int CarrierTurret::getSubType()
{
    return TURRET;
}

EntityTypeId CarrierTurret::getTypeId()
{
    return EntityTypeId::TCarrierTurret;
}


void CarrierTurret::embody(dWorldID world, dSpaceID space)
{
    me = dBodyCreate(world);
    embody(me);
    geom = dCreateBox(space, Weapon::width, Weapon::height, Weapon::length);
    dGeomSetBody(geom, me);
    dGeomSetPosition(geom, pos[0], pos[1], pos[2]);
}


void CarrierTurret::embody(dBodyID myBodySelf)
{
	dMass m;

    float myMass = 0.001f;                  // @NOTE: I want it to do not disrupt too much the structure of the carrier.

	dBodySetPosition(myBodySelf, pos[0], pos[1], pos[2]);
    dMassSetBox(&m,1,Weapon::width, Weapon::height, Weapon::length);
	dMassAdjust(&m, myMass*1.0f);
	dBodySetMass(myBodySelf,&m);
    
    me = myBodySelf;
    
}



void  CarrierTurret::drawModel(float yRot, float xRot, float x, float y, float z)
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
        //_model->draw(textures["metal"]);
        drawRectangularBox(Weapon::width, Weapon::height, Weapon::length);

        glTranslatef(0.0f,firingpos[1]+0.83f,0.0f); // The nozzle location is a little bit up from the viewport.

        glRotatef(270.0f, 0.0f, 1.0f, 0.0f);

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


Vec3f CarrierTurret::getForward()
{
    Vec3f fw = toVectorInFixedSystem(0, 0, 1,Weapon::azimuth,-Weapon::elevation);
    return fw;
}

void CarrierTurret::setForward(float x, float y, float z)
{
    CarrierTurret::setForward(Vec3f(x,y,z));
}
void CarrierTurret::setForward(Vec3f forw)
{
    Weapon::elevation = getDeclination(forw);
    Weapon::azimuth = getAzimuth(forw);

    Weapon::setForward(forw);
}


Vec3f CarrierTurret::getFiringPort()
{
    return Vec3f(getPos()[0],getPos()[1]+firingpos[1],getPos()[2]);
}

void CarrierTurret::getViewPort(Vec3f &Up, Vec3f &position, Vec3f &fw)
{
    position = getPos();
    position[1] += firingpos[1]; // Move upwards to the center of the real rotation.
    fw = toWorld(me, toVectorInFixedSystem(0,0,1,azimuth, -elevation));
    Up = toVectorInFixedSystem(0.0f, 1.0f, 0.0f,0,0);

    Vec3f orig;

    fw = fw.normalize();
    orig = position;
    Up[0]=Up[2]=0;Up[1]=1;
    position = position + (abs(zoom))*fw;

    //forward = -orig+position;
}

Vehicle* CarrierTurret::aimAndFire(dWorldID world, dSpaceID space, Vec3f target)
{
    Vec3f aimTarget = target - (getPos() + firingpos);
    Vec3f aim = toBody(me,aimTarget);

    // You need to account of the firing position.
    //aim[1] = aim[1]-firingpos[1];

    setForward((aim));
    Weapon::azimuth = getAzimuth(aim);
    Weapon::elevation = getDeclination(aim);


    Vehicle *action = fire(0,world,space,5);

    return action;
}

Vehicle* CarrierTurret::fire(int weapon, dWorldID world, dSpaceID space)
{
    return fire(0,world, space, 1);
}

Vehicle* CarrierTurret::fire(int weapon, dWorldID world, dSpaceID space, int shellloadingtime)
{
    if (getTtl()>0)
        return NULL;

    Gunshot *action = new Gunshot();
    // Need axis conversion.
    action->init();

    Vec3f position = getPos();
    position[1] += (firingpos[1]); // Move upwards to the center of the real rotation.
    Vec3f fw = toWorld(me, toVectorInFixedSystem(0,0,1,azimuth, -elevation));
    Vec3f Up = toVectorInFixedSystem(0.0f, 1.0f, 0.0f,0,0);

    Vec3f orig;
    fw = fw.normalize();
    orig = position;
    position = position + 40*fw;
    fw = -orig+position;

    Vec3f Ft = fw*100;

    Vec3f f1(0.0,0.0,1.0);
    Vec3f f2 = fw.cross(f1);
    f2 = f2.normalize();
    float alpha = _acos( fw.dot(f1)/(f1.magnitude()*fw.magnitude()));

    dMatrix3 Re;
    dRSetIdentity(Re);
    dRFromAxisAndAngle(Re,f2[0],f2[1],f2[2],-alpha);

    action->embody(world,space);
    action->setPos(position[0],position[1],position[2]);

    Vec3f d = action->getPos() - getPos();

    //dout << d << std::endl;

    dBodySetLinearVel(action->getBodyID(),Ft[0],Ft[1],Ft[2]);
    dBodySetRotation(action->getBodyID(),Re);

    // Shell loading time.
    setTtl(shellloadingtime);

    // I can set power or something here.
    return (Vehicle*)action;
}

// Pick the target that was identified, aim to it and fire.
void CarrierTurret::doControl()
{
    Controller c;

    c.registers = registers;

    CarrierTurret::doControl(c);
}

void CarrierTurret::doControl(Controller controller)
{
    doControl(controller.registers);
}


/**
 * The values are modified from the rc
 * @brief Turret::doControl
 * @param controller
 */
void CarrierTurret::doControl(struct controlregister conts)
{
    zoom = 20.0f + conts.precesion*100;

    elevation -= conts.pitch * (20.0f/abs(zoom)) ;
    azimuth += conts.roll * (20.0f/abs(zoom)) ;

    //dout << "Azimuth: " << azimuth << " Inclination: " << elevation << std::endl;

    setForward(toVectorInFixedSystem(0,0,1,azimuth, -elevation));
}



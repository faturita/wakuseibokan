#include <unordered_map>
#include "../ThreeMaxLoader.h"
#include "../actions/Gunshot.h"
#include "Weapon.h"
#include "CarrierTurret.h"

extern std::unordered_map<std::string, GLuint> textures;


CarrierTurret::CarrierTurret(int faction) : Weapon(faction)
{
    CarrierTurret::zoom = 20.0f;
    CarrierTurret::firingpos = Vec3f(0.0f,19.0f,0.0f);
}

void CarrierTurret::init()
{
    //Load the model
    _model = (Model*)T3DSModel::loadModel("structures/turretbase.3ds",0.0f,-8.14f,0.0f,1,1,1,textures["metal"]);
    if (_model != NULL)
    {
        _topModel = (Model*) T3DSModel::loadModel("structures/turrettop.3ds",0,0,0,1,1,1,0);
    }

    Weapon::height=27.97;
    Weapon::length=11.68;
    Weapon::width=11.68;

    setForward(0,0,1);
    
}

int CarrierTurret::getType()
{
    return WALRUS;
}

int CarrierTurret::getSubType()
{
    return TURRET;
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

void CarrierTurret::attachTo(dWorldID world, Vehicle *attacher, float x, float y, float z)
{
    // @NOTE, it takes the position of the attacher and creates a fixed joint between them.  The joint is created in the world, in the zero group.
    setPos(attacher->getPos()[0]+x, attacher->getPos()[1]+y, attacher->getPos()[2]+z);

    joint = dJointCreateFixed(world,0);
    dJointAttach (joint,attacher->getBodyID(), getBodyID());

    dJointSetFixed(joint);     // @NOTE: SetFixed is mandatory, otherwise the objects share the same center.
    dJointSetFixedParam(joint,dParamSuspensionERP,0.0f );
    dJointSetFixedParam(joint,dParamSuspensionCFM, 0.0f);

}

dBodyID CarrierTurret::getBodyID()
{
    return me;
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

        glScalef(1.0f,1.0f,1.0f);
        //glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
        _model->draw(textures["metal"]);
        //drawRectangularBox(Structure::width, Structure::height, Structure::length);

        glTranslatef(0.0f,27.97f-8.140f,0.0f);

        glRotatef(270.0f, 0.0f, 1.0f, 0.0f);

        glRotatef(-Weapon::azimuth,0.0f,1.0f,0.0f);
        glRotatef(-Weapon::elevation,0.0f,0.0f,1.0f);

        _topModel->setTexture(textures["metal"]);
        _topModel->draw();

        // Gun shots
        //glTranslatef((firing+=100),0.0f,0.0f);
        //drawArrow(100.0f,0.0f,0.0f,0.0,1.0,0.0);


        // Laser Beam
        //drawArrow(10000.0f,0.0f,0.0f,0.0,1.0,0.0);

        //glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
        glPopMatrix();

        /**glPushMatrix();
        glLoadIdentity();
        glLoadMatrixf(modelview);
        glTranslatef((firing+=50),0.0f,0.0f);
        drawArrow(100.0f,0.0f,0.0f,1.0,0.0,0.0);

        glPopMatrix();**/
    }
    else
    {
        printf ("model is null\n");
    }
}


void CarrierTurret::doDynamics(dBodyID body)
{
    me = body;
    doDynamics();
}

void CarrierTurret::stop()
{

}

void CarrierTurret::doDynamics()
{


    wrapDynamics(me);
    
}


Vec3f CarrierTurret::getForward()
{
    Vec3f forward = toVectorInFixedSystem(0, 0, 1,Weapon::azimuth,-Weapon::elevation);
    return forward;
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
    //return Vec3f(getPos()[0],20.1765f, getPos()[2]);
    return Vec3f(getPos()[0],getPos()[1]+firingpos[1],getPos()[2]);
}

void CarrierTurret::getViewPort(Vec3f &Up, Vec3f &position, Vec3f &fw)
{
    position = getPos();
    position[1] += 19.0f; // Move upwards to the center of the real rotation.
    fw = getForward();
    Up = toVectorInFixedSystem(0.0f, 1.0f, 0.0f,0,0);

    Vec3f orig;

    fw = fw.normalize();
    orig = position;
    Up[0]=Up[2]=0;Up[1]=4;// poner en 4 si queres que este un toque arriba desde atras.
    position = position + (abs(zoom))*fw;

    //forward = -orig+position;
}



Vehicle* CarrierTurret::fire(dWorldID world, dSpaceID space)
{
    if (getTtl()>0)
        return NULL;

    Gunshot *action = new Gunshot();
    // Need axis conversion.
    action->init();

    Vec3f position = getPos();
    position[1] += 19.0f; // Move upwards to the center of the real rotation.
    forward = getForward();
    Vec3f Up = toVectorInFixedSystem(0.0f, 1.0f, 0.0f,0,0);

    Vec3f orig;

    forward = forward.normalize();
    orig = position;
    position = position + 40*forward;
    forward = -orig+position;

    Vec3f Ft = forward*100;

    Vec3f f1(0.0,0.0,1.0);
    Vec3f f2 = forward.cross(f1);
    f2 = f2.normalize();
    float alpha = acos( forward.dot(f1)/(f1.magnitude()*forward.magnitude()));

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
    setTtl(1);

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


/**
 * The values are modified from the rc
 * @brief Turret::doControl
 * @param controller
 */
void CarrierTurret::doControl(Controller controller)
{
    zoom = 20.0f + controller.registers.precesion*100;

    elevation -= controller.registers.pitch * (20.0f/abs(zoom)) ;
    azimuth += controller.registers.roll * (20.0f/abs(zoom)) ;

    //dout << "Azimuth: " << azimuth << " Inclination: " << elevation << std::endl;

    setForward(toVectorInFixedSystem(0,0,1,azimuth, -elevation));
}



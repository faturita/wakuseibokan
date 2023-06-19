#include <unordered_map>
#include "Artillery.h"
#include "../actions/Shell.h"

#include "../sounds/sounds.h"

#include "../profiling.h"

extern std::unordered_map<std::string, GLuint> textures;

Artillery::Artillery(int faction)
{
    Artillery::zoom = 20.0f;
    setFaction(faction);
}

void Artillery::init()
{
    //Load the model
    _model = (Model*)T3DSModel::loadModel("structures/turretbase.3ds",0.0f,-8.14f,0.0f,1,1,1,textures["sky"]);
    if (_model != NULL)
    {

    }

    Structure::height=27.97;
    Structure::length=11.68;
    Structure::width=11.68;

    Artillery::firingpos = Vec3f(0.0f,19.0f,0.0f);

    setName("Artillery");

    setForward(Vec3f(0,0,1));
}

void Artillery::drawModel(float yRot, float xRot, float x, float y, float z)
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
        //_model->draw(Structure::texture);
        //drawRectangularBox(Structure::width, Structure::height, Structure::length);
        glRotatef(270.0f, 0.0f, 1.0f, 0.0f);
        glRotatef(-Structure::azimuth,0.0f,1.0f,0.0f);
        glRotatef(-Structure::elevation,0.0f,0.0f,1.0f);
        draw3DSModel("structures/turrettop.3ds",0.0f,0.0f,0.0f,1,textures["sky"]);
        glPopMatrix();

    }
    else
    {
        printf ("model is null\n");
    }
}

Vec3f Artillery::getForward()
{
    //Vec3f forward = toVectorInFixedSystem(0, 0, 1,Structure::azimuth,-Structure::inclination);
    //return forward;

    return forward;
}

void Artillery::setForward(float x, float y, float z)
{
    Artillery::setForward(Vec3f(x,y,z));
}

void Artillery::setForward(Vec3f forw)
{
    Structure::elevation = getDeclination(forw);
    Structure::azimuth = getAzimuth(forw);

    Structure::setForward(forw);

}

Vec3f Artillery::getFiringPort()
{
    //return Vec3f(getPos()[0],20.1765f, getPos()[2]);
    return Vec3f(getPos()[0],getPos()[1]+firingpos[1],getPos()[2]);
}

void Artillery::getViewPort(Vec3f &Up, Vec3f &position, Vec3f &fw)
{
    position = getPos();
    position[1] += 2.0f;
    fw = toVectorInFixedSystem(0, 0, 1,azimuth,-elevation);
    Up = toVectorInFixedSystem(0.0f, 1.0f, 0.0f,0,0);

    //dout << "Forward:" << forward << std::endl;

    Vec3f orig;


    fw = fw.normalize();
    orig = position;
    Up[0]=Up[2]=0;Up[1]=4;// poner en 4 si queres que este un toque arriba desde atras.
    position = position + (abs(zoom))*fw;

    //forward = -orig+position;

}



Vehicle* Artillery::fire(int weapon, dWorldID world, dSpaceID space)
{
    if (getTtl()>0)
        return NULL;

    Shell *action = new Shell();
    // Need axis conversion.
    action->init();


    Vec3f position = getPos();
    forward = getForward();forward = toVectorInFixedSystem(0, 0, 1,azimuth,-elevation);
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
    setTtl(200);

    // I can set power or something here.
    return (Vehicle*)action;
}

void Artillery::doControl()
{
    Controller c;

    c.registers = registers;

    Artillery::doControl(c);
}

/**
 * The values are modified from the rc
 * @brief Turret::doControl
 * @param controller
 */
void Artillery::doControl(Controller controller)
{
    zoom = 20.0f + controller.registers.precesion*100;

    elevation -= controller.registers.pitch * (20.0f/abs(zoom)) ;
    azimuth += controller.registers.roll * (20.0f/abs(zoom)) ;

    setForward(toVectorInFixedSystem(0,0,1,azimuth, -elevation));

}


int Artillery::getSubType()
{
    return ARTILLERY;
}

EntityTypeId Artillery::getTypeId()
{
    return EntityTypeId::TArtillery;
}

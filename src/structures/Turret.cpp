#include <unordered_map>
#include "Turret.h"
#include "../actions/Gunshot.h"
#include "../profiling.h"

extern std::unordered_map<std::string, GLuint> textures;

Turret::Turret(int faction)
{
    Turret::zoom = 20.0f;
    setFaction(faction);
    Turret::firingpos = Vec3f(0.0f,19.0f,0.0f);
}

void Turret::init()
{
    //Load the model
    _model = (Model*)T3DSModel::loadModel("structures/turretbase.3ds",0.0f,-8.14f,0.0f,1,1,1,textures["metal"]);
    if (_model != NULL)
    {
        _topModel = (Model*) T3DSModel::loadModel("structures/turrettop.3ds",0,0,0,1,1,1,0);
    }

    Structure::height=27.97;
    Structure::length=11.68;
    Structure::width=11.68;

    setName("Turret");

    setForward(0,0,1);
}

void Turret::drawModel(float yRot, float xRot, float x, float y, float z)
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

        glRotatef(-Structure::azimuth,0.0f,1.0f,0.0f);
        glRotatef(-Structure::elevation,0.0f,0.0f,1.0f);

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

Vec3f Turret::getForward()
{
    //Vec3f forward = toVectorInFixedSystem(0, 0, 1,Structure::azimuth,-Structure::inclination);
    return forward;
}

void Turret::setForward(float x, float y, float z)
{
    Turret::setForward(Vec3f(x,y,z));
}
void Turret::setForward(Vec3f forw)
{
    Structure::elevation = getDeclination(forw);
    Structure::azimuth = getAzimuth(forw);

    Structure::setForward(forw);

}


Vec3f Turret::getFiringPort()
{
    //return Vec3f(getPos()[0],20.1765f, getPos()[2]);
    return Vec3f(getPos()[0],getPos()[1]+firingpos[1],getPos()[2]);
}

void Turret::getViewPort(Vec3f &Up, Vec3f &position, Vec3f &fw)
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



Vehicle* Turret::fire(int weapon, dWorldID world, dSpaceID space)
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
    float alpha = _acos( forward.dot(f1)/(f1.magnitude()*forward.magnitude()));

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
    setTtl(20);

    // I can set power or something here.
    return (Vehicle*)action;
}

// Pick the target that was identified, aim to it and fire.
void Turret::doControl()
{
    Controller c;

    c.registers = registers;

    Turret::doControl(c);
}


/**
 * The values are modified from the rc
 * @brief Turret::doControl
 * @param controller
 */
void Turret::doControl(Controller controller)
{
    zoom = 20.0f + controller.registers.precesion*100;

    elevation -= controller.registers.pitch * (20.0f/abs(zoom)) ;
    azimuth += controller.registers.roll * (20.0f/abs(zoom)) ;

    //dout << "Azimuth: " << azimuth << " Inclination: " << elevation << std::endl;

    setForward(toVectorInFixedSystem(0,0,1,azimuth, -elevation));
}

int Turret::getSubType()
{
    return TURRET;
}

EntityTypeId Turret::getTypeId()
{
    return EntityTypeId::TTurret;
}

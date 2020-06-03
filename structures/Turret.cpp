#include "Turret.h"
#include "../actions/Gunshot.h"

Turret::Turret(int faction)
{
    Turret::zoom = 20.0f;
    setFaction(faction);
}

void Turret::init()
{
    //Load the model
    _model = (Model*)T3DSModel::loadModel("structures/turretbase.3ds",0.0f,-8.14f,0.0f,1,1,1,Structure::texture);
    if (_model != NULL)
        _model->setAnimation("run");

    Structure::height=27.97;
    Structure::length=11.68;
    Structure::width=11.68;

    Turret::firingpos = Vec3f(0.0f,19.0f,0.0f);

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
        _model->draw(Structure::texture);
        //drawRectangularBox(Structure::width, Structure::height, Structure::length);

        glTranslatef(0.0f,27.97f-8.140f,0.0f);

        glRotatef(270.0f, 0.0f, 1.0f, 0.0f);

        glRotatef(-Structure::azimuth,0.0f,1.0f,0.0f);
        glRotatef(-Structure::elevation,0.0f,0.0f,1.0f);

        draw3DSModel("structures/turrettop.3ds",0.0f,0.0f,0.0f,1,Structure::texture);

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

void Turret::getViewPort(Vec3f &Up, Vec3f &position, Vec3f &forward)
{
    position = getPos();
    position[1] += 19.0f; // Move upwards to the center of the real rotation.
    forward = getForward();
    Up = toVectorInFixedSystem(0.0f, 1.0f, 0.0f,0,0);

    Vec3f orig;

    forward = forward.normalize();
    orig = position;
    Up[0]=Up[2]=0;Up[1]=4;// poner en 4 si queres que este un toque arriba desde atras.
    position = position + (abs(zoom))*forward;

    //forward = -orig+position;
}



Vehicle* Turret::fire(dWorldID world, dSpaceID space)
{
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

    //std::cout << d << std::endl;


    dBodySetLinearVel(action->getBodyID(),Ft[0],Ft[1],Ft[2]);
    dBodySetRotation(action->getBodyID(),Re);

    // I can set power or something here.
    return (Vehicle*)action;
}

void Turret::doControl()
{
    Controller c;

    c.registers = myCopy;

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

    //std::cout << "Azimuth: " << azimuth << " Inclination: " << elevation << std::endl;

    setForward(toVectorInFixedSystem(0,0,1,azimuth, -elevation));
}

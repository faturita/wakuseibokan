#include "Artillery.h"
#include "../actions/Gunshot.h"

Artillery::Artillery(int faction)
{
    Artillery::zoom = 20.0f;
    setFaction(faction);
}

void Artillery::init()
{
    //Load the model
    _model = (Model*)T3DSModel::loadModel("structures/turretbase.3ds",0.0f,-8.14f,0.0f,1,1,1,Structure::texture);
    if (_model != NULL)
        _model->setAnimation("run");

    Structure::height=27.97;
    Structure::length=11.68;
    Structure::width=11.68;

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
        draw3DSModel("structures/turrettop.3ds",0.0f,0.0f,0.0f,1,Structure::texture);
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

void Artillery::getViewPort(Vec3f &Up, Vec3f &position, Vec3f &forward)
{
    position = getPos();
    forward = getForward();
    Up = toVectorInFixedSystem(0.0f, 1.0f, 0.0f,0,0);

    std::cout << "Forward:" << forward << std::endl;

    Vec3f orig;


    forward = forward.normalize();
    orig = position;
    //Up[0]=Up[2]=0;Up[1]=4;// poner en 4 si queres que este un toque arriba desde atras.
    position = position + (abs(zoom))*forward;

    std::cout << "View Port:" << position << std::endl;

    //forward = -orig+position;
}



Vehicle* Artillery::fire(dWorldID world, dSpaceID space)
{
    Gunshot *action = new Gunshot();
    // Need axis conversion.
    action->init();



    Vec3f position = getPos();
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

void Artillery::doControl()
{
    Controller c;

    c.registers = myCopy;

    //c.registers.roll = 1;
    //if ((rand() % 100 + 1)<10)
    //    firing = !firing;

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

    // @NOTE debug
    printf ("Incl:%10.5f    Az: %10.5f\n", elevation, azimuth);

    elevation -= controller.registers.pitch * (20.0f/abs(zoom)) ;
    azimuth += controller.registers.roll * (20.0f/abs(zoom)) ;

    setForward(toVectorInFixedSystem(0,0,1,azimuth, -elevation));

}

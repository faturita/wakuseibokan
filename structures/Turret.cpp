#include "Turret.h"

int firing=0;

Turret::Turret()
{

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
        glRotatef(-Structure::inclination,0.0f,0.0f,1.0f);

        draw3DSModel("structures/turrettop.3ds",0.0f,0.0f,0.0f,1,Structure::texture);

        glTranslatef((firing+=100),0.0f,0.0f);
        drawArrow(100.0f,0.0f,0.0f,0.0,1.0,0.0);

        if (firing==1000) firing=0;

        //glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
        glPopMatrix();
    }
    else
    {
        printf ("model is null\n");
    }
}

Vec3f Turret::getForward()
{
    Vec3f forward = toVectorInFixedSystem(0, 0, 1,Structure::azimuth,-Structure::inclination);
    return forward;
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
    position = position + 20*forward;
    forward = -orig+position;
}

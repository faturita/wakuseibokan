#include "Turret.h"

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

        _model->draw(Structure::texture);
        //drawRectangularBox(Structure::width, Structure::height, Structure::length);

        glTranslatef(0.0f,27.97f-8.140f,0.0f);

        glRotatef(-Structure::azimuth,0.0f,1.0f,0.0f);
        glRotatef(Structure::inclination,0.0f,0.0f,1.0f);
        draw3DSModel("structures/turrettop.3ds",0.0f,0.0f,0.0f,1,Structure::texture);


        glPopMatrix();
    }
    else
    {
        printf ("model is null\n");
    }
}

void Turret::getViewPort(Vec3f &Up, Vec3f &position, Vec3f &forward)
{
    position = getPos();
    forward = getForward();
    Up = toVectorInFixedSystem(0.0f, 1.0f, 0.0f,0,0);

    Vec3f orig;

    forward = forward.normalize();
    orig = position;
    Up[0]=Up[2]=0;Up[1]=90;// poner en 4 si queres que este un toque arriba desde atras.
    position = position - 100*forward + Up;
    forward = orig-position;
}

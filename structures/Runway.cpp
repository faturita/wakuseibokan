#include "Runway.h"

Runway::Runway()
{

}

void Runway::init()
{
    //Load the model
    _model = (Model*)T3DSModel::loadModel("structures/runway.3ds",0.0f,5.0,0.0f,10,Structure::texture);
    if (_model != NULL)
        _model->setAnimation("run");

    Structure::height=50;
    Structure::length=8;
    Structure::width=8;

    setForward(0,0,1);
}

void Runway::drawModel(float yRot, float xRot, float x, float y, float z)
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

        glPopMatrix();
    }
    else
    {
        printf ("model is null\n");
    }
}

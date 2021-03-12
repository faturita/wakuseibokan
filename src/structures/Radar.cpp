#include "Radar.h"

extern GLuint _textureSky;
extern GLuint _textureMetal;

Radar::Radar(int faction)
{
    setFaction(faction);
}


void Radar::init()
{
    //Load the model
    _model = (Model*)T3DSModel::loadModel("structures/radar.3ds",-0,-0,0,1,1,1,Structure::texture);
    if (_model != NULL)
    {

    }

    Structure::height=150;
    Structure::length=70;
    Structure::width=40;

    setForward(0,0,1);
}

void Radar::drawModel(float yRot, float xRot, float x, float y, float z)
{
    float f[3];
    f[0] = 0; f[1] = 0; f[2] = 0;

    //Draw the saved model
    if (true || _model != NULL)
    {
        glPushMatrix();
        glTranslatef(x, y, z);

        glScalef(50.0f,50.0f,50.0f);

        doTransform(f,R);

        _model->draw(_textureSky);
        //drawRectangularBox(Structure::width, Structure::height, Structure::length);

        glPopMatrix();
    }
    else
    {
        printf ("model is null\n");
    }
}

int Radar::getSubType()
{
    return RADAR;
}

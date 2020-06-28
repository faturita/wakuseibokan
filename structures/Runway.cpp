#include "Runway.h"


extern GLuint _textureRoad;

Runway::Runway(int faction)
{
    setFaction(faction);
}

void Runway::init()
{
    //Load the model
    _model = (Model*)T3DSModel::loadModel("structures/runway.3ds",-466.06f,0.0f,0.0f,20,1,10,Structure::texture);
    if (_model != NULL)
    {

    }

    Structure::height=2;
    Structure::length=1000;
    Structure::width=20;

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

        //_model->draw(Structure::texture);
        //drawRectangularBox(Structure::width, Structure::height, Structure::length, _textureRoad);
        drawTheRectangularBox(_textureRoad,Structure::width, Structure::height, Structure::length);

        glPopMatrix();
    }
    else
    {
        printf ("model is null\n");
    }
}

int Runway::getType()
{
    return LANDINGABLE;
}

void Runway::taxi(Manta *m)
{
    m->setPos(pos[0],pos[1]+10, pos[2]);
    dBodySetPosition(m->getBodyID(),pos[0],pos[1]+10,pos[2]);
}

#include "Structure.h"

extern GLuint _textureBox;

Structure::Structure()
{

}

void Structure::init()
{
    //Load the model
    _model = (Model*)T3DSModel::loadModel("structures/structure.3ds",160.99f,-19.48f,76.36f,1,_textureBox);
    if (_model != NULL)
        _model->setAnimation("run");

    Structure::height=50;
    Structure::length=8;
    Structure::width=8;

    setForward(0,0,1);
}

void Structure::drawModel()
{
    drawModel(0,0,pos[0],pos[1],pos[2]);
}

void Structure::drawModel(float yRot, float xRot, float x, float y, float z)
{
    float f[3];
    f[0] = 0; f[1] = 0; f[2] = 0;

    //Draw the saved model
    if (_model != NULL)
    {
        glPushMatrix();
        glTranslatef(x, y, z);

        glScalef(1.0f,1.0f,1.0f);

        _model->draw();
        //drawRectangularBox(Structure::width, Structure::height, Structure::length);

        glPopMatrix();
    }
    else
    {
        printf ("model is null\n");
    }
}

dGeomID Structure::getGeom()
{
    return geom;
}

void Structure::embody(dWorldID world, dSpaceID space)
{
    geom = dCreateBox(space, Structure::width, Structure::height, Structure::length);
    dGeomSetPosition(geom, pos[0], pos[1], pos[2]);
}

void Structure::embody(dBodyID myBodySelf)
{

}

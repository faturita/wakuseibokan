#include <unordered_map>
#include "Factory.h"

extern std::unordered_map<std::string, GLuint> textures;

Factory::Factory(int faction)
{
    setFaction(faction);
}


void Factory::init()
{
    //Load the model
    _model = (Model*)T3DSModel::loadModel("structures/watertower.3ds",-19.0f,-6.36f,4.0f,1,1,1,textures["metal"]);

    if (_model != NULL)
    {

    }

    Structure::height=90;
    Structure::length=200;
    Structure::width=200;

    setName("Factory");

    setForward(0,0,1);
}

void Factory::drawModel(float yRot, float xRot, float x, float y, float z)
{
    float f[3];
    f[0] = 0; f[1] = 0; f[2] = 0;

    //Draw the saved model
    if (true || _model != NULL)
    {
        glPushMatrix();
        glTranslatef(x, y, z);

        glScalef(1.0f,1.0f,1.0f);

        doTransform(f,R);

        //_model->draw(Structure::texture);
        drawTexturedBox(textures["metal"], Structure::width, Structure::height, Structure::length);

        glPopMatrix();
    }
    else
    {
        printf ("model is null\n");
    }
}

int Factory::getSubType()
{
    return FACTORY;
}

EntityTypeId Factory::getTypeId()
{
    return EntityTypeId::TFactory;
}

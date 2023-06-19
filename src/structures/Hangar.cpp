#include <unordered_map>
#include "Hangar.h"

extern std::unordered_map<std::string, GLuint> textures;

Hangar::Hangar(int faction)
{
    setFaction(faction);
}

void Hangar::init()
{
    //Load the model
    _model = (Model*)T3DSModel::loadModel("structures/hangar.3ds",-19.0f,-6.36f,4.0f,1,1,1,textures["metal"]);
    if (_model != NULL)
    {

    }

    Structure::height=50;
    Structure::length=64.90;
    Structure::width=85.72;

    setName("Hangar");

    setForward(0,0,1);
}

void Hangar::drawModel(float yRot, float xRot, float x, float y, float z)
{
    float f[3];
    f[0] = 0; f[1] = 0; f[2] = 0;

    //Draw the saved model
    if (_model != NULL)
    {
        glPushMatrix();
        glTranslatef(x, y, z);

        glScalef(1.0f,1.0f,1.0f);

        //glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);
        glRotatef(180.0f, 0.0f, 1.0f, 0.0f);

        doTransform(f,R);

        _model->draw(textures["metal"]);
        //drawRectangularBox(Structure::width, Structure::height, Structure::length);

        glPopMatrix();
    }
    else
    {
        printf ("model is null\n");
    }
}
int Hangar::getSubType()
{
    return HANGAR;
}

EntityTypeId Hangar::getTypeId()
{
    return EntityTypeId::THangar;
}

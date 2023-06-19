#include <unordered_map>
#include "Antenna.h"

extern std::unordered_map<std::string, GLuint> textures;

Antenna::Antenna(int faction)
{
    setFaction(faction);
}


void Antenna::init()
{
    //Load the model
    _model = (Model*)T3DSModel::loadModel("structures/galileo.3ds",0,-0,0,1,1,1,textures["sky"]);
    if (_model != NULL)
    {

    }

    Structure::height=40;
    Structure::length=40;
    Structure::width=40;

    setName("Antenna");

    setForward(0,0,1);
}

void Antenna::drawModel(float yRot, float xRot, float x, float y, float z)
{
    float f[3];
    f[0] = 0; f[1] = 0; f[2] = 0;

    //Draw the saved model
    if (true || _model != NULL)
    {
        glPushMatrix();
        glTranslatef(x, y, z);

        glRotatef(180,1,0,0);
        glScalef(3.0f,3.0f,3.0f);

        doTransform(f,R);

        _model->draw(textures["sky"]);
        //drawRectangularBox(Structure::width, Structure::height, Structure::length);

        glPopMatrix();
    }
    else
    {
        printf ("model is null\n");
    }
}

int Antenna::getSubType()
{
    return ANTENNA;
}

EntityTypeId Antenna::getTypeId()
{
    return EntityTypeId::TAntenna;
}

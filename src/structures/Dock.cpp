#include <unordered_map>
#include "Dock.h"

extern std::unordered_map<std::string, GLuint> textures;

Dock::Dock(int faction)
{
    setFaction(faction);
}


void Dock::init()
{
    //Load the model
    _model = (Model*)T3DSModel::loadModel("structures/hangar.3ds",-19.0f,-6.36f,4.0f,1,1,1,Structure::texture);
    if (_model != NULL)
    {

    }

    Structure::height=2;
    Structure::length=500;
    Structure::width=20;

    setForward(0,0,1);
}

void Dock::drawModel(float yRot, float xRot, float x, float y, float z)
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

        drawTexturedBox(textures["metal"],20,50,40);
        drawTexturedBox(textures["metal"],Structure::width, Structure::height, Structure::length);

        glPopMatrix();
    }
    else
    {
        printf ("model is null\n");
    }
}

int Dock::getSubType()
{
    return DOCK;
}

bool Dock::checkHeightOffset(int heightOffset)
{
    return (heightOffset < 1) ;
}

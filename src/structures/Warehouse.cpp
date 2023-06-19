#include <unordered_map>
#include "Warehouse.h"

extern std::unordered_map<std::string, GLuint> textures;

Warehouse::Warehouse(int faction)
{
    setFaction(faction);
}


void Warehouse::init()
{
    //Load the model
    _model = (Model*)T3DSModel::loadModel("structures/hangar.3ds",-19.0f,-6.36f,4.0f,1,1,1,textures["metal"]);
    if (_model != NULL)
    {

    }

    Structure::height=50;
    Structure::length=70;
    Structure::width=40;

    setName("Warehouse");

    setForward(0,0,1);
}

void Warehouse::drawModel(float yRot, float xRot, float x, float y, float z)
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
        drawRectangularBox(Structure::width, Structure::height, Structure::length, Vec3f(0.0f, 0.2f, 0.0f),Vec3f(0.2, 0.2f, 0.0f),Vec3f(0.0f, 0.0f, 0.2f),Vec3f(0.2f, 0.0f, 0.1f));

        glPopMatrix();
    }
    else
    {
        printf ("model is null\n");
    }
}

int Warehouse::getSubType()
{
    return WAREHOUSE;
}

EntityTypeId Warehouse::getTypeId()
{
    return EntityTypeId::TWarehouse;
}

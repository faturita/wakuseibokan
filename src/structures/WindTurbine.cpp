#include <unordered_map>
#include "../profiling.h"
#include "WindTurbine.h"

extern std::unordered_map<std::string, GLuint> textures;

WindTurbine::WindTurbine(int faction)
{
    setFaction(faction);
}


void WindTurbine::init()
{
    //Load the model
    _model = (Model*)T3DSModel::loadModel(filereader("structures/waterdeposit.3ds"),-0,-0,0,1,1,1,textures["sky"]);
    if (_model != NULL)
    {

    }

    Structure::height=150;
    Structure::length=70;
    Structure::width=40;

    setName("WindTurbine");

    setForward(0,0,1);
}

void WindTurbine::drawModel(float yRot, float xRot, float x, float y, float z)
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

        _model->draw(textures["sky"]);
        //drawRectangularBox(Structure::width, Structure::height, Structure::length);

        glPopMatrix();
    }
    else
    {
        CLog::Write(CLog::Debug,"Model is null.\n");
    }
}

int WindTurbine::getSubType()
{
    return RADAR;
}

EntityTypeId WindTurbine::getTypeId()
{
    return EntityTypeId::TWindTurbine;
}

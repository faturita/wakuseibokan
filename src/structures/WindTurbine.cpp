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
    _model = (Model*)T3DSModel::loadModel(filereader("structures/windmill.3ds"),-0,15*5,0,5,5,5,textures["metal"]);
    if (_model != NULL)
    {

    }

    Structure::height=200;
    Structure::length=50;
    Structure::width=50;

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

        glScalef(1.0f,1.0f,1.0f);

        doTransform(f,R);

        _model->draw(textures["metal"]);
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
    return WINDTURBINE;
}

EntityTypeId WindTurbine::getTypeId()
{
    return EntityTypeId::TWindTurbine;
}

void WindTurbine::tick()
{
    production -= 0.01;
    if (production < 0)
    {
        production = 1;
        cargo[CargoTypes::POWERFUEL]++;
    }
}

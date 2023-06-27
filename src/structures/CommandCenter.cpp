#include <unordered_map>
#include "CommandCenter.h"

extern std::unordered_map<std::string, GLuint> textures;

extern std::vector<Structure*> structures;

int CommandCenter::getIslandType() const
{
    return islandType;
}

CommandCenter::CommandCenter(int faction, int islandType)
{
    setTtl(CommandCenter::BUILDING_TIME);
    setFaction(faction);
    CommandCenter::islandType = islandType;
}


void CommandCenter::init()
{
    //Load the model
    _model = (Model*)T3DSModel::loadModel("structures/hangar.3ds",-19.0f,-6.36f,4.0f,1,1,1,textures["military"]);
    if (_model != NULL)
    {

    }

    Structure::height=40;
    Structure::length=30;
    Structure::width=40;

    setName("CommandCenter");

    setForward(0,0,1);
}

void CommandCenter::drawModel(float yRot, float xRot, float x, float y, float z)
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

        drawTexturedBox(textures["military"],Structure::width, Structure::height, Structure::length);

        glPopMatrix();
    }
    else
    {
        printf ("model is null\n");
    }
}

void CommandCenter::getViewPort(Vec3f &Up, Vec3f &position, Vec3f &fw)
{   
    position = getPos();
    fw = toVectorInFixedSystem(0, 0, 1,Structure::azimuth,Structure::elevation);
    Up = toVectorInFixedSystem(0.0f, 1.0f, 0.0f,0,0);

    Vec3f orig;

    fw = fw.normalize();
    orig = position;
    Up[0]=Up[2]=0;Up[1]=10;// poner en 4 si queres que este un toque arriba desde atras.
    position = position - 90*fw + Up;
    fw = orig-position;

    setForward(fw);
}

int CommandCenter::getType()
{
    return CONTROL;
}

int CommandCenter::getSubType()
{
    return COMMANDCENTER;
}

EntityTypeId CommandCenter::getTypeId()
{
    return EntityTypeId::TCommandCenter;
}

void CommandCenter::restart()
{
    setTtl(CommandCenter::BUILDING_TIME);
}

void CommandCenter::setUnderAttack(bool isunderattack)
{
    CommandCenter::isunderattack = isunderattack;
}
bool CommandCenter::isUnderAttack()
{
    return CommandCenter::isunderattack;
}
void CommandCenter::setTimer(unsigned long timer)
{
    myTimer = timer;
}
unsigned long CommandCenter::getTimer()
{
    return myTimer;
}


#include <unordered_map>
#include "../units/Turtle.h"
#include "../engine.h"
#include "Armory.h"


extern std::unordered_map<std::string, GLuint> textures;

Armory::Armory(int faction)
{
    setFaction(faction);
}


void Armory::init()
{
    //Load the model
    _model = (Model*)T3DSModel::loadModel("structures/hangar.3ds",-19.0f,-6.36f,4.0f,1,1,1,textures["metal"]);
    if (_model != NULL)
    {

    }

    Structure::width=20;
    Structure::height=50;
    Structure::length=200;

    setName("Armory");

    setForward(0,0,1);
}

void Armory::drawModel(float yRot, float xRot, float x, float y, float z)
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

        //drawRectangularBox(width, height, length);

        //drawTexturedBox(textures["metal"],20,50,200);
        // @FIXME: Modify the ODE model to make a small box in the middle with the right size.
        // Here I put "2" because I want to look like a dock, but a little bit bigger to make it easier to be hit.
        drawTexturedBox(textures["metal"],Structure::width, Structure::height, Structure::length);

        glPopMatrix();
    }
    else
    {
        CLog::Write(CLog::Debug,"Model is null.\n");
    }
}

int Armory::getSubType()
{
    return ARMORY;
}

EntityTypeId Armory::getTypeId()
{
    return EntityTypeId::TArmory;
}

bool Armory::checkHeightOffset(int heightOffset)
{
    return (heightOffset >= 2) ;
}

Vehicle* Armory::spawn(dWorldID  world,dSpaceID space,int type, int number)
{
    Vec3f p;
    p = pos + getForward().normalize()*150;

    Turtle* _turtle = spawnTurtle(pos, 0.0, getFaction(),number, 3, space,world);

    //p = getForward().normalize()*(1500);
    //_walrus->goTo(Vec3f(pos[0]-p[0]-140*(number+1),pos[1]-p[1]+1,pos[2]-p[2]));
    //_walrus->enableAuto();

    alignToMe(_turtle->getBodyID());

    dMatrix3 Re2;
    dRSetIdentity(Re2);
    dRFromAxisAndAngle(Re2,0.0,1.0,0.0,PI);
    dBodySetRotation(_turtle->getBodyID(),Re2);

    p = p + Vec3f(0,10,0);

    Wheel *l,*r,*bl,*br;
    _turtle->setPos(p);
    _turtle->getWheels(l,r,bl,br);
    l->setPos(p);
    r->setPos(p);
    bl->setPos(p);
    br->setPos(p);

    dout << "Turtle::spawn: " << p << std::endl;

    return _turtle;
}

void Armory::getViewPort(Vec3f &Up, Vec3f &position, Vec3f &fwd)
{
    position = getPos();
    fwd = toVectorInFixedSystem(0, 0, 1,Structure::azimuth,Structure::elevation);

    Up = toVectorInFixedSystem(0.0f, 1.0f, 0.0f,0,0);

    Vec3f orig;

    fwd = fwd.normalize();
    orig = position;
    Up[0]=Up[2]=0;Up[1]=15;// poner en 4 si queres que este un toque arriba desde atras.
    position = position - 200*fwd + Up;
    fwd = orig-position;
}

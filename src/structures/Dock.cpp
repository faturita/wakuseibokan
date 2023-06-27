#include <unordered_map>
#include "../units/Walrus.h"
#include "Dock.h"

extern std::unordered_map<std::string, GLuint> textures;

Dock::Dock(int faction)
{
    setFaction(faction);
}


void Dock::init()
{
    //Load the model
    _model = (Model*)T3DSModel::loadModel("structures/hangar.3ds",-19.0f,-6.36f,4.0f,1,1,1,textures["metal"]);
    if (_model != NULL)
    {

    }

    Structure::width=20;
    Structure::height=5;
    Structure::length=500;

    setName("Dock");

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

        //drawRectangularBox(width, height, length);

        drawTexturedBox(textures["metal"],20,50,40);
        // @FIXME: Modify the ODE model to make a small box in the middle with the right size.
        // Here I put "2" because I want to look like a dock, but a little bit bigger to make it easier to be hit.
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

EntityTypeId Dock::getTypeId()
{
    return EntityTypeId::TDock;
}

bool Dock::checkHeightOffset(int heightOffset)
{
    return (heightOffset < 1) ;
}

Vehicle* Dock::spawn(dWorldID  world,dSpaceID space,int type, int number)
{
    Walrus *_walrus = new Walrus(getFaction());
    _walrus->init();
    _walrus->setNameByNumber(number);
    _walrus->embody(world,space);
    Vec3f p;
    p = getForward().normalize()*(getDimensions()[2]/2.0+300.0);
    _walrus->setPos(pos[0]-p[0],pos[1]-p[1]+1,pos[2]-p[2]);
    _walrus->setStatus(SailingStatus::SAILING);
    _walrus->stop();
    _walrus->inert = true;

    p = getForward().normalize()*(1500);
    _walrus->goTo(Vec3f(pos[0]-p[0]-140*(number+1),pos[1]-p[1]+1,pos[2]-p[2]));
    _walrus->enableAuto();

    alignToMe(_walrus->getBodyID());

    dMatrix3 Re2;
    dRSetIdentity(Re2);
    dRFromAxisAndAngle(Re2,0.0,1.0,0.0,PI);
    dBodySetRotation(_walrus->getBodyID(),Re2);

    return _walrus;
}

void Dock::getViewPort(Vec3f &Up, Vec3f &position, Vec3f &fwd)
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

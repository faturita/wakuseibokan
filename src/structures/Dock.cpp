#include <unordered_map>
#include "../units/Walrus.h"
#include "../units/CargoShip.h"
#include "Dock.h"

extern std::unordered_map<std::string, GLuint> textures;

Dock::Dock(int faction)
{
    setFaction(faction);
}


void Dock::init()
{
    //Load the model
    _model = (Model*)T3DSModel::loadModel(filereader("structures/hangar.3ds"),-19.0f,-6.36f,4.0f,1,1,1,textures["metal"]);
    if (_model != NULL)
    {

    }

    Structure::width=20;
    Structure::height=5;
    Structure::length=800;

    setName("Dock");

    setForward(0,0,1);

    Structure::azimuth = 0;
    Structure::elevation = 0;
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
        glPushMatrix();glTranslatef(0, 0, 400.0);drawTexturedBox(textures["metal"],20,50,40);glPopMatrix();
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
    Vehicle *v = NULL;

    if (type == VehicleSubTypes::CARGOSHIP)
    {
        CargoShip *cg = new CargoShip(getFaction());
        cg->init();
        cg->setNameByNumber(number);
        cg->embody(world,space);
        v = cg;
    }
    else 
    {
        Walrus *_walrus = new Walrus(getFaction());
        _walrus->init();
        _walrus->setNameByNumber(number);
        _walrus->embody(world,space);
        v = _walrus;
    }

    Vec3f p(0,0,0);
    // @NOTE: Be sure that 800 is enough according to the dock size.
    p = getForward().normalize()*800;
    v->setPos(pos[0]-p[0],pos[1]-p[1]+1,pos[2]-p[2]);
    v->setStatus(SailingStatus::SAILING);
    v->stop();
    //v->inert = true;

    alignToMe(v->getBodyID());

    dBodyID objectbody = v->getBodyID();
    if (type == VehicleSubTypes::CARGOSHIP)
    {
        dBodyAddRelForce(objectbody,0.0f,0.0f,-200000.0f);
    }
    else 
    {
        dBodyAddRelForce(objectbody,0.0f,0.0f,-200.0f);
        p = getForward().normalize()*(1500);
        v->goTo(Vec3f(pos[0]-p[0]-140*(number+1),pos[1]-p[1]+1,pos[2]-p[2]));
        v->enableAuto();
    }

    

    dMatrix3 Re2;
    dRSetIdentity(Re2);
    dRFromAxisAndAngle(Re2,0.0,1.0,0.0,0*PI);
    //dBodySetRotation(v->getBodyID(),Re2);

    return v;
}

void Dock::getViewPort(Vec3f &Up, Vec3f &position, Vec3f &fwd)
{
    position = getPos();

    Up = toVectorInFixedSystem(0.0f, 1.0f, 0.0f,0,0);

    Vec3f orig;

    fwd = getForward();
    fwd = fwd.rotateOnY(PI);
    fwd = toVectorInFixedSystem(fwd[0], fwd[1], fwd[2],Structure::azimuth,Structure::elevation);

    fwd = fwd.normalize();
    orig = position;
    Up[0]=Up[2]=0;Up[1]=15;// 4 for up a little bit from the back
    position = position - 200*fwd + Up;
    fwd = orig-position;
}

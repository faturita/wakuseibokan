#include <unordered_map>
#include "Runway.h"
#include "../units/Medusa.h"


extern std::unordered_map<std::string, GLuint> textures;

Runway::Runway(int faction)
{
    setFaction(faction);
}

void Runway::init()
{
    //Load the model
    _model = (Model*)T3DSModel::loadModel("structures/runway.3ds",-466.06f,0.0f,0.0f,20,1,10,textures["road"]);
    if (_model != NULL)
    {

    }

    Structure::height=2;
    Structure::length=1000;
    Structure::width=20;

    setName("Runway");

    setForward(0,0,1);
}

void Runway::drawModel(float yRot, float xRot, float x, float y, float z)
{
    float f[3];
    f[0] = 0; f[1] = 0; f[2] = 0;

    //Draw the saved model
    if (_model != NULL)
    {
        glPushMatrix();
        glTranslatef(x, y, z);

        glScalef(1.0f,1.0f,1.0f);

        doTransform(f,R);

        //_model->draw(Structure::texture);
        //drawRectangularBox(Structure::width, Structure::height, Structure::length, _textureRoad);
        drawTheRectangularBox(textures["road"],Structure::width, Structure::height, Structure::length);

        glPopMatrix();
    }
    else
    {
        printf ("model is null\n");
    }
}

void Runway::getViewPort(Vec3f &Up, Vec3f &position, Vec3f &fwd)
{
    position = getPos();
    fwd = toVectorInFixedSystem(0, 0, 1,Structure::azimuth,Structure::elevation);

    Up = toVectorInFixedSystem(0.0f, 1.0f, 0.0f,0,0);

    Vec3f orig;

    fwd = fwd.normalize();
    orig = position;
    Up[0]=Up[2]=0;Up[1]=15;// poner en 4 si queres que este un toque arriba desde atras.
    position = position - 100*fwd + Up;
    fwd = orig-position;
}

int Runway::getType()
{
    return LANDINGABLE;
}

void Runway::taxi(Manta *m)
{
    m->setPos(pos[0],pos[1]+10, pos[2]);
    dBodySetPosition(m->getBodyID(),pos[0],pos[1]+10,pos[2]);
}

void Runway::launch(Manta* m)
{
    m->inert = false;
    m->setStatus(FlyingStatus::FLYING);
    m->elevator = +12;
    struct controlregister c;
    c.thrust = 600.0f/(10.0);
    c.pitch = 12;
    m->setControlRegisters(c);
    m->setThrottle(600.0f);
    Vec3f p = m->getPos();
    p[1] += 20;
    m->setPos(p);
    dBodySetPosition(m->getBodyID(),p[0],p[1],p[2]);

    m->release(getForward());
    m->setForward(getForward());

    Vec3f f;
    f = m->getForward();
    //f= Vec3f(0.0f, 0.0f, 1.0f);  // @Hack to avoid the issue of the alignment of manta with the carrier.
    //f = f*200;
    dBodySetLinearVel(m->getBodyID(),f[0],f[1],f[2]);
}

Vehicle* Runway::spawn(dWorldID  world,dSpaceID space,int type, int number)
{
    Vehicle *v;

    if (type == MANTA)
    {
        Medusa *_manta1 = new Medusa(getFaction());
        _manta1->init();
        _manta1->setName("Medusa",number);
        _manta1->embody(world, space);
        _manta1->setPos(pos[0],pos[1]+28, pos[2]);
        _manta1->setStatus(FlyingStatus::LANDED);
        _manta1->inert = true;
        alignToMe(_manta1->getBodyID());
        v = (Vehicle*)_manta1;
    }

    return v;
}

int Runway::getSubType()
{
    return RUNWAY;
}

EntityTypeId Runway::getTypeId()
{
    return EntityTypeId::TRunway;
}

#include "Gunshot.h"

float Gunshot::getDamage() const
{
    return damage;
}

void Gunshot::setDamage(float value)
{
    damage = value;
}

Gunshot::Gunshot()
{
    Vehicle::setTtl(100);
}

Gunshot::~Gunshot()
{
    dGeomDestroy(geom);
    dBodyDestroy(me);

    assert(0 || !"Destroying bullets from the Gunshot object. This should not happen now.");
}

void Gunshot::init()
{
    Gunshot::height=0.1f;
    Gunshot::length=200.0f; // 4000 is more clear from the point of view of the bullets, but dissapears quickly when hit the target.
    Gunshot::width=0.1f;

    Gunshot::mass = 1.0f;

    setDamage(2);

    setName("Gunshot");

    setForward(0,0,1);
}

void Gunshot::drawModel()
{
    drawModel(0,0,pos[0],pos[1],pos[2]);
}

void Gunshot::setOrigin(dBodyID origin)
{
    Gunshot::origin = origin;
}

dBodyID Gunshot::getOrigin()
{
    return origin;
}


void Gunshot::drawModel(float yRot, float xRot, float x, float y, float z)
{
    float f[3];
    f[0] = 0; f[1] = 0; f[2] = 0;


    //Draw the saved model
    if (visible)
    {
        glPushMatrix();
        glTranslatef(x, y, z);

        glScalef(1.0f,1.0f,1.0f);

        //doTransform(f, R);

        //Vec3f v = dBodyGetLinearVelVec(me);
        Vec3f v = getVelocity();

        v = v*1.0/10.0;

        drawLine(v[0],v[1],v[2],1.0,0.0,0.0);

        glPopMatrix();
    }
    //else
    //{
    //    printf ("model is null\n");
    //}
}


void Gunshot::doDynamics(dBodyID body)
{
    //dBodyAddForce(body,0,9.81f,0);

    // @NOTE: Bullets are really unstable.
    if (VERIFY(pos, body))
        wrapDynamics(body);
}


void Gunshot::embody(dWorldID world, dSpaceID space)
{
    me = dBodyCreate(world);
    embody(me);
    //geom = dCreateSphere( space, 2.64f);
    geom = dCreateBox( space, Gunshot::width, Gunshot::height, Gunshot::length);
    dGeomSetBody(geom, me);
}

void Gunshot::embody(dBodyID myBodySelf)
{
    dMass m;

    float myMass = Gunshot::mass;

    dBodySetPosition(myBodySelf, pos[0], pos[1], pos[2]);
    dMassSetBox(&m, 1,Gunshot::width, Gunshot::height, Gunshot::length);
    dMassAdjust(&m, myMass*1.0f);
    dBodySetMass(myBodySelf,&m);

    me = myBodySelf;

}

int Gunshot::getType()
{
    return ACTION;
}

EntityTypeId Gunshot::getTypeId()
{
    return EntityTypeId::TGunshot;
}

void Gunshot::setVisible(bool val)
{
    Gunshot::visible = val;
    setTtl(2);
}

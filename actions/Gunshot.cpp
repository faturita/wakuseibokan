#include "Gunshot.h"

Gunshot::Gunshot()
{
    ttl = 100;
}

Gunshot::~Gunshot()
{
    dGeomDestroy(geom);
    dBodyDestroy(me);

    printf("Good bye....\n");
}

void Gunshot::init()
{
    Gunshot::height=0.1f;
    Gunshot::length=0.1f;
    Gunshot::width=0.1f;

    setForward(0,0,1);
}

void Gunshot::drawModel()
{
    drawModel(0,0,pos[0],pos[1],pos[2]);
}

void Gunshot::drawModel(float yRot, float xRot, float x, float y, float z)
{
    float f[3];
    f[0] = 0; f[1] = 0; f[2] = 0;

    //Draw the saved model
    //if (_model != NULL)
    //{
        glPushMatrix();
        glTranslatef(x, y, z);

        glScalef(1.0f,1.0f,1.0f);

        //doTransform(f, R);

        Vec3f v = dBodyGetLinearVelVec(me);

        v = v*100;

        drawArrow(v[0],v[1],v[2],1.0,0.0,0.0);
        //drawRectangularBox(Gunshot::width, Gunshot::height, Gunshot::length);

        glPopMatrix();
    //}
    //else
    //{
    //    printf ("model is null\n");
    //}
}


void Gunshot::doDynamics(dBodyID body)
{
    //dBodyAddForce(body,0,9.81f,0);

    if (ttl>0)
        ttl--;

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

    float myMass = 1000.0f;

    dBodySetPosition(myBodySelf, pos[0], pos[1], pos[2]);
    dMassSetBox(&m, 1,Gunshot::width, Gunshot::height, Gunshot::length);
    dMassAdjust(&m, myMass*1.0f);
    dBodySetMass(myBodySelf,&m);

    me = myBodySelf;

}

int Gunshot::getType()
{
    return 5;
}

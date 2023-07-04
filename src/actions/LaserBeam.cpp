#include "LaserBeam.h"
#include "../profiling.h"

LaserBeam::LaserBeam()
{
    Vehicle::setTtl(100);
    range = 4000.0f;
}

LaserBeam::~LaserBeam()
{
    dGeomDestroy(geom);
    dBodyDestroy(me);
}

void LaserBeam::init()
{
    Gunshot::height=0.1f;
    Gunshot::length=range;
    Gunshot::width=0.1f;

    setName("LaserBeam");

    setForward(0,0,1);
}

void LaserBeam::drawModel()
{
    drawModel(0,0,pos[0],pos[1],pos[2]);
}

void LaserBeam::drawModel(float yRot, float xRot, float x, float y, float z)
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

        Vec3f v = getVelocity();

        v = v*range;

        drawArrow(v[0],v[1],v[2],0.0,1.0,0.0);
        //drawArrow(10000.0f,0.0f,0.0f,0.0,1.0,0.0);
        //drawRectangularBox(Gunshot::width, Gunshot::height, Gunshot::length);

        glPopMatrix();
    //}
    //else
    //{
    //    printf ("model is null\n");
    //}
}


void LaserBeam::doDynamics(dBodyID body)
{
    //dBodyAddForce(body,0,9.81f,0);
    //wrapDynamics(body);
    Vec3f v = dBodyGetLinearVelVec(me);
    setVelocity(v[0],v[1],v[2]);
}


void LaserBeam::embody(dWorldID world, dSpaceID space)
{
    me = dBodyCreate(world);
    embody(me);
    //geom = dCreateSphere( space, 2.64f);
    geom = dCreateBox( space, Gunshot::width, Gunshot::height, Gunshot::length);
    dBodySetGravityMode(me,0);
    dGeomSetBody(geom, me);
}

void LaserBeam::embody(dBodyID myBodySelf)
{
    dMass m;

    float myMass = 1.0f;

    dBodySetPosition(myBodySelf, pos[0], pos[1], pos[2]);
    dMassSetBox(&m, 1,Gunshot::width, Gunshot::height, Gunshot::length);
    dMassAdjust(&m, myMass*1.0f);
    dBodySetMass(myBodySelf,&m);

    me = myBodySelf;

}

int LaserBeam::getType()
{
    return ACTION;
}

EntityTypeId LaserBeam::getTypeId()
{
    return EntityTypeId::TLaserBeam;
}


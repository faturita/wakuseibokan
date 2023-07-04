#include "LaserRay.h"

LaserRay::LaserRay()
{
    // @FIXME: LaserRay should last all the way until the LaserTurret is overheated.  This value is too big.
    Vehicle::setTtl(10000);
}

LaserRay::~LaserRay()
{
    dGeomDestroy(geom);
    dBodyDestroy(me);

    assert(!"This destructor should not be executed.");
}

void LaserRay::init()
{
    setForward(0,0,1);

    setName("LaserRay");
}

void LaserRay::drawModel()
{
    drawModel(0,0,pos[0],pos[1],pos[2]);
}

void LaserRay::drawModel(float yRot, float xRot, float x, float y, float z)
{
    float f[3];
    f[0] = 0; f[1] = 0; f[2] = 0;

    //Draw the saved model
    //if (_model != NULL)
    //{
        glPushMatrix();
        glTranslatef(x, y, z);

        glScalef(1.0f,1.0f,1.0f);

        doTransform(f, R);

        //Vec3f v = dBodyGetLinearVelVec(me);

        //drawArrow(v[0],v[1],v[2],0.0,1.0,0.0);
        drawArrow(0.0f,0.0f,4000.0f,0.0,1.0,0.0);
        //drawRectangularBox(Gunshot::width, Gunshot::height, Gunshot::length);

        glPopMatrix();
    //}
    //else
    //{
    //    printf ("model is null\n");
    //}
}


void LaserRay::doDynamics(dBodyID body)
{
    //dBodyAddForce(body,0,9.81f,0);
    //wrapDynamics(body);
}


void LaserRay::embody(dWorldID world, dSpaceID space)
{
    geom = dCreateRay(space, 4000.0f);
    dGeomSetPosition(geom,pos[0],pos[1],pos[2]);
}


void LaserRay::setPos(float x, float y, float z)
{
    pos[0] = x;
    pos[1] = y;
    pos[2] = z;

}

void LaserRay::setPos(const Vec3f &newpos)
{
    pos = newpos;
}

void LaserRay::embody(dBodyID myBodySelf)
{


}

int LaserRay::getType()
{
    return RAY;
}

EntityTypeId LaserRay::getTypeId()
{
    return EntityTypeId::TLaserRay;
}

void LaserRay::disable()
{
    setTtl(1);
}

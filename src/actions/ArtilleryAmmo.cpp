#include <unordered_map>
#include "../ThreeMaxLoader.h"
#include "ArtilleryAmmo.h"

extern std::unordered_map<std::string, GLuint> textures;

ArtilleryAmmo::ArtilleryAmmo()
{
    Vehicle::setTtl(500);
}

ArtilleryAmmo::~ArtilleryAmmo()
{
    dGeomDestroy(geom);
    dBodyDestroy(me);

    assert(0 || !"Destroying bullets from the Gunshot object. This should not happen now.");
}

void ArtilleryAmmo::init()
{
    ArtilleryAmmo::height=5.5f;
    ArtilleryAmmo::length=5.5f;
    ArtilleryAmmo::width=5.5f;

    ArtilleryAmmo::mass = 10.01f;

    //Load the model
    _model = (Model*)T3DSModel::loadModel("units/shell.3ds",0.0f,0.0f,0.0f,1,1,1,textures["metal"]);
    if (_model != NULL)
    {
    }

    setDamage(80);
    setName("Ammo");

    setForward(0,0,1);
}

void ArtilleryAmmo::drawModel()
{
    drawModel(0,0,pos[0],pos[1],pos[2]);
}

void ArtilleryAmmo::drawModel(float yRot, float xRot, float x, float y, float z)
{
    float f[3];
    f[0] = 0; f[1] = 0; f[2] = 0;

    //Draw the saved model
    if (visible)
    {
        glPushMatrix();
        glTranslatef(x, y, z);

        glScalef(1.0f,1.0f,1.0f);

        //drawArrow(v[0],v[1],v[2],1.0,0.0,0.0);
        //drawRedBox(Gunshot::width, Gunshot::height, Gunshot::length);

        doTransform(f, R);
        _model->setTexture(textures["metal"]);
        _model->draw();

        glPopMatrix();
    }
    //else
    //{
    //    printf ("model is null\n");
    //}
}


void ArtilleryAmmo::doDynamics(dBodyID body)
{
    //dBodyAddForce(body,0,9.81f,0);

    // @NOTE: Bullets are really unstable.
    if (VERIFY(pos, body))
        wrapDynamics(body);
}


void ArtilleryAmmo::embody(dWorldID world, dSpaceID space)
{
    me = dBodyCreate(world);
    embody(me);
    //geom = dCreateSphere( space, 2.64f);
    geom = dCreateBox( space, ArtilleryAmmo::width, ArtilleryAmmo::height, ArtilleryAmmo::length);
    dGeomSetBody(geom, me);
}

void ArtilleryAmmo::embody(dBodyID myBodySelf)
{
    dMass m;

    float myMass = ArtilleryAmmo::mass;

    dBodySetPosition(myBodySelf, pos[0], pos[1], pos[2]);
    dMassSetBox(&m, 1,ArtilleryAmmo::width, ArtilleryAmmo::height, ArtilleryAmmo::length);
    dMassAdjust(&m, myMass*1.0f);
    dBodySetMass(myBodySelf,&m);

    me = myBodySelf;

}

int ArtilleryAmmo::getType()
{
    return ACTION;
}

EntityTypeId ArtilleryAmmo::getTypeId()
{
    return EntityTypeId::TArtilleryAmmo;
}


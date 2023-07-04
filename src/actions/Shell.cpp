#include <unordered_map>
#include "../ThreeMaxLoader.h"
#include "Shell.h"

extern std::unordered_map<std::string, GLuint> textures;


Shell::Shell()
{
    Vehicle::setTtl(1000);
}

Shell::~Shell()
{
    dGeomDestroy(geom);
    dBodyDestroy(me);

    assert(0 || !"Destroying bullets from the Gunshot object. This should not happen now.");
}

void Shell::init()
{
    Shell::height=10.0f;
    Shell::length=10.0f;
    Shell::width=10.0f;

    Shell::mass = 1000.0f;

    //Load the model
    _model = (Model*)T3DSModel::loadModel("units/shell.3ds",0.0f,0.0f,0.0f,1,1,1,textures["metal"]);
    if (_model != NULL)
    {
    }

    setDamage(20);
    setName("Shell");

    setForward(0,0,1);
}


void Shell::drawModel()
{
    drawModel(0,0,pos[0],pos[1],pos[2]);
}

void Shell::drawModel(float yRot, float xRot, float x, float y, float z)
{
    float f[3];
    f[0] = 0; f[1] = 0; f[2] = 0;

    //Draw the saved model
    if (visible)
    {
        glPushMatrix();
        glTranslatef(x, y, z);

        glScalef(1.0f,1.0f,1.0f);

        doTransform(f, R);

        //drawArrow(v[0],v[1],v[2],1.0,0.0,0.0);
        //drawRectangularBox(Gunshot::width, Gunshot::height, Gunshot::length);
        _model->setTexture(textures["metal"]);
        _model->draw();



        glPopMatrix();
    }
    //else
    //{
    //    printf ("model is null\n");
    //}
}


void Shell::doDynamics(dBodyID body)
{
    //dBodyAddForce(body,0,9.81f,0);

    // @NOTE: Bullets are really unstable.
    if (VERIFY(pos, body))
        wrapDynamics(body);
}


void Shell::embody(dWorldID world, dSpaceID space)
{
    me = dBodyCreate(world);
    embody(me);
    //geom = dCreateSphere( space, 2.64f);
    geom = dCreateBox( space, Shell::width, Shell::height, Shell::length);
    dGeomSetBody(geom, me);
}

void Shell::embody(dBodyID myBodySelf)
{
    dMass m;

    float myMass = Shell::mass;

    dBodySetPosition(myBodySelf, pos[0], pos[1], pos[2]);
    dMassSetBox(&m, 1,Shell::width, Shell::height, Shell::length);
    dMassAdjust(&m, myMass*1.0f);
    dBodySetMass(myBodySelf,&m);

    me = myBodySelf;

}

int Shell::getType()
{
    return EXPLOTABLEACTION;
}

EntityTypeId Shell::getTypeId()
{
    return EntityTypeId::TShell;
}




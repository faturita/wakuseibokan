#include <unordered_map>
#include "../ThreeMaxLoader.h"
#include "AdvancedManta.h"

extern std::unordered_map<std::string, GLuint> textures;

AdvancedManta::AdvancedManta(int newfaction) : SimplifiedDynamicManta(newfaction)
{

}

void AdvancedManta::init()
{
    //Load the model
    _model = (Model*)T3DSModel::loadModel("units/medusa.3ds",0,0,0,1,1,1,0);
    if (_model != NULL)
    {
        //_topModel = (Model*)T3DSModel::loadModel("structures/turrettop.3ds",0,0,0,0.1,0.1,0.1,0);
    }

    width=10.0f;
    height=3.0f;
    length=10.0f;

    setForward(0,0,1);

    status = 0;
}


void AdvancedManta::drawModel(float yRot, float xRot, float x, float y, float z)
{
    float f[3];
    f[0] = 0; f[1] = 0; f[2] = 0;

    //Draw the saved model
    if (_model != NULL)
    {
        glPushMatrix();
        glTranslatef(x, y, z);

        glScalef(2.0f,2.0f,2.0f);

        doTransform(f, R);

        //drawArrow();
        //drawArrow(S[0],S[1],S[2],1.0,0.0,0.0);

        // Draw linear velocity
        //drawArrow(V[0],V[1],V[2],0.0,1.0,0.0);

        //drawRectangularBox(width/2.0f, height/2.0f, length/2.0f);

        glRotatef(180.0f, 0.0f, 0.0f, 1.0f);
        glRotatef(-180.0f, 1.0f, 0.0f, 0.0f);
        glRotatef(-180.0f, 0.0f, 1.0f, 0.0f);

        glColor3f(1.0,1.0f,1.0f);
        _model->setTexture(textures["sky"]);
        _model->draw();

        glPopMatrix();
    }
    else
    {
        printf ("model is null\n");
    }
}

int AdvancedManta::getSubType()
{
    return SIMPLEMANTA;
}

EntityTypeId AdvancedManta::getTypeId()
{
    return EntityTypeId::TAdvancedManta;
}

void AdvancedManta::embody(dWorldID world, dSpaceID space)
{
    me = dBodyCreate(world);
    embody(me);
    //geom = dCreateSphere( space, 2.64f);
    geom = dCreateBox( space, width, height, length);
    dGeomSetBody(geom, me);
}

void AdvancedManta::embody(dBodyID myBodySelf)
{
    dMass m;

    float myMass = 10.0f;

    dBodySetPosition(myBodySelf, pos[0], pos[1], pos[2]);
    dMassSetBox(&m,1, width, height, length);
    dMassAdjust(&m, myMass*1.0f);
    dBodySetMass(myBodySelf,&m);

    me = myBodySelf;
}

void AdvancedManta::getViewPort(Vec3f &Up, Vec3f &position, Vec3f &forward)
{
    position = getPos();
    forward = getForward();
    Up = toVectorInFixedSystem(0.0f, 1.0f, 0.0f,0,0);

    Vec3f orig;


    forward = forward.normalize();
    orig = position;
    Up[0]=Up[2]=0;Up[1]=4;// poner en 4 si queres que este un toque arriba desde atras.
    position = position - 20*forward + Up;
    forward = orig-position;
}

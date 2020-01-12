#include "Beluga.h"
#include "../ThreeMaxLoader.h"
#include "../sounds/sounds.h"

extern GLuint _textureRoad;

Beluga::Beluga(int faction) : Balaenidae(faction)
{

}

void Beluga::init()
{
    //Load the model
    _model = (Model*)T3DSModel::loadModel("units/beluga.3ds",-1.4f,0.0f,0.0f,1,_textureRoad);
    if (_model != NULL)
        _model->setAnimation("run");

    setForward(0,0,1);

}

void Beluga::embody(dWorldID world, dSpaceID space)
{
    me = dBodyCreate(world);
    embody(me);
    geom = dCreateBox( space, 100.0f, 58.0f, 500.0f);   // scale 50
    dGeomSetBody(geom, me);

}

void Beluga::embody(dBodyID myBodySelf)
{
    dMass m;

    float myMass = 250.0f;

    dBodySetPosition(myBodySelf, pos[0], pos[1], pos[2]);
    //dMassSetBox(&m,1,1.0f, 4, 5.0f);
    dMassSetBox(&m, 1,100.0f, 58.0f, 5000.0f);
    //dMassSetSphere(&m,1,radius);
    dMassAdjust(&m, myMass*1.0f);
    dBodySetMass(myBodySelf,&m);

    me = myBodySelf;

}

void Beluga::drawModel(float yRot, float xRot, float x, float y, float z)
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
        //drawArrow(V[0],V[1],V[2],0.0,1.0,0.0);

        //drawRectangularBox(100.0f/2.0f, 58.0f/2.0f, 500.0f/2.0f);

        glRotatef(-180.0f, 1.0f, 0.0f, 0.0f);
        glRotatef(-180.0f, 0.0f, 0.0f, 1.0f);
        glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);
        glRotatef(-180.0f, 0.0f, 1.0f, 0.0f);
        glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);

        //glRotatef(yRot, 0.0f, 0.0f, 1.0f);

        //glRotatef(xRot, 1.0f, 0.0f, 0.0f);

        _model->draw();

        //glTranslatef(-0.4f,0.62f,-0.5f);
        //drawTheRectangularBox(_textureRoad,8.0f, 1.0f, 1.0f);



        glPopMatrix();
    }
    else
    {
        printf ("model is null\n");
    }
}

//draw3DSModel("units/beluga.3ds",1200.0+100,15.0,700.0+300.0,1,_textureBox);

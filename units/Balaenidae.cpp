#include "Balaenidae.h"
#include "../ThreeMaxLoader.h"

extern GLuint _textureMetal;

Balaenidae::Balaenidae()
{

}

void Balaenidae::init()
{
    //Load the model
    //_model = MD2Model::load("mantagood.md2");
    //if (_model != NULL)
    //    _model->setAnimation("run");

    setForward(0,0,1);

}

int Balaenidae::getType()
{
    return 4;
}

void Balaenidae::drawModel(float yRot, float xRot, float x, float y, float z)
{
    float f[3];
    f[0] = 0; f[1] = 0; f[2] = 0;

    //Draw the saved model
    //if (_model != NULL)
    //{
        glPushMatrix();
        glTranslatef(x, y, z);

        glScalef(50.0f,20.0f,50.0f);

        doTransform(f, R);

        //drawArrow();
        //drawArrow(S[0],S[1],S[2],1.0,0.0,0.0);
        //drawArrow(V[0],V[1],V[2],0.0,1.0,0.0);

        glRotatef(-180.0f, 1.0f, 0.0f, 0.0f);
        glRotatef(-180.0f, 0.0f, 0.0f, 1.0f);
        glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);
        glRotatef(-180.0f, 0.0f, 1.0f, 0.0f);

        //glRotatef(yRot, 0.0f, 0.0f, 1.0f);

        //glRotatef(xRot, 1.0f, 0.0f, 0.0f);

        //_model->draw();
        draw3DSModel("units/carrier.3ds",0.0f,0.0f,0.0f,1,_textureMetal);

        glPopMatrix();
    //}
    //else
    //{
        //printf ("model is null\n");
    //}
}

void Balaenidae::embody(dWorldID world, dSpaceID space)
{
    me = dBodyCreate(world);
    embody(me);
    //geom = dCreateSphere( space, 2.64f);
    geom = dCreateBox( space, 100.0f, 40, 500.0f);   // scale 50
    dGeomSetBody(geom, me);
}

void Balaenidae::embody(dBodyID myBodySelf)
{
    dMass m;

    float myMass = 250.0f;
    float radius = 2.64f;
    float length = 7.0f;

    dBodySetPosition(myBodySelf, pos[0], pos[1], pos[2]);
    dMassSetBox(&m,1,4.0f,2.64f,10.0f);
    //dMassSetSphere(&m,1,radius);
    dMassAdjust(&m, myMass*1.0f);
    dBodySetMass(myBodySelf,&m);

    me = myBodySelf;

}

void Balaenidae::doControl(Controller controller)
{
    setThrottle(-controller.thrust*2*5);

    Balaenidae::rudder = controller.roll;
}

void Balaenidae::getViewPort(Vec3f &Up, Vec3f &position, Vec3f &forward)
{
    position = getPos();
    forward = getForward();
    Up = toVectorInFixedSystem(0.0f, 1.0f, 0.0f,0,0);

    Vec3f orig;


    forward = forward.normalize();
    orig = position;
    Up[0]=Up[2]=0;Up[1]=100;// poner en 4 si queres que este un toque arriba desde atras.
    position = position - 400*forward + Up;
    forward = orig-position;
}


void Balaenidae::drawModel()
{
    drawModel(0,0,pos[0],pos[1],pos[2]);
}

void Balaenidae::doDynamics()
{
    doDynamics(getBodyID());
}


void Balaenidae::doDynamics(dBodyID body)
{
    Vec3f Ft;

    Ft[0]=0;Ft[1]=0;Ft[2]=getThrottle();
    dBodyAddRelForce(body,Ft[0],Ft[1],Ft[2]);

    dBodyAddRelTorque(body,0.0f,Balaenidae::rudder,0.0f);

    wrapDynamics(body);
}


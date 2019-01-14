#include "Balaenidae.h"
#include "SimplifiedDynamicManta.h"
#include "../ThreeMaxLoader.h"
#include "../odeutils.h"

extern GLuint _textureMetal;

extern GLuint _textureRoad;

Balaenidae::~Balaenidae()
{
    delete _model;
}

Balaenidae::Balaenidae()
{

}

void Balaenidae::init()
{
    //Load the model
    _model = (Model*)T3DSModel::loadModel("units/carrier.3ds",-1.4f,0.0f,0.0f,1,_textureMetal);
    if (_model != NULL)
        _model->setAnimation("run");

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
    if (_model != NULL)
    {
        glPushMatrix();
        glTranslatef(x, y, z);

        glScalef(50.0f,20.0f,50.0f);

        doTransform(f, R);

        //drawArrow();
        //drawArrow(S[0],S[1],S[2],1.0,0.0,0.0);
        //drawArrow(V[0],V[1],V[2],0.0,1.0,0.0);

        //drawRectangularBox(100.0f/50.0f, 40.0f/20.0f, 500.f/50.0f);

        glRotatef(-180.0f, 1.0f, 0.0f, 0.0f);
        glRotatef(-180.0f, 0.0f, 0.0f, 1.0f);
        glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);
        glRotatef(-180.0f, 0.0f, 1.0f, 0.0f);

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

void Balaenidae::embody(dWorldID world, dSpaceID space)
{
    me = dBodyCreate(world);
    embody(me);
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
    //dMassSetBox(&m,1,1.0f, 4, 5.0f);
    dMassSetBox(&m, 1,100.0f, 40, 500.0f);
    //dMassSetSphere(&m,1,radius);
    dMassAdjust(&m, myMass*1.0f);
    dBodySetMass(myBodySelf,&m);

    me = myBodySelf;

}

void Balaenidae::doControl(Controller controller)
{
    if (getThrottle()==0 and controller.registers.thrust != 0)
        honk();
    setThrottle(-controller.registers.thrust*2*5);

    Balaenidae::rudder = -controller.registers.roll;
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

    dBodyAddRelTorque(body,0.0f,Balaenidae::rudder*1000,0.0f);

    wrapDynamics(body);
}

void Balaenidae::doControl()
{
    Controller c;

    c.registers = myCopy;

    c.registers.roll = 1;
    if ((rand() % 100 + 1)<10)
        c.registers.thrust = 20;
    else
        c.registers.thrust = 0;

    doControl(c);
}

Vehicle* Balaenidae::spawn(dWorldID  world,dSpaceID space,int type)
{
    // @FIXME do it depending on the type of the element to spawn.
    SimplifiedDynamicManta *_manta1 = new SimplifiedDynamicManta();
    _manta1->init();
    _manta1->setPos(pos[0],pos[1]+50, pos[2]);
    _manta1->embody(world, space);
    _manta1->setStatus(0);
    _manta1->inert = true;
    alignToMe(_manta1->getBodyID());

    return (Vehicle*)_manta1;
}

void Balaenidae::taxi(Manta *m)
{
    m->setPos(pos[0],pos[1]+50, pos[2]);
}

void Balaenidae::launch(Manta* m)
{
    m->inert = false;
    m->setStatus(2);
    m->elevator = +12;
    struct controlregister c;
    c.thrust = 1500.0f/(-10.0);
    c.pitch = 12;
    m->setControlRegisters(c);
    m->setThrottle(1500.0f);
    Vec3f p = m->getPos();
    p[1] += 10;
    m->setPos(p);
    dBodySetPosition(m->getBodyID(),p[0],p[1],p[2]);
    //dBodySetLinearVel(m->getBodyID(),0.0,0.0,(1.0f)*7000);
}

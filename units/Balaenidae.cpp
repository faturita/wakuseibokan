#include "Balaenidae.h"
#include "SimplifiedDynamicManta.h"
#include "Walrus.h"
#include "../ThreeMaxLoader.h"
#include "../sounds/sounds.h"

extern GLuint _textureMetal;

extern GLuint _textureRoad;

Balaenidae::~Balaenidae()
{
    delete _model;
}

Balaenidae::Balaenidae(int newfaction)
{
    // Choose your own side.
    setFaction(newfaction);
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

void Balaenidae::doControl(struct controlregister regs)
{
    if (getThrottle()==0 and regs.thrust != 0)
        honk();
    setThrottle(regs.thrust*2*5);

    Balaenidae::rudder = -regs.roll;
}

void Balaenidae::doControl(Controller controller)
{
    doControl(controller.registers);
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

    if (offshoring == 1) {
        offshoring=0;
        setStatus(Balaenidae::SAILING);
    }
    else if (offshoring > 0)
    {
        // Add a retractive force to keep it out of the island.
        Vec3f ap = Balaenidae::ap;

        setThrottle(0.0);

        Vec3f V = ap*(-10000);

        dBodyAddRelForce(body,V[0],V[1],V[2]);
        offshoring--;
    }


    // Buyoncy
    //if (pos[1]<0.0f)
    //    dBodyAddRelForce(me,0.0,9.81*20050.0f,0.0);

    dReal *v = (dReal *)dBodyGetLinearVel(body);

    dVector3 O;
    dBodyGetRelPointPos( body, 0,0,0, O);

    dVector3 F;
    dBodyGetRelPointPos( body, 0,0,1, F);

    F[0] = (F[0]-O[0]);
    F[1] = (F[1]-O[1]);
    F[2] = (F[2]-O[2]);

    Vec3f vec3fF;
    vec3fF[0] = F[0];vec3fF[1] = F[1]; vec3fF[2] = F[2];

    Vec3f vec3fV;
    vec3fV[0]= v[0];vec3fV[1] = v[1]; vec3fV[2] = v[2];

    speed = vec3fV.magnitude();

    vec3fV = vec3fV * 0.02f;

    dBodyAddRelForce(body,vec3fV[0],vec3fV[1],vec3fV[2]);

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

void Balaenidae::offshore()
{
    Balaenidae::offshoring = 100;
    Balaenidae::ap = dBodyGetLinearVelInBody(me);
    Balaenidae::ap = Balaenidae::ap.normalize();
}

Vehicle* Balaenidae::spawn(dWorldID  world,dSpaceID space,int type, int number)
{
    Vehicle *v;

    if (type == MANTA)
    {
        SimplifiedDynamicManta *_manta1 = new SimplifiedDynamicManta(getFaction());
        _manta1->init();
        _manta1->setNumber(number);
        _manta1->embody(world, space);
        _manta1->setPos(pos[0],pos[1]+28, pos[2]);
        _manta1->setStatus(Manta::ON_DECK);
        _manta1->inert = true;
        alignToMe(_manta1->getBodyID());
        v = (Vehicle*)_manta1;
    } else if (type == WALRUS)
    {
        Walrus *_walrus = new Walrus(getFaction());
        _walrus->init();
        _walrus->setNumber(number);
        _walrus->embody(world,space);
        Vec3f p;
        p = p.normalize();
        p = getForward()*450;
        _walrus->setPos(pos[0]-p[0],pos[1]-p[1]+40,pos[2]-p[2]);
        _walrus->setStatus(Walrus::SAILING);
        _walrus->stop();
        dBodyAddRelForce(me,10.0f,0.0f,0.0f);

        alignToMe(_walrus->getBodyID());
        v = (Vehicle*)_walrus;
    }

    return v;
}

void Balaenidae::taxi(Manta *m)
{
    m->setPos(pos[0],pos[1]+10, pos[2]);
    dBodySetPosition(m->getBodyID(),pos[0],pos[1]+28,pos[2]);
    m->setStatus(Manta::ON_DECK);
    alignToMe(m->getBodyID());
}

void Balaenidae::launch(Manta* m)
{
    m->inert = true;
    m->setStatus(Manta::FLYING);
    m->elevator = +12;
    struct controlregister c;
    c.thrust = 600.0f/(10.0);
    c.pitch = 12;
    m->setControlRegisters(c);
    m->setThrottle(600.0f);
    Vec3f p = m->getPos();
    p[1] += 20;
    m->setPos(p);
    dBodySetPosition(m->getBodyID(),p[0],p[1],p[2]);
    // @FIXME: Fix the rotation of Manta after it is launched (due to the existence of angularPos in Manta).

    //((SimplifiedDynamicManta*)m)->angularPos[0] = 0;

    Vec3f f;
    f = m->getForward();
    f= Vec3f(0.0f, 0.0f, 1.0f);  // @Hack to avoid the issue of the alignment of manta with the carrier.
    f = f*200;
    dBodySetLinearVel(m->getBodyID(),f[0],f[1],f[2]);
}

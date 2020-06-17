#include "Balaenidae.h"
#include "SimplifiedDynamicManta.h"
#include "Walrus.h"
#include "AdvancedWalrus.h"
#include "../ThreeMaxLoader.h"
#include "../sounds/sounds.h"

extern GLuint _textureMetal;

extern GLuint _textureRoad;

extern std::vector<std::string> messages;
extern std::vector<BoxIsland*> islands;

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

    dBodySetPosition(myBodySelf, pos[0], pos[1], pos[2]);
    //dMassSetBox(&m,1,1.0f, 4, 5.0f);
    dMassSetBox(&m, 1,100.0f, 40, 500.0f);
    //dMassSetSphere(&m,1,radius);
    dMassAdjust(&m, myMass*1.0f);
    dBodySetMass(myBodySelf,&m);

    me = myBodySelf;

}

void Balaenidae::doControl()
{
    Controller c;

    c.registers = myCopy;


    Vec3f Po = getPos();

    Po[1] = 0.0f;

    Vec3f Pf(145 kmf, -1.0f, 89 kmf - 3.5 kmf);

    Pf = destination;

    Vec3f T = Pf - Po;

    float eh, midpointpitch;


    if (!reached && T.magnitude()>500)
    {
        float distance = T.magnitude();

        Vec3f F = getForward();

        F = F.normalize();
        T = T.normalize();


        // Potential fields from the islands (to avoid them)

        int nearesti = 0;
        float closest = 0;
        for(int i=0;i<islands.size();i++)
        {
            BoxIsland *b = islands[i];
            Vec3f l(b->getX(),0.0f,b->getZ());

            if ((l-Po).magnitude()<closest || closest ==0) {
                closest = (l-Po).magnitude();
                nearesti = i;
            }
        }

        c.registers.thrust = 1500.0f;

        if (distance<10000.0f)
        {
            c.registers.thrust = 800.0f;
        }

        if (distance<2000.0f)
        {
            c.registers.thrust = 150.0f;
        }

        if (closest > 1800 && closest < 4000)
        {
            BoxIsland *b = islands[nearesti];
            Vec3f l = b->getPos();
            Vec3f d = Po-l;

            d = d.normalize();

            T = T+d;
            T = T.normalize();

            c.registers.thrust = 45.0f;
        }



        float e = acos(  T.dot(F) );

        float signn = T.cross(F) [1];


        printf("T: %10.3f, %10.3f %10.3f %10.3f\n", closest, distance, e, signn);

        if (abs(e)>=0.5f)
        {
            c.registers.roll = 30.0 * (signn>0?+1:-1) ;
        } else
        if (abs(e)>=0.4f)
        {
            c.registers.roll = 20.0 * (signn>0?+1:-1) ;
        } else
        if (abs(e)>=0.2f)
            c.registers.roll = 10.0 * (signn>0?+1:-1) ;
        else {
            c.registers.roll = 0.0f;
        }


    } else {
        if (!reached)
        {
            char str[256];
            sprintf(str, "Balaenidae has arrived to destination.");
            messages.insert(messages.begin(), str);
            reached = true;
            c.registers.thrust = 0.0f;
            c.registers.roll = 0.0f;
            disableAuto();
        }
    }

    doControl(c);
}

void Balaenidae::doControl(Controller controller)
{
    doControl(controller.registers);
}

void Balaenidae::doControl(struct controlregister regs)
{
    if (getThrottle()==0 and regs.thrust != 0)
        honk();
    setThrottle(regs.thrust*2*5);

    Balaenidae::rudder = -regs.roll;
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


    if (VERIFY(pos, body))
    {
        dBodyAddRelForce(body,Ft[0],Ft[1],Ft[2]);

        dBodyAddRelTorque(body,0.0f,Balaenidae::rudder*1000,0.0f);

        dBodyAddRelForce(body,vec3fV[0],vec3fV[1],vec3fV[2]);
    }


    wrapDynamics(body);
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
        AdvancedWalrus *_walrus = new AdvancedWalrus(getFaction());
        _walrus->init();
        _walrus->setNumber(number);
        _walrus->embody(world,space);
        Vec3f p;
        p = p.normalize();
        p = getForward()*450;
        _walrus->setPos(pos[0]-p[0]-140*(number+1),pos[1]-p[1]+40,pos[2]-p[2]);
        _walrus->setStatus(Walrus::SAILING);
        _walrus->stop();
        _walrus->inert = true;
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
    m->inert = false;
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
    m->release(getForward());
    m->setForward(getForward());

    Vec3f f;
    f = m->getForward();
    //f= Vec3f(0.0f, 0.0f, 1.0f);  // @Hack to avoid the issue of the alignment of manta with the carrier.
    //f = f*200;
    dBodySetLinearVel(m->getBodyID(),f[0],f[1],f[2]);
}

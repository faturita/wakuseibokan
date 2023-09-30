#include "Balaenidae.h"
#include "SimplifiedDynamicManta.h"
#include "Walrus.h"
#include "AdvancedWalrus.h"
#include "Otter.h"
#include "Otter.h"
#include "AdvancedManta.h"
#include "../profiling.h"
#include "../ThreeMaxLoader.h"
#include "../sounds/sounds.h"
#include "../actions/Missile.h"
#include "../actions/AAM.h"
#include "../keplerivworld.h"

extern std::unordered_map<std::string, GLuint> textures;
extern std::vector<Message> messages;
extern std::vector<BoxIsland*> islands;
extern container<Vehicle*> entities;

Balaenidae::~Balaenidae()
{
    // @FIXME Check this.
    //delete _model;
    assert( !"This destructor is not being executed.");


}

Balaenidae::Balaenidae(int newfaction)
{
    // Choose your own side.
    setFaction(newfaction);
}

void Balaenidae::init()
{
    //Load the model
    _model = (Model*)T3DSModel::loadModel("units/carrier.3ds",-1.4f,0.0f,0.0f,1,textures["metal"]);

    setForward(0,0,1);

    setName("Balaenidae");

    width = 100;
    height = 40;
    length = 500;

}

void Balaenidae::clean()
{
    CLog::Write(CLog::Debug,"Carrier: Destructor.\n");
}

int Balaenidae::getType()
{
    return CARRIER;
}

int Balaenidae::getSubType()
{
    return BALAENIDAE;
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

        //drawArrow(10);
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

dSpaceID Balaenidae::embody_in_space(dWorldID world, dSpaceID space)
{
    body_space = dSimpleSpaceCreate (space);
    dSpaceSetCleanup (body_space,1);

    embody(world, body_space);

    return body_space;
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

    c.registers = registers;


    Vec3f Po = getPos();

    Po[1] = 0.0f;

    Vec3f Pf(145 kmf, -1.0f, 89 kmf - 3.5 kmf);

    Pf = destination;

    Vec3f T = Pf - Po;

    if (dst_status != DestinationStatus::REACHED && T.magnitude()>500)
    {
        float distance = T.magnitude();

        Vec3f F = getForward();

        F = F.normalize();
        T = T.normalize();


        // Potential fields from the islands (to avoid them)
        int nearesti = 0;
        float closest = 0;
        for(size_t i=0;i<islands.size();i++)
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

        // Potential fields to avoid islands (works great).
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


        float e = _acos(  T.dot(F) );

        float signn = T.cross(F) [1];

        CLog::Write(CLog::Debug,"T: %10.3f, %10.3f %10.3f %10.3f\n", closest, distance, e, signn);


        /**
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
        **/

        c.registers.roll = abs(e) * (signn>0?+1:-1)  * 40;


    } else {
        if (dst_status != DestinationStatus::REACHED )
        {
            char str[256];
            Message mg;
            mg.faction = getFaction();
            sprintf(str, "%s has arrived to destination.", FACTION(getFaction()));
            mg.msg = std::string(str);
            messages.insert(messages.begin(), mg);
            dst_status = DestinationStatus::REACHED;
            c.registers.thrust = 0.0f;
            setThrottle(0.0);
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

        dBodyAddForce(body,V[0],V[1],V[2]);
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
    Balaenidae::ap =getForward();
    Balaenidae::ap = Balaenidae::ap.normalize();
}

Vehicle* Balaenidae::spawn(dWorldID  world,dSpaceID space,int type, int number)
{
    Vehicle *v = NULL;

    if (type == MANTA)
    {
        AdvancedManta *_manta1 = new AdvancedManta(getFaction());
        _manta1->init();
        _manta1->setNameByNumber(number);
        _manta1->embody(world, space);
        _manta1->setPos(pos[0],pos[1]+28, pos[2]);
        _manta1->setStatus(FlyingStatus::ON_DECK);
        _manta1->inert = true;
        alignToMe(_manta1->getBodyID());
        v = (Vehicle*)_manta1;
    } else if (type == WALRUS)
    {
        Otter *_walrus = new Otter(getFaction());
        _walrus->init();
        dSpaceID car_space = _walrus->embody_in_space(world, space);
        Vec3f p;
        p = p.normalize();
        p = getForward().normalize()*(getDimensions()[2]/2.0+15.0);
        //_walrus->setPos(pos[0]-p[0]-140*(number+1),pos[1]-p[1]+1,pos[2]-p[2]);
        _walrus->setPos(pos[0]-p[0],pos[1]-p[1]+1,pos[2]-p[2]);
        _walrus->stop();
        _walrus->setNameByNumber(number);
        _walrus->setStatus(SailingStatus::SAILING);
        dBodyAddRelForce(me,10.0f,0.0f,0.0f);
        v = (Vehicle*)_walrus;

        p = getForward().normalize()*(1000);
        p = p.rotateOnY(getRandom(-PI/2.0+PI/4.0, PI/2.0 - PI/4));
        p = getPos()-p;

        _walrus->goTo(p);
        _walrus->enableAuto();

        Vec3f dimensions(5.0f,4.0f,10.0f);


        Wheel * _fr= new Wheel(getFaction(), 0.001, 30.0);
        _fr->init();
        _fr->embody(world, car_space);
        _fr->attachTo(world,_walrus,4.9f, -3.0, 5.8);
        _fr->stop();

        entities.push_back(_fr, _fr->getGeom());


        Wheel * _fl= new Wheel(getFaction(), 0.001, 30.0);
        _fl->init();
        _fl->embody(world, car_space);
        _fl->attachTo(world,_walrus, -4.9f, -3.0, 5.8);
        _fl->stop();

        entities.push_back(_fl, _fl->getGeom());


        Wheel * _br= new Wheel(getFaction(), 0.001, 30.0);
        _br->init();
        _br->embody(world, car_space);
        _br->attachTo(world,_walrus, 4.9f, -3.0, -5.8);
        _br->stop();

        entities.push_back(_br, _br->getGeom());

        Wheel * _bl= new Wheel(getFaction(), 0.001, 30.0);
        _bl->init();
        _bl->embody(world, car_space);
        _bl->attachTo(world,_walrus, -4.9f, -3.0, -5.8);
        _bl->stop();

        entities.push_back(_bl, _bl->getGeom());

        _walrus->addWheels(_fl, _fr, _bl, _br);

        _fl->setSteering(true);
        _fr->setSteering(true);

        // Get the alignment of the carrier and invert it for the walrus.
        dMatrix3 Re1,Re2,Re3;
        dQuaternion q1,q2,q3;
        dBodyCopyRotation(me,Re1);
        dRFromAxisAndAngle(Re2,0.0,1.0,0.0,PI);
        dQfromR(q1,Re1);
        dQfromR(q2,Re2);
        dQMultiply0(q3,q1,q2);
        dRfromQ(Re3,q3);
        dBodySetRotation(_walrus->getBodyID(),Re3);

    }

    return v;
}

void Balaenidae::taxi(Manta *m)
{
    m->setPos(pos[0],pos[1]+10, pos[2]);
    dBodySetPosition(m->getBodyID(),pos[0],pos[1]+28,pos[2]);
    m->setStatus(FlyingStatus::ON_DECK);
    alignToMe(m->getBodyID());
}

void Balaenidae::launch(Manta* m)
{
    if (m->getSubType() == CEPHALOPOD)
    {
        m->enableAuto();
    } else
    {
        m->inert = false;
        m->setStatus(FlyingStatus::FLYING);
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
}


Vehicle* Balaenidae::fire(int weapon, dWorldID world, dSpaceID space)
{
    Missile *action = new Missile(getFaction());
    // Need axis conversion.
    action->init();
    action->setOrigin(me);

    Vec3f position = getPos();
    position[1] += .5f; // Move upwards to the center of the real rotation.
    forward = getForward();
    Vec3f Up = toVectorInFixedSystem(0.0f, 1.0f, 0.0f,0,0);

    Vec3f orig;

    forward = forward.normalize();
    orig = position;
    position = position + 60.0f*forward;
    forward = -orig+position;

    Vec3f f1(0.0,0.0,1.0);
    Vec3f f2 = forward.cross(f1);
    f2 = f2.normalize();
    float alpha = _acos( forward.dot(f1)/(f1.magnitude()*forward.magnitude()));

    dMatrix3 Re;
    dRSetIdentity(Re);
    dRFromAxisAndAngle(Re,f2[0],f2[1],f2[2],-alpha);


    position = orig;
    position[1] += 80;
    action->embody(world,space);
    action->setPos(position[0],position[1],position[2]);


    Vec3f Ft=forward + Vec3f(0,20,0);
    Ft=Ft*300;
    //dBodySetLinearVel(action->getBodyID(),Ft[0],Ft[1],Ft[2]);
    dBodyAddForce(action->getBodyID(), Ft[0],Ft[1],Ft[2]);
    dBodySetRotation(action->getBodyID(),Re);

    // I can set power or something here.
    return (Vehicle*)action;
}

std::vector<size_t> Balaenidae::getWeapons()
{
    return weapons;
}

void Balaenidae::addWeapon(size_t w)
{
    weapons.push_back(w);
}

EntityTypeId Balaenidae::getTypeId()
{
    return EntityTypeId::TBalaenidae;
}

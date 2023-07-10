#include "../profiling.h"
#include "../math/yamathutil.h"
#include "../sounds/sounds.h"
#include "../ThreeMaxLoader.h"
#include "../actions/Gunshot.h"
#include <vector>

#ifdef __linux
#include <iomanip>
#elif __APPLE__
#endif

#include "../messages.h"
#include "Cephalopod.h"

extern dWorldID world;
extern dSpaceID space;
#include "../container.h"
#include "../sounds/sounds.h"

extern container<Vehicle*> entities;
extern std::unordered_map<std::string, GLuint> textures;
extern std::vector<Message> messages;

Cephalopod::Cephalopod(int newfaction) : SimplifiedDynamicManta(newfaction)
{

}

void Cephalopod::init()
{
    //draw3DSModel("units/drone.3ds",1200.0+100,15.0,700.0+300.0,1,_textureBox);


    //Load the model
    _model = (Model*)T3DSModel::loadModel("units/drone.3ds",0,-13,0,0.4,1,1,0);
    if (_model != NULL)
    {
        //_topModel = (Model*)T3DSModel::loadModel("structures/turrettop.3ds",0,0,0,0.1,0.1,0.1,0);
    }


    Cephalopod::height=15.0f;
    Cephalopod::width=20.0f;
    Cephalopod::length=32.0f;


    setForward(0,0,1);

    status = 0;
}

void Cephalopod::embody(dWorldID world, dSpaceID space)
{
    me = dBodyCreate(world);
    embody(me);
    //geom = dCreateSphere( space, 2.64f);
    geom = dCreateBox( space, width, height, length);
    dGeomSetBody(geom, me);
}

void Cephalopod::embody(dBodyID myBodySelf)
{
    dMass m;

    float myMass = 20.0f;

    dBodySetPosition(myBodySelf, pos[0], pos[1], pos[2]);
    dMassSetBox(&m,1,width,height,length);
    dMassAdjust(&m, myMass*1.0f);
    dBodySetMass(myBodySelf,&m);

    me = myBodySelf;
}


void Cephalopod::drawModel(float yRot, float xRot, float x, float y, float z)
{
    float f[3];
    f[0] = 0; f[1] = 0; f[2] = 0;

    //Draw the saved model
    if (_model != NULL)
    {
        glPushMatrix();
        glTranslatef(x, y, z);

        glScalef(1.0f,1.0f,1.0f);

        doTransform(f, R);

        //drawArrow();
        //drawArrow(S[0],S[1],S[2],1.0,0.0,0.0);

        // Draw linear velocity
        //drawArrow(V[0],V[1],V[2],0.0,1.0,0.0);

        //drawRectangularBox(width, height, length);

        glRotatef(180.0f, 0.0f, 0.0f, 1.0f);
        glRotatef(-180.0f, 1.0f, 0.0f, 0.0f);
        glRotatef(-180.0f, 0.0f, 1.0f, 0.0f);

        glColor3f(1.0,1.0f,1.0f);
        doMaterial();
        _model->setTexture(textures["metal"]);
        _model->draw();

        glPopMatrix();
    }
    else
    {
        printf ("model is null\n");
    }
}

void  Cephalopod::getViewPort(Vec3f &Up, Vec3f &position, Vec3f &viewforward)
{
    position = getPos();
    viewforward = toVectorInFixedSystem(0,0,1,pan, tilt);                         // I dont care the declination for the viewport

    // ViewForward is in body coordinates, I need to convert it to global coordinates.
    viewforward = toWorld(me, viewforward);

    Up = toVectorInFixedSystem(0.0f, 1.0f, 0.0f,0,0);

    Vec3f orig;


    viewforward = viewforward.normalize();
    orig = position;
    Up[0]=Up[2]=0;Up[1]=20;
    position = position - 90*viewforward + Up;
    viewforward = orig-position;
}

void Cephalopod::doMaterial()
{
    GLfloat specref[] = { 1.0f, 1.0f, 1.0f, 1.0f};

    glEnable(GL_COLOR_MATERIAL);

    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    glMaterialfv(GL_FRONT, GL_SPECULAR, specref);
    glMateriali(GL_FRONT, GL_SHININESS,128);
}

void Cephalopod::doControl(struct controlregister regs)
{
    Manta::doControl(regs);

    Cephalopod::pan = regs.yaw;
    Cephalopod::tilt = regs.bank;
}

int Cephalopod::getSubType()
{
    return CEPHALOPOD;
}

EntityTypeId Cephalopod::getTypeId()
{
    return EntityTypeId::TCephalopod;
}

void Cephalopod::doDynamics()
{
    doDynamics(getBodyID());
}

float Cephalopod::getVerticalAttitude()
{
    dVector3 result;
    dBodyVectorFromWorld(me, 0,1,0,result);

    Vec3f upInTheWorld = Vec3f(result[0],result[1],result[2]);
    Vec3f upInTheBody = Vec3f(0.0f,1.0f,0.0f);

    upInTheWorld = upInTheWorld.normalize();

    //CLog::Write(CLog::Debug,"Angle between vectors %10.5f\n", _acos(upInBody.dot(Up))*180.0/PI);

    float verattitude = _acos(upInTheWorld.dot(upInTheBody))*180.0/PI;

    return verattitude;
}

float Cephalopod::getRollAngle()
{
    dVector3 result;
    dBodyVectorFromWorld(me, 0,1,0,result);

    Vec3f upInTheWorld = Vec3f(result[0],result[1],result[2]);
    Vec3f upInTheBody = Vec3f(0.0f,1.0f,0.0f);

    upInTheWorld = upInTheWorld.normalize();

    Vec3f upInTheWorldProjection2 = Vec3f(upInTheWorld[0], upInTheWorld[1], 0.0);

    upInTheWorldProjection2 = upInTheWorldProjection2.normalize();

    float rollattitude = _acos( upInTheWorldProjection2.dot(upInTheBody))*180.0/PI;

    return rollattitude * sgn(upInTheWorldProjection2[0]);
}

float Cephalopod::getPitchAngle()
{
    dVector3 result;
    dBodyVectorFromWorld(me, 0,1,0,result);

    Vec3f upInTheWorld = Vec3f(result[0],result[1],result[2]);
    Vec3f upInTheBody = Vec3f(0.0f,1.0f,0.0f);

    upInTheWorld = upInTheWorld.normalize();

    Vec3f upInTheWorldProjection1 = Vec3f(0.0,upInTheWorld[1],upInTheWorld[2]);

    upInTheWorldProjection1 = upInTheWorldProjection1.normalize();

    float pitchattitude = _acos( upInTheWorldProjection1.dot(upInTheBody))*180.0/PI;

    return pitchattitude * sgn(upInTheWorldProjection1[2]);
}

void Cephalopod::doDynamics(dBodyID body)
{
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


    dVector3 result;
    dBodyVectorFromWorld(body, 0,1,0,result);

    Vec3f upInTheWorld = Vec3f(result[0],result[1],result[2]);
    Vec3f upInTheBody = Vec3f(0.0f,1.0f,0.0f);

    upInTheWorld = upInTheWorld.normalize();

    //CLog::Write(CLog::Debug,"Angle between vectors %10.5f\n", _acos(upInBody.dot(Up))*180.0/PI);

    float attitude = _acos(upInTheWorld.dot(upInTheBody))*180.0/PI;

    //dout << "Attitude:" << attitude << std::endl;

    Vec3f upInTheWorldProjection1 = Vec3f(0.0,upInTheWorld[1],upInTheWorld[2]);

    upInTheWorldProjection1 = upInTheWorldProjection1.normalize();

    float pitchattitude = _acos( upInTheWorldProjection1.dot(upInTheBody))*180.0/PI;



    Vec3f upInTheWorldProjection2 = Vec3f(upInTheWorld[0], upInTheWorld[1], 0.0);

    upInTheWorldProjection2 = upInTheWorldProjection2.normalize();

    float rollattitude = _acos( upInTheWorldProjection2.dot(upInTheBody))*180.0/PI;


    if (speed > 10 && speed <32 )
    {
        //droneflying();
        //island = NULL;  // This is not necessary but my bring some issues when the drone bounces.

    }

    if (height > 30 && getStatus() == FlyingStatus::LANDED)
    {
        setStatus(FlyingStatus::FLYING);
        island = NULL;
    }

    Vec3f upInBody = Vec3f(result[0],result[1],result[2]);
    Vec3f Up = Vec3f(0.0f,1.0f,0.0f);

    upInBody = upInBody.normalize();

    //CLog::Write(CLog::Debug,"Angle between vectors %10.5f\n", _acos(upInBody.dot(Up))*180.0/PI);

    float attitudes = _acos(upInBody.dot(Up))*180.0/PI;

    if (attitudes>70 || attitudes<-70)
    {
        // Cephalopod has tumbled.
        damage(1);
    }

    if (VERIFY(pos,me) && !Vehicle::inert)
    {
        // Drone stability
        float e1 = pitchattitude;
        float e2 = rollattitude;


        float pitchtorque = 0;
        if ((abs(e1))>10.0f)
        {
            pitchtorque =  0.0+1.0 * (upInTheWorldProjection1[2]>0 ? +5 : -5);
        }

        float rolltorque = 0;
        if ((abs(e2))>10.0f)
        {
            rolltorque =  0.0+1.0 * (upInTheWorldProjection2[0]>0 ? -5 : +5);
        }

        dBodyAddRelTorque(body,pitchtorque, 0,0);
        dBodyAddRelTorque(body,0, 0,rolltorque);

        dout << "Pitch Deviation:" << pitchattitude << "(" << pitchtorque << ")" <<
                ":" << " roll deviation:" << rollattitude << "(" << rolltorque << ")" <<
                std::endl;


        if (attitude < 45)
        {
            //dBodyAddRelForce (body,0, 0,getThrottle());

            dBodyAddRelTorque(body, 0, -aileron*16, 0);


            //Vec3f p1(-rudder*4, getThrottle(),-elevator*10);
            //Vec3f p2(-rudder*4, getThrottle(),-elevator*10);

            //p = toVectorInFixedSystem(p[0],p[1],p[2],aileron*10,elevator*10);

            //dBodyAddRelTorque(body,rudder, 0,0);

            // Roll
            //dBodyAddRelTorque(body, 0,0, aileron);

            Vec3f p1(0,1,0);
            Vec3f p2(0,1,0);

            // In degrees.  The max value of the rotor axis inclination is 90 degrees (body coordinates)
            float inclination = clipped(elevator*10, -90, +90);

            CLog::Write(CLog::Debug,"Inclination %10.5f\n", inclination);

            p1 = toVectorInFixedSystem(p1[0],p1[1],p1[2],0*10,inclination);
            p2 = toVectorInFixedSystem(p2[0],p2[1],p2[2],0*10,inclination);

            p1 = p1.normalize();
            p2 = p2.normalize();

            p1 = p1*getThrottle();
            p2 = p2*getThrottle();

            dBodyAddRelForceAtRelPos(body,p1[0], p1[1], p1[2], 0.0, +35, 0.0);
            dBodyAddRelForceAtRelPos(body,p2[0], p2[1], p2[2], 0.0, -35, 0.0);
        }
    }

    wrapDynamics(body);
}



Vehicle* Cephalopod::fire(int weapon, dWorldID world, dSpaceID space)
{
    Gunshot *action = new Gunshot();
    // Need axis conversion.
    action->init();
    action->setOrigin(me);


    Vec3f position = getPos();
    position[1] += 0.0f; // Move upwards to the center of the real rotation.
    // Check where are we aiming in body coordinates.
    forward = toVectorInFixedSystem(0,0,1,pan, tilt);
    dVector3 result;
    dBodyVectorToWorld(me, forward[0],forward[1],forward[2],result);

    forward = Vec3f(result[0],result[1],result[2]);
    Vec3f Up = toVectorInFixedSystem(0.0f, 1.0f, 0.0f,0,0);

    Vec3f orig;


    forward = forward.normalize();
    orig = position;
    position = position + 60.0f*forward;
    forward = -orig+position;

    Vec3f Ft = forward*100;

    Vec3f f1(0.0,0.0,1.0);
    Vec3f f2 = forward.cross(f1);
    f2 = f2.normalize();
    float alpha = _acos( forward.dot(f1)/(f1.magnitude()*forward.magnitude()));

    dMatrix3 Re;
    dRSetIdentity(Re);
    dRFromAxisAndAngle(Re,f2[0],f2[1],f2[2],-alpha);


    action->embody(world,space);
    action->setPos(position[0],position[1],position[2]);
    dBodySetLinearVel(action->getBodyID(),Ft[0],Ft[1],Ft[2]);
    dBodySetRotation(action->getBodyID(),Re);

    // I can set power or something here.
    return (Vehicle*)action;
}


void Cephalopod::drop()
{
    autostatus = AutoStatus::DROP;
}

void Cephalopod::doControlDrop()
{
    Controller c;

    c.registers = registers;

    Vec3f Po = getPos();

    float height = Po[1];

    float thrust = 40.0;

    c.registers.thrust = thrust/(10.0);
    c.registers.yaw = 0;
    c.registers.roll = 0;
    setThrottle(thrust);

    Manta::doControl(c);
}



void Cephalopod::hoover(float sp3)
{
    Controller c;

    c.registers = registers;

    Vec3f Po = getPos();

    float height = Po[1];

    if (dst_status != DestinationStatus::REACHED)
    {
        char msg[256];
        Message mg;
        sprintf(msg, "%s has arrived to destination.", getName().c_str());
        mg.faction = getFaction();
        mg.msg = std::string(msg);
        messages.insert(messages.begin(), mg);
        dst_status = DestinationStatus::REACHED;
        setStatus(FlyingStatus::HOLDING);
    }
    // Hoover
    float thrust = 200;

    if (height > (sp3 - 10))
    {
        thrust = 98.1;
    }

    c.registers.thrust = thrust/(10.0);
    c.registers.yaw = 0;
    c.registers.roll = 0;
    setThrottle(thrust);

    Manta::doControl(c);
}

void Cephalopod::doControl()
{
    switch (autostatus) {
    case AutoStatus::DOGFIGHT:
        doControlDogFight();
        break;
    case AutoStatus::ATTACK:
        doControlAttack();
        break;
    case AutoStatus::LANDING:
        doControlLanding();
        break;
    case AutoStatus::DROP:
        doControlDrop();
        break;
    default:case AutoStatus::DESTINATION:
        doControlDestination(destination,1000);
        break;
    case AutoStatus::FREE:
        goTo(getPos()+getForward().normalize()*100);
        setAutoStatus(AutoStatus::DESTINATION);
        break;
    }

}

void Cephalopod::doControlAttack()
{
    dout << "A:" << std::setw(3) << getName() << std::setw(11) << destination << std::setw(3) << flyingstate << std::endl;

    Vec3f target = destination;
    switch (flyingstate) {
        default:case 0:
        {
            target = Vec3f(destination[0],400,destination[2]);
            doControlDestination(target,1000);
            Vec3f loc(destination[0],400,destination[2]);
            Vec3f ploc(getPos()[0],400,getPos()[2]);
            if ((loc-ploc).magnitude()<2500)
                flyingstate = 1;
        }
        break;
        case 1:
        {
            Vec3f target = destination - getPos();
            Vec3f aim = toBody(me,target);

            dout << "Attack Target:" << aim <<  ":Loc: " << getPos() << " Target: " << destination<< std::endl;


            float azimuth=getAzimuth(aim);
            float declination=getDeclination(aim);

            struct controlregister c;
            c = getControlRegisters();
            c.yaw = azimuth;
            c.bank = -declination;
            c.precesion = c.roll = c.pitch = 0;
            c.thrust = 10;
            setControlRegisters(c);
            if (getTtl() % 23 == 0)
            {

                Vehicle *action = fire(0,world,space);

                if (action != NULL)
                {
                    entities.push_at_the_back(action,action->getGeom());
                    gunshot();
                }

            }
            doControl(c);
        }
        break;
    }

}

void Cephalopod::doControlLanding()
{
    doControlDestination(destination, 350);

    if (dst_status == DestinationStatus::REACHED)
    {
        Controller c;

        c.registers = registers;
        float thrust = 0;

        c.registers.thrust = thrust/(10.0);
        c.registers.yaw = 0;
        c.registers.roll = 0;
        setThrottle(thrust);

        setStatus(FlyingStatus::DOCKING);

        Manta::doControl(c);
    }
}
void Cephalopod::doControlDestination(Vec3f target, float threshold)
{
    Controller c;

    c.registers = registers;

    Vec3f Po = getPos();

    float height = Po[1];

    float declination = getDeclination(getForward());

    float sp2=-0,        sp3 = 300;

    Vec3f T = (target - Po);

    sp2 = getDeclination(T);

    float e1 = _acos(  T.normalize().dot(getForward().normalize()) );
    float e2 = sp2 - declination;
    float e3 = sp3 - height;

    dout << "T:Az:"
              << std::setw(10) << getAzimuth(getForward())
              << std::setw(10) << getAzimuth(T)
              << "(" << std::setw(12) << e1 << ")"
              << std::setw(10) << declination
              << std::setw(10) << sp2
              << " Destination:"
              << std::setw(10) << T.magnitude() << std::endl;

    et1 = e1;
    et2 = e2;
    et3 = e3;


    dout << "Destination:" << T.magnitude() << std::setw(11) << target << std::endl;

    c.registers.roll = 0;
    if (height > 200)
    {
        if (!(dst_status != DestinationStatus::REACHED && map(T).magnitude()>threshold))
        {
            hoover(sp3);
            return;
        }
        c.registers.pitch = -4.5;

        if (map(T).magnitude()<3500) c.registers.pitch = -4.5;

        if (getAzimuth(getForward())>270 && getAzimuth(T)<(getAzimuth(getForward())-180))
            e1 = e1 * (-1);
        else if (getAzimuth(getForward()) < getAzimuth(T) && (getAzimuth(T) - getAzimuth(getForward()))<180)
            e1 = e1 * (-1);

        if ((abs(e1))>0.2f)
        {
            c.registers.roll =  (e1>0 ? -5 : +5);
        }
    }

    // Add Hoovering
    float thrust = 200;

    if (height > (sp3 - 10))
    {
        thrust = 98.1;
    }

    c.registers.thrust = thrust/(10.0);
    c.registers.yaw = 0;
    setThrottle(thrust);

    Manta::doControl(c);
}

BoxIsland *Cephalopod::getIsland() const
{
    return island;
}

void Cephalopod::setIsland(BoxIsland *value)
{
    island = value;
}

void Cephalopod::setNameByNumber(int number)
{
    setNumber(number);
    setName("Cephalopod",number);
}

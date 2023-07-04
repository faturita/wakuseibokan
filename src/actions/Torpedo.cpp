#include <unordered_map>
#include "../ThreeMaxLoader.h"
#include "../control.h"

#include "../actions/Gunshot.h"

#include "../math/yamathutil.h"

#include "../profiling.h"

#include "Torpedo.h"

extern std::unordered_map<std::string, GLuint> textures;

Torpedo::Torpedo(int faction)
{
    Vehicle::setTtl(1500);
    setFaction(faction);
}

Torpedo::~Torpedo()
{
    dGeomDestroy(geom);
    dBodyDestroy(me);

    assert(0 || !"Destroying bullets from the Gunshot object. This should not happen now.");
}

void Torpedo::init()
{
    // -130.46696	 180.85544	   5.74906
    _model = (Model*)T3DSModel::loadModel("units/missile.3ds",130.46696,-5.74,178.85544,1,1,1,0);   // 130 180
    if (_model != NULL)
    {

    }

    Torpedo::height=0.5f;
    Torpedo::length=7.0f;
    Torpedo::width=1.0f;

    // Let's make Torpedos bigger so that they can more easily hit the target.
    Torpedo::height=0.5f;
    Torpedo::length=7.0f;
    Torpedo::width=10.0f;

    Torpedo::mass = 1.0f;

    setDamage(10000);
    setName("Torpedo");

    setForward(0,0,1);

    visible = true;
}

void Torpedo::drawModel()
{
    drawModel(0,0,pos[0],pos[1],pos[2]);
}

void Torpedo::drawModel(float yRot, float xRot, float x, float y, float z)
{
    float f[3];
    f[0] = 0; f[1] = 0; f[2] = 0;

    //Draw the saved model
    if (visible)
    {
        glPushMatrix();
        glTranslatef(x, y, z);

        doTransform(f, R);

        glScalef(1.5f,1.5f,1.5f);

        doMaterial();

        //drawRectangularBox(width, height, length);
        _model->setTexture(textures["road"]);
        _model->draw();

        glPopMatrix();
    }
    //else
    //{
    //    printf ("model is null\n");
    //}
}

void Torpedo::doMaterial()
{
    GLfloat specref[] = { 1.0f, 1.0f, 1.0f, 1.0f};

    glEnable(GL_COLOR_MATERIAL);

    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    glMaterialfv(GL_FRONT, GL_SPECULAR, specref);
    glMateriali(GL_FRONT, GL_SHININESS,128);
}


void Torpedo::doDynamics(dBodyID body)
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

    Vec3f upInBody = Vec3f(result[0],result[1],result[2]);
    Vec3f Up = Vec3f(0.0f,1.0f,0.0f);

    upInBody = upInBody.normalize();

    //CLog::Write(CLog::Debug,"Angle between vectors %10.5f\n", _acos(upInBody.dot(Up))*180.0/PI);

    float attitude = _acos(upInBody.dot(Up))*180.0/PI;


    if (VERIFY(pos,me) && !Vehicle::inert)
    {
        //dBodyAddRelForce (body,0, 0,getThrottle());

        //dBodyAddRelTorque(body, 0, -xRotAngle*0.1, 0);
        Vec3f p(0.0, 0.0, getThrottle());

        p = toVectorInFixedSystem(p[0],p[1],p[2],-xRotAngle*0.1, 0.0);

        dBodyAddRelForceAtRelPos(body,p[0], p[1], p[2], 0.0, -0.8, -3.4);
    }

    wrapDynamics(body);
}


void Torpedo::embody(dWorldID world, dSpaceID space)
{
    me = dBodyCreate(world);
    embody(me);
    //geom = dCreateSphere( space, 2.64f);
    geom = dCreateBox( space, Torpedo::width, Torpedo::height, Torpedo::length);
    dGeomSetBody(geom, me);
}

void Torpedo::embody(dBodyID myBodySelf)
{
    dMass m;

    float myMass = Torpedo::mass;

    dBodySetPosition(myBodySelf, pos[0], pos[1], pos[2]);
    dMassSetBox(&m, 1,Torpedo::width, Torpedo::height, Torpedo::length);
    dMassAdjust(&m, myMass*1.0f);
    dBodySetMass(myBodySelf,&m);

    me = myBodySelf;

}

int Torpedo::getType()
{
    return CONTROLABLEACTION;
}

void Torpedo::release(Vec3f orientation)
{
    Vehicle::inert = false;
}

void Torpedo::doControl(Controller controller)
{

    doControl(controller.registers);

}

void Torpedo::doControl()
{
    Controller c;

    c.registers = registers;


    Vec3f Po = getPos();

    Po[1] = 0.0f;

    Vec3f Pf = destination;

    Vec3f T = Pf - Po;

    if (dst_status != DestinationStatus::REACHED && T.magnitude()>100)
    {
        float distance = T.magnitude();

        Vec3f F = getForward();

        F = F.normalize();
        T = T.normalize();


        // @FIXME: This bang-bang controller really sucks.  It can be improved.
        float e = _acos(  T.dot(F) );

        float signn = T.cross(F) [1];


        c.registers.roll = abs(e) * (signn>0?+1:-1)  * 20;
        c.registers.thrust = 40-abs(e) * 10;

        // This is a little caveat that I allow myself to do.  When the angle of the target vs forward is small, stop it just once.
        if (abs(e)<0.1f)
        {
            runonce { stop(); }
        }

        CLog::Write(CLog::Debug,"T: %10.3f %10.3f %10.3f %10.3f\n", distance, e, signn, c.registers.thrust);


    } else {
        if (dst_status != DestinationStatus::REACHED)
        {
            dst_status = DestinationStatus::REACHED;
            autostatus = AutoStatus::FREE;
        }
    }

    doControl(c);
}

void Torpedo::doControl(struct controlregister conts)
{
    // Thrust is limited !  It cannot be unbounded !
    if (conts.thrust>20.0f)
        conts.thrust=20.0f;

    setThrottle(conts.thrust);

    xRotAngle = conts.roll;

    registers.thrust = getThrottle();

    registers = conts;

}

void Torpedo::setVisible(bool val)
{
    Gunshot::visible = val;
    setTtl(50);
}

EntityTypeId Torpedo::getTypeId()
{
    return EntityTypeId::TTorpedo;
}


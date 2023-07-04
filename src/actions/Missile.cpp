#include <unordered_map>
#include "Missile.h"
#include "../ThreeMaxLoader.h"
#include "../profiling.h"

extern std::unordered_map<std::string, GLuint> textures;

Missile::Missile(int faction)
{
    Vehicle::setTtl(1000);
    setFaction(faction);
}

Missile::~Missile()
{
    dGeomDestroy(geom);
    dBodyDestroy(me);

    assert(0 || !"Destroying bullets from the Gunshot object. This should not happen now.");
}

void Missile::clean()
{
    smoke.clean();
}

void Missile::init()
{
    // -130.46696	 180.85544	   5.74906
    _model = (Model*)T3DSModel::loadModel("units/missile.3ds",130.46696,-5.74,180.85544,1,1,1,0);   // 130 180
    if (_model != NULL)
    {

    }

    Missile::height=1.0f;
    Missile::length=5.0f;
    Missile::width=1.0f;

    Missile::mass = 1.0f;

    setDamage(10000);
    setName("Missile");

    setForward(0,0,1);
}

void Missile::drawModel()
{
    drawModel(0,0,pos[0],pos[1],pos[2]);
}

void Missile::drawModel(float yRot, float xRot, float x, float y, float z)
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

        smoke.drawModel(getPos(),getForward());
    }
    //else
    //{
    //    printf ("model is null\n");
    //}
}

void Missile::doMaterial()
{
    GLfloat specref[] = { 1.0f, 1.0f, 1.0f, 1.0f};

    glEnable(GL_COLOR_MATERIAL);

    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    glMaterialfv(GL_FRONT, GL_SPECULAR, specref);
    glMateriali(GL_FRONT, GL_SHININESS,128);
}

void Missile::doDynamics(dBodyID body)
{
    dBodyAddForce(body,0,9.81f,0);

    Vec3f p(0.0, 0.0, getThrottle());

    p = toVectorInFixedSystem(p[0],p[1],p[2],rudder, elevator);

    dBodyAddRelForceAtRelPos(body,p[0], p[1], p[2], 0.0, 0.0, -length - 0.1);


    // @NOTE: Bullets are really unstable.
    if (VERIFY(pos, body))
        wrapDynamics(body);
}


void Missile::embody(dWorldID world, dSpaceID space)
{
    me = dBodyCreate(world);
    embody(me);
    //geom = dCreateSphere( space, 2.64f);
    geom = dCreateBox( space, Missile::width, Missile::height, Missile::length);
    dGeomSetBody(geom, me);
}

void Missile::embody(dBodyID myBodySelf)
{
    dMass m;

    float myMass = Missile::mass;

    dBodySetPosition(myBodySelf, pos[0], pos[1], pos[2]);
    dMassSetBox(&m, 1,Missile::width, Missile::height, Missile::length);
    dMassAdjust(&m, myMass*1.0f);
    dBodySetMass(myBodySelf,&m);

    me = myBodySelf;

}

int Missile::getType()
{
    return CONTROLABLEACTION;
}

void Missile::doControlControl2(Vec3f target, float thrust)
{

    Controller c;

    c.registers = registers;

    Vec3f fwd = getForward();

    Vec3f Po = getPos();

    float height = Po[1];
    float declination = getDeclination(fwd);

    float sp2=-0,        sp3 = 720;

    Vec3f T = (target - Po);

    sp2 = getDeclination(T);

    // This function returns principal arc cosine of x, in the interval [0, pi] radians.
    float e1 = _acos(  T.normalize().dot(fwd.normalize()) );
    float e2 = sp2 - declination;
    float e3 = sp3 - height;

    // Set the sign of e1 in relation to rolling encoding (@NOTE Check Manta control is basically the same code).
    if (getAzimuth(getForward())>270 && getAzimuth(T)<(getAzimuth(getForward())-180))
        e1 = e1 * (-1);
    else if (getAzimuth(getForward()) < getAzimuth(T) && (getAzimuth(T) - getAzimuth(getForward()))<180)
        e1 = e1 * (-1);


    float Kp1 = 14.0,        Kp2 = 01.5,            Kp3 = 1.2;
    float Ki1 = 12.2,        Ki2 = 00.0,            Ki3 = 0.6;
    float Kd1 = 67.3,        Kd2 = 00.0,            Kd3 = 11.8;

    float r1 =  rt1 + Kp1 * (e1 - et1)        + Ki1 * (e1 + et1)/2.0 + Kd1 * (e1 - 2 * et1 + ett1);
    float r2 =  rt2 + Kp2 * (e2 - et2)        + Ki2 * (e2 + et2)/2.0 + Kd2 * (e2 - 2 * et2 + ett2);
    float r3 =  rt3 + Kp3 * (e3 - et3)        + Ki3 * (e3 + et3)/2.0 + Kd3 * (e3 - 2 * et3 + ett3);

    dout << "Azimuth:" << getAzimuth(fwd) << "/" << e1 << "- Declination: " << declination << "/" << sp2 << " Destination:" << T.magnitude() << std::endl;


    if (abs((getAzimuth(fwd)-getAzimuth(T)))<0.1) {
        dBodySetAngularVel(me,0,0,0);
        runonceinclass { dout << "Locked in target!" << std::endl; dBodySetLinearVel(me,0,0,0); }
        r1=0;}
    //if (abs((getDeclination(getForward())-getDeclination(T)))<0.1) { //dBodySetAngularVel(me,0,0,0);
    //    r2=0;}

    r1 = clipmax(r1, 2);
    r1 = clipmin(r1, -2);
    r2 = clipmax(r2, 8);
    r2 = clipmin(r2, -8);
    r3 = clipmax(r3, 20);
    r3 = clipmin(r3, -20);



    rt1 = r1;
    rt2 = r2;
    rt3 = r3;


    float roll = -r1;
    float pitch = -r2 + r3*0;

    //float thrust = (e3>800? e3 :800);

    ett1 = et1;
    ett2 = et2;
    ett3 = et3;

    et1 = e1;
    et2 = e2;
    et3 = e3;


    c.registers.thrust = thrust;
    c.registers.pitch = pitch;
    c.registers.roll = roll;
    c.registers.yaw = 0;
    setThrottle(thrust);

    doControl(c);

}


void Missile::doControl()
{

    Controller c;

    c.registers = registers;

    c.registers.thrust = 100;

    //Vec3f p = getPos() - destination;

    //dout << "Destination:" << p.magnitude() << std::endl;

    //c.registers.roll = 0;
    //c.registers.pitch = 0;

    doControlControl2(destination,100);


    //doControl(c.registers);



}

void Missile::doControl(Controller controller)
{

    doControl(controller.registers);

    /**
    Vec3f target = destination;

    Vec3f Po = getPos();

    float height = Po[1];
    float declination = getDeclination(getForward());

    float sp2=-0,        sp3 = 720;

    Vec3f T = (target - Po);

    float e1 = _acos(  T.normalize().dot(getForward().normalize()) );

    // Set the sign of e1 in relation to rolling encoding.
    if (getAzimuth(getForward())>270 && getAzimuth(T)<(getAzimuth(getForward())-180))
        e1 = e1 * (-1);
    else if (getAzimuth(getForward()) < getAzimuth(T))
        e1 = e1 * (-1);


    dout << "Destination:" << T.magnitude() << " Azimuth:" << getAzimuth(T) << " vs " << getAzimuth(getForward()) << "("<< e1 << ")" << std::endl;
**/

}

void Missile::doControl(struct controlregister conts)
{

    elevator    = -conts.pitch * 0.01;
    rudder      = -conts.roll  * 0.01;

    setThrottle(100.0);

    registers = conts;

}

void Missile::setVisible(bool val)
{
    Gunshot::visible = val;
    setTtl(50);
}

EntityTypeId Missile::getTypeId()
{
    return EntityTypeId::TMissile;
}



//draw3DSModel("units/missile.3ds",1200.0+100,15.0,700.0+300.0,1,_textureBox);

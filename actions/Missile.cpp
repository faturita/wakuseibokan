#include "Missile.h"
#include "../ThreeMaxLoader.h"

extern GLuint _textureRoad;

Missile::Missile()
{
    Vehicle::setTtl(50000);
}

Missile::~Missile()
{
    dGeomDestroy(geom);
    dBodyDestroy(me);

    assert(0 || !"Destroying bullets from the Gunshot object. This should not happen now.");
}

void Missile::init()
{
    // -130.46696	 180.85544	   5.74906
    _model = (Model*)T3DSModel::loadModel("units/missile.3ds",130.46696,-5.74,180.85544,1,1,1,0);   // 130 180
    if (_model != NULL)
        _model->setAnimation("run");

    Missile::height=4.0f;
    Missile::length=4.0f;
    Missile::width=4.0f;

    Missile::mass = 1.0f;

    setDamage(10000);

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

        Vec3f p(0.0, 0.0, getThrottle());

        p = toVectorInFixedSystem(p[0],p[1],p[2],rudder, elevator);

        drawArrow(p[0],p[1],p[2],1,0,0,1);

        glScalef(1.5f,1.5f,1.5f);

        doMaterial();

        _model->setTexture(_textureRoad);
        _model->draw();

        glPopMatrix();
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

    dBodyAddRelForceAtRelPos(body,p[0], p[1], p[2], 0.0, 0.0, -3.9);


    // @FIXME: Bullets are really unstable.
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

    c.registers = myCopy;

    Vec3f Po = getPos();

    float height = Po[1];
    float declination = getDeclination(getForward());

    float sp2=-0,        sp3 = 720;

    Vec3f T = (target - Po);


    std::cout << "Destination:" << T.magnitude() << std::endl;

    //if (map(T).magnitude()<3500) thrust = 2.0;


    sp2 = getDeclination(T);

    // This function returns principal arc cosine of x, in the interval [0, pi] radians.
    float e1 = acos(  T.normalize().dot(getForward().normalize()) );
    float e2 = sp2 - declination;
    float e3 = sp3 - height;

    // Set the sign of e1 in relation to rolling encoding.
    if (getAzimuth(getForward())>270 && getAzimuth(T)<(getAzimuth(getForward())-180))
        e1 = e1 * (-1);
    else if (getAzimuth(getForward()) < getAzimuth(T))
        e1 = e1 * (-1);


    float Kp1 = 39.2,        Kp2 = 1.3,          Kp3 = 1.2;
    float Ki1 = 14.9,        Ki2 = 1.6,          Ki3 = 0.6;
    float Kd1 = 19.3,        Kd2 = 1.2,            Kd3 = 11.8;


    float e[3] = { e1, e2, e3 };


    float r1 =  rt1 + Kp1 * (e1 - et1)        + Ki1 * (e1 + et1)/2.0 + Kd1 * (e1 - 2 * et1 + ett1);
    float r2 =  rt2 + Kp2 * (e2 - et2)        + Ki2 * (e2 + et2)/2.0 + Kd2 * (e2 - 2 * et2 + ett2);
    float r3 =  rt3 + Kp3 * (e3 - et3)        + Ki3 * (e3 + et3)/2.0 + Kd3 * (e3 - 2 * et3 + ett3);

    std::cout << "Azimuth:" << getAzimuth(getForward()) << "/" << e1 << "- Declination: " << declination << "/" << sp2 << " Destination:" << T.magnitude() << std::endl;

    r1 = max(r1, 2);
    r1 = min(r1, -2);
    r2 = max(r2, 2);
    r2 = min(r2, -2);
    r3 = max(r3, 20);
    r3 = min(r3, -20);



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

    c.registers = myCopy;

    c.registers.thrust = 5;

    //Vec3f p = getPos() - destination;

    //std::cout << "Destination:" << p.magnitude() << std::endl;

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

    float e1 = acos(  T.normalize().dot(getForward().normalize()) );

    // Set the sign of e1 in relation to rolling encoding.
    if (getAzimuth(getForward())>270 && getAzimuth(T)<(getAzimuth(getForward())-180))
        e1 = e1 * (-1);
    else if (getAzimuth(getForward()) < getAzimuth(T))
        e1 = e1 * (-1);


    std::cout << "Destination:" << T.magnitude() << " Azimuth:" << getAzimuth(T) << " vs " << getAzimuth(getForward()) << "("<< e1 << ")" << std::endl;
**/

}

void Missile::doControl(struct controlregister conts)
{

    elevator    = -conts.pitch * 0.01;
    rudder      = -conts.roll  * 0.01;

    setThrottle(conts.thrust);

}

//draw3DSModel("units/missile.3ds",1200.0+100,15.0,700.0+300.0,1,_textureBox);

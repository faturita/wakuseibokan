#include <iomanip>
#include "AAM.h"
#include "../ThreeMaxLoader.h"
#include "../control.h"

#include "../actions/Gunshot.h"

#include "../math/yamathutil.h"

#include "../profiling.h"

extern std::unordered_map<std::string, GLuint> textures;

AAM::AAM(int faction)
{
    Vehicle::setTtl(1000);
    setFaction(faction);
}

AAM::~AAM()
{
    dGeomDestroy(geom);
    dBodyDestroy(me);

    assert(0 || !"Destroying bullets from the Gunshot object. This should not happen now.");
}

void AAM::clean()
{
    smoke.clean();
}

void AAM::init()
{
    // -130.46696	 180.85544	   5.74906
    _model = (Model*)T3DSModel::loadModel("units/missile.3ds",130.46696,-5.74,180.85544,1,1,1,0);   // 130 180
    if (_model != NULL)
    {

    }

    AAM::height=10.0f;
    AAM::length=50.0f;
    AAM::width=10.0f;

    AAM::mass = 1.0f;

    setDamage(10000);

    setName("AAM");

    setForward(0,0,1);
}

void AAM::drawModel()
{
    drawModel(0,0,pos[0],pos[1],pos[2]);
}

void AAM::drawModel(float yRot, float xRot, float x, float y, float z)
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

void AAM::doMaterial()
{
    GLfloat specref[] = { 1.0f, 1.0f, 1.0f, 1.0f};

    glEnable(GL_COLOR_MATERIAL);

    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    glMaterialfv(GL_FRONT, GL_SPECULAR, specref);
    glMateriali(GL_FRONT, GL_SHININESS,128);
}

void AAM::rotateBody(dBodyID body)
{
    dMatrix3 R;
    dRSetIdentity(R);

    //dRFromEulerAngles (R, Manta::elevator*0.005,0,
    //                  -Manta::aileron*0.01);

    angularPos += (rudder);
   // angularPos[0] -= (Manta::aileron*0.001);

    dQuaternion q;


    dRFromAxisAndAngle(R,0,1,0,angularPos);
    dQfromR(q,R);

    //dQMultiply0(q3,q2,q1);

    float x = getAzimuthRadians(getForward());

    //dout << "Bearing:" << getBearing() << "," << x << " Anglular Pos:" << angularPos[0] << std::endl;

    //if (!Vehicle::inert)
        dBodySetQuaternion(body,q);

}

void AAM::doDynamics(dBodyID body)
{
    dBodyAddForce(body,0,9.81f,0);

    Vec3f p(0.0, 0.0, getThrottle());

    //p = toVectorInFixedSystem(p[0],p[1],p[2],rudder, elevator);

    //dBodyAddRelForceAtRelPos(body,p[0], p[1], p[2], 0.0, 0.0, -length - 0.1);

    rotateBody(body);

    dBodyAddForce(body, 0, elevator, 0);
    dBodyAddRelForce(body, 0,0,getThrottle());


    // @NOTE: Bullets are really unstable.
    if (VERIFY(pos, body))
        wrapDynamics(body);
}


void AAM::embody(dWorldID world, dSpaceID space)
{
    me = dBodyCreate(world);
    embody(me);
    //geom = dCreateSphere( space, 2.64f);
    geom = dCreateBox( space, AAM::width, AAM::height, AAM::length);
    dGeomSetBody(geom, me);
}

void AAM::embody(dBodyID myBodySelf)
{
    dMass m;

    float myMass = AAM::mass;

    dBodySetPosition(myBodySelf, pos[0], pos[1], pos[2]);
    dMassSetBox(&m, 1,AAM::width, AAM::height, AAM::length);
    dMassAdjust(&m, myMass*1.0f);
    dBodySetMass(myBodySelf,&m);

    me = myBodySelf;

}

int AAM::getType()
{
    return CONTROLABLEACTION;
}

void AAM::release(Vec3f orientation)
{
    Vehicle::inert = false;

    angularPos = getAzimuthRadians(orientation);
}


void AAM::doControl()
{

    Controller c;

    c.registers = registers;

    c.registers.thrust = 100;

    doControlFlipping(destination, 1);

}

void AAM::doControlFlipping(Vec3f target, float thrust)
{

    Controller c;

    c.registers = registers;

    Vec3f Po = getPos();

    float height = Po[1];

    float declination = getDeclination(getForward());

    float sp2=-0,        sp3 = target[1];

    Vec3f T = (target - Po);

    sp2 = getDeclination(T);

    float e1 = _acos(  T.normalize().dot(getForward().normalize()) );
    float e2 = sp2 - declination;
    float e3 = sp3 - height;

    // Pitch control, though working, the level of precission required to hit the target is hard to attain.
    float Kp3 = 110.1;
    float Ki3 = 5.1;
    float Kd3 = 2452.5;

    float r3 =  rt3 + Kp3 * (e3 - et3)        + Ki3 * (e3 + et3)/2.0 + Kd3 * (e3 - 2 * et3 + ett3);


    // Yaw is controlled by the target vector changing the yaw angle, hence roll info is zero.
    setForward(T);
    release(T);

    dout << "T:Az:"
              << std::setw(10) << getAzimuth(getForward())
              << std::setw(10) << getAzimuth(T)
              << "(" << std::setw(12) << e1 << ")"
              << std::setw(10) << declination
              << std::setw(10) << sp2
              << " Destination:"
              << std::setw(10) << T.magnitude() << std::endl;

    if (abs(height - sp3)<5)
    {
        if (a++<10  && T.magnitude()<100) {
            r3=0;
            stop();
        }
    }

    r3 = clipmax(r3, 600);
    r3 = clipmin(r3, -600);

    rt3 = r3;

    float pitch = -r3;

    ett3 = et3;

    c.registers.thrust = thrust/(10.0);
    c.registers.pitch = pitch;
    c.registers.yaw = 0;
    c.registers.roll = 0;
    setThrottle(thrust);

    doControl(c);
}


void AAM::doControl(Controller controller)
{

    doControl(controller.registers);


}

void AAM::doControl(struct controlregister conts)
{

    elevator    = -conts.pitch * 0.01;
    rudder      = -conts.roll  * 0.01;

    // Throttle is fixed when you "teleoperate" the missile.
    setThrottle(100.0);

    registers = conts;

}

void AAM::setVisible(bool val)
{
    Gunshot::visible = val;
    setTtl(50);
}

EntityTypeId AAM::getTypeId()
{
    return EntityTypeId::TAAM;
}


//draw3DSModel("units/missile.3ds",1200.0+100,15.0,700.0+300.0,1,_textureBox);

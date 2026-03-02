#include <unordered_map>
#include <cmath>
#include "Artillery.h"
#include "../actions/Shell.h"
#include "../math/yamathutil.h"

#include "../sounds/sounds.h"

#include "../profiling.h"


extern std::unordered_map<std::string, GLuint> textures;

Artillery::Artillery(int faction)
{
    Artillery::zoom = 20.0f;
    setFaction(faction);
}

void Artillery::init()
{
    //Load the model
    _model = (Model*)T3DSModel::loadModel(filereader("structures/turretbase.3ds"),0.0f,-8.14f,0.0f,1,1,1,textures["sky"]);
    if (_model != NULL)
    {

    }

    Structure::height=27.97;
    Structure::length=11.68;
    Structure::width=11.68;

    Artillery::firingpos = Vec3f(0.0f,19.0f,0.0f);
    muzzleVelocity = 15.0f;

    setName("Artillery");

    setForward(Vec3f(0,0,1));
}

void Artillery::drawModel(float yRot, float xRot, float x, float y, float z)
{
    float f[3];
    f[0] = 0; f[1] = 0; f[2] = 0;

    //Draw the saved model
    if (_model != NULL)
    {
        glPushMatrix();
        glTranslatef(x, y, z);

        glScalef(1.0f,1.0f,1.0f);
        //glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
        _model->draw();
        //drawRectangularBox(Structure::width, Structure::height, Structure::length);
        glRotatef(270.0f, 0.0f, 1.0f, 0.0f);
        glRotatef(-Structure::azimuth,0.0f,1.0f,0.0f);
        glRotatef(-Structure::elevation,0.0f,0.0f,1.0f);
        draw3DSModel(filereader("structures/turrettop.3ds"),0.0f,27.97f-8.14f,0.0f,1,textures["sky"]);
        glPopMatrix();

    }
    else
    {
        CLog::Write(CLog::Debug,"Model is null.\n");
    }
}

Vec3f Artillery::getForward()
{
    //Vec3f forward = toVectorInFixedSystem(0, 0, 1,Structure::azimuth,-Structure::inclination);
    //return forward;

    return forward;
}

void Artillery::setForward(float x, float y, float z)
{
    Artillery::setForward(Vec3f(x,y,z));
}

void Artillery::setForward(Vec3f forw)
{
    Structure::elevation = getDeclination(forw);
    Structure::azimuth = getAzimuth(forw);

    Structure::setForward(forw);

}

Vec3f Artillery::getFiringPort()
{
    //return Vec3f(getPos()[0],20.1765f, getPos()[2]);
    return Vec3f(getPos()[0],getPos()[1]+firingpos[1],getPos()[2]);
}

/**
 * Calibration table: (landing_distance, elevation_used).
 * To calibrate: set fixed elevation, fire, measure where shell lands (horizontal dist).
 * Fill table with (distance, elevation) pairs. Interpolation gives elevation for any distance.
 * elevation: -90=zenith, 0=horizon, positive=down (game convention)
 * Tune these values from actual game firings for best accuracy.
 */
struct TrajectoryCalib { float distance; float elevation; float velocity; };
// Ascending-only calibration table: each entry is the low-angle shot that reaches that distance.
// Using only the ascending portion of each velocity curve ensures monotone interpolation.
// Gaps between velocity bands (2896→3193, 3905→4231, 4915→5889) interpolate smoothly.
static const TrajectoryCalib TRAJ_CALIB[] = {
    // v=15: ascending -2° to -13°, range 1918–2897m
    { 1918.4f,  -2.0f, 15.0f },
    { 2199.9f,  -3.0f, 15.0f },
    { 2402.3f,  -4.0f, 15.0f },
    { 2553.4f,  -5.0f, 15.0f },
    { 2658.2f,  -6.0f, 15.0f },
    { 2736.7f,  -7.0f, 15.0f },
    { 2833.6f,  -9.0f, 15.0f },
    { 2879.4f, -11.0f, 15.0f },
    { 2896.6f, -13.0f, 15.0f },
    // v=20: ascending -3° to -11°, range 3193–3905m
    { 3193.2f,  -3.0f, 20.0f },
    { 3444.9f,  -4.0f, 20.0f },
    { 3613.5f,  -5.0f, 20.0f },
    { 3798.9f,  -7.0f, 20.0f },
    { 3878.8f,  -9.0f, 20.0f },
    { 3905.2f, -11.0f, 20.0f },
    // v=25: ascending -3° to -10°, range 4231–4915m
    { 4231.5f,  -3.0f, 25.0f },
    { 4513.1f,  -4.0f, 25.0f },
    { 4787.4f,  -6.0f, 25.0f },
    { 4888.6f,  -8.0f, 25.0f },
    { 4915.1f, -10.0f, 25.0f },
    // v=30: ascending -7° to -9°, range 5889–5922m
    { 5889.2f,  -7.0f, 30.0f },
    { 5922.2f,  -9.0f, 30.0f },
};
static const int NUM_TRAJ_CALIB = sizeof(TRAJ_CALIB) / sizeof(TRAJ_CALIB[0]);

/** Interpolate elevation and muzzle velocity for target horizontal distance (and optional height correction). */
static void interpolateAimParams(float targetDist, float targetHeightOffset,
                                 float &outElevation, float &outVelocity)
{
    if (targetDist <= TRAJ_CALIB[0].distance) {
        outElevation = TRAJ_CALIB[0].elevation;
        outVelocity  = TRAJ_CALIB[0].velocity;
        return;
    }
    if (targetDist >= TRAJ_CALIB[NUM_TRAJ_CALIB-1].distance) {
        outElevation = TRAJ_CALIB[NUM_TRAJ_CALIB-1].elevation;
        outVelocity  = TRAJ_CALIB[NUM_TRAJ_CALIB-1].velocity;
        return;
    }

    for (int i = 1; i < NUM_TRAJ_CALIB; i++) {
        if (targetDist < TRAJ_CALIB[i].distance) {
            float distRange = TRAJ_CALIB[i].distance - TRAJ_CALIB[i-1].distance;
            float distFrac  = (targetDist - TRAJ_CALIB[i-1].distance) / distRange;

            float elevation = TRAJ_CALIB[i-1].elevation
                            + (TRAJ_CALIB[i].elevation - TRAJ_CALIB[i-1].elevation) * distFrac;
            // Optional height correction
            elevation += targetHeightOffset * 0.5f;

            float velocity = TRAJ_CALIB[i-1].velocity
                           + (TRAJ_CALIB[i].velocity - TRAJ_CALIB[i-1].velocity) * distFrac;

            outElevation = elevation;
            outVelocity  = velocity;
            return;
        }
    }
    // Should never reach here
    outElevation = 0.0f;
    outVelocity  = 15.0f;
}

Vehicle* Artillery::aimAndFireAtTarget(Vec3f target,dWorldID world, dSpaceID space)
{
    // Find the vector between them, and the parameters for the turret to hit the vehicle, regardless of its random position.
    Vec3f firingloc = getFiringPort();

    dout << "Target distance: " << (target - firingloc).magnitude() << " Target height offset: " << (target[1] - firingloc[1]) << std::endl;

    float outElevation, outVelocity;
    interpolateAimParams((target - firingloc).magnitude(), target[1] - firingloc[1],
                         outElevation, outVelocity);
    elevation = outElevation;
    muzzleVelocity = outVelocity;
    azimuth = getAzimuth((target)-(firingloc));

    struct controlregister c;
    c.pitch = 0.0;
    c.roll = 0.0;
    setControlRegisters(c);
    setForward(toVectorInFixedSystem(0,0,1,azimuth, -elevation));

    dout << this <<  ":Azimuth: " << azimuth << " Inclination: " << elevation << " Muzzle Velocity: " << muzzleVelocity << std::endl;

    Vehicle *action = fire(0,world,space);

    return action;
}

void Artillery::getViewPort(Vec3f &Up, Vec3f &position, Vec3f &fw)
{
    position = getPos();
    position[1] += 2.0f;
    fw = toVectorInFixedSystem(0, 0, 1,azimuth,-elevation);
    Up = toVectorInFixedSystem(0.0f, 1.0f, 0.0f,0,0);

    //dout << "Forward:" << forward << std::endl;

    Vec3f orig;


    fw = fw.normalize();
    orig = position;
    Up[0]=Up[2]=0;Up[1]=4;// poner en 4 si queres que este un toque arriba desde atras.
    position = position + (abs(zoom))*fw;

    //forward = -orig+position;

}



Vehicle* Artillery::fire(int weapon, dWorldID world, dSpaceID space)
{
    if (getTtl()>0)
        return NULL;

    Shell *action = new Shell();
    // Need axis conversion.
    action->init();


    Vec3f position = getFiringPort();
    forward = getForward();forward = toVectorInFixedSystem(0, 0, 1,azimuth,-elevation);
    Vec3f Up = toVectorInFixedSystem(0.0f, 1.0f, 0.0f,0,0);

    Vec3f orig;


    forward = forward.normalize();
    orig = position;
    position = position + 40*forward;
    forward = -orig+position;

    Vec3f Ft = forward * muzzleVelocity;

    Vec3f f1(0.0,0.0,1.0);
    Vec3f f2 = forward.cross(f1);
    f2 = f2.normalize();
    float alpha = _acos( forward.dot(f1)/(f1.magnitude()*forward.magnitude()));

    dMatrix3 Re;
    dRSetIdentity(Re);
    dRFromAxisAndAngle(Re,f2[0],f2[1],f2[2],-alpha);

    action->embody(world,space);
    action->setPos(position[0],position[1],position[2]);

    Vec3f d = action->getPos() - getPos();

    //dout << d << std::endl;

    dout << "Elevation:" << elevation << " Azimuth:" << azimuth << std::endl;


    dBodySetLinearVel(action->getBodyID(),Ft[0],Ft[1],Ft[2]);
    dBodySetRotation(action->getBodyID(),Re);

    artilleryshot(getPos());
    setTtl(200);

    // I can set power or something here.
    return (Vehicle*)action;
}

void Artillery::doControl()
{
    Controller c;

    c.registers = registers;

    Artillery::doControl(c);
}

/**
 * The values are modified from the rc
 * @brief Turret::doControl
 * @param controller
 */
void Artillery::doControl(Controller controller)
{
    zoom = 20.0f + controller.registers.precesion*100;

    elevation -= controller.registers.pitch * (20.0f/abs(zoom)) ;
    azimuth += controller.registers.roll * (20.0f/abs(zoom)) ;

    setForward(toVectorInFixedSystem(0,0,1,azimuth, -elevation));

}


int Artillery::getSubType()
{
    return ARTILLERY;
}

EntityTypeId Artillery::getTypeId()
{
    return EntityTypeId::TArtillery;
}

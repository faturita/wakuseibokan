#include <unordered_map>
#include <cmath>
#include "Artillery.h"
#include "../actions/Shell.h"
#include "../math/yamathutil.h"

#include "../sounds/sounds.h"

#include "../profiling.h"

// Muzzle velocity - only angle is varied for aiming (interpolation table)
static const float SHELL_MUZZLE_VELOCITY = 15.0f;

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
struct TrajectoryCalib { float distance; float elevation; };
static const TrajectoryCalib TRAJ_CALIB[] = {
    { 1918.4f, -2.0f },
    { 2199.9f, -3.0f },
    { 2402.3f, -4.0f },
    { 2553.4f, -5.0f },
    { 2658.2f, -6.0f },
    { 2736.7f, -7.0f },
    { 2736.7f, -8.0f },
    { 2833.6f, -9.0f },
    { 2833.6f, -10.0f },
    { 2879.4f, -11.0f },
    { 2879.4f, -12.0f },
    { 2896.6f, -13.0f },
    { 2896.6f, -14.0f },
    { 2894.7f, -15.0f },
    { 2894.7f, -16.0f },
    { 2880.4f, -17.0f },
    { 2880.4f, -18.0f },
    { 2880.4f, -19.0f },
    { 2842.8f, -20.0f }
};
static const int NUM_TRAJ_CALIB = sizeof(TRAJ_CALIB) / sizeof(TRAJ_CALIB[0]);

/** Interpolate elevation for target horizontal distance (and optional height correction). */
static float interpolateElevation(float targetDist, float targetHeightOffset)
{
    // Simple linear interpolation between nearest calibration points. Could be improved with better interpolation or a formula.
    if (targetDist <= TRAJ_CALIB[0].distance) return TRAJ_CALIB[0].elevation;
    if (targetDist >= TRAJ_CALIB[NUM_TRAJ_CALIB-1].distance) return TRAJ_CALIB[NUM_TRAJ_CALIB-1].elevation;

    for (int i = 1; i < NUM_TRAJ_CALIB; i++) {
        if (targetDist < TRAJ_CALIB[i].distance) {
            float distRange = TRAJ_CALIB[i].distance - TRAJ_CALIB[i-1].distance;
            float elevRange = TRAJ_CALIB[i].elevation - TRAJ_CALIB[i-1].elevation;
            float distFrac = (targetDist - TRAJ_CALIB[i-1].distance) / distRange;
            float elevation = TRAJ_CALIB[i-1].elevation + elevRange * distFrac;

            // Optional height correction: simple proportional adjustment based on target height offset
            elevation += targetHeightOffset * 0.5f; // Tune this factor as needed

            return elevation;
        }
    }
    return 0.0f; // Should never reach here
}

Vehicle* Artillery::aimAndFireAtTarget(Vec3f target,dWorldID world, dSpaceID space)
{
    // Find the vector between them, and the parameters for the turret to hit the vehicle, regardless of its random position.
    Vec3f firingloc = getFiringPort();

    dout << "Target distance: " << (target - firingloc).magnitude() << " Target height offset: " << (target[1] - firingloc[1]) << std::endl;

    elevation = interpolateElevation((target - firingloc).magnitude(), target[1] - firingloc[1]);
    azimuth = getAzimuth((target)-(firingloc));

    struct controlregister c;
    c.pitch = 0.0;
    c.roll = 0.0;
    setControlRegisters(c);
    setForward(toVectorInFixedSystem(0,0,1,azimuth, -elevation));

    dout << this <<  ":Azimuth: " << azimuth << " Inclination: " << elevation << std::endl;

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

    Vec3f Ft = forward * SHELL_MUZZLE_VELOCITY;

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

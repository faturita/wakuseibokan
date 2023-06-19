#include "Laserturret.h"

#include "../profiling.h"

#include "../actions/LaserBeam.h"
#include "../actions/LaserRay.h"

LaserTurret::LaserTurret(int faction) : Turret(faction)
{
    setName("Laser");
}


void LaserTurret::locateLaserRay(LaserRay *action)
{
    Vec3f position = getPos();
    position[1] += 19.0f; // Move upwards to the center of the real rotation.
    forward = getForward();

    Vec3f orig;
    forward = forward.normalize();
    orig = position;
    position = position + 20*forward;
    forward = -orig+position;


    Vec3f f1(0.0,0.0,1.0);
    Vec3f f2 = forward.cross(f1);
    f2 = f2.normalize();
    float alpha = _acos( forward.dot(f1)/(f1.magnitude()*forward.magnitude()));

    dMatrix3 Re;
    dRSetIdentity(Re);
    dRFromAxisAndAngle(Re,f2[0],f2[1],f2[2],-alpha);

    action->setPos(position[0],position[1],position[2]);
    dGeomSetPosition(action->getGeom(),position[0],position[1],position[2]);

    //dMatrix3 R;
    //dGeomSetPosition(action->getGeom(),2000.0f,20.5f,-4000.0f);   // 0 20 -4000
    //dRSetIdentity(R);
    //dRFromAxisAndAngle (R,0.0f,1.0f,0.0f,PI);

    action->setR(Re);
    dGeomSetRotation (action->getGeom(),Re);
}

void LaserTurret::setForward(Vec3f forw)
{
    Turret::setForward(forw);

    if (LaserTurret::ls) locateLaserRay(LaserTurret::ls);
}

Vehicle* LaserTurret::fire(int weapon, dWorldID world, dSpaceID space)
{
    LaserRay *action=NULL;

    if (!firing && getTtl()<0)
    {
        action = new LaserRay();
        action->init();
        action->embody(world,space);
        locateLaserRay(action);

        setTtl(LASER_OVERHEATING);
        firing = true;

        LaserTurret::ls = action;
    } else if (getTtl()<0){
        LaserTurret::ls->disable();
        LaserTurret::ls = NULL;
        setTtl(LASER_RECHARGING);
        firing = false;
    }


    return action;
}

void LaserTurret::tick()
{
    if (firing && LaserTurret::ls && getTtl()<=0)
    {
        firing = false;
        setTtl(LASER_RECHARGING);
        LaserTurret::ls->disable();
        LaserTurret::ls = NULL;
        CLog::Write(CLog::Debug,"Shutting off laser\n");
    }

    Vehicle::tick();

}
void LaserTurret::doControl()
{
    Controller c;

    c.registers = registers;

    Turret::doControl(c);
}

void LaserTurret::doControl(Controller cr)
{
    // Let's move the ODE RayClass with me.  When firing is disabled, it will be finally destroyed (by setting ttl).
    Turret::doControl(cr);
    if (LaserTurret::ls) locateLaserRay(LaserTurret::ls);
}

int LaserTurret::getSubType()
{
    return LASERTURRET;
}

EntityTypeId LaserTurret::getTypeId()
{
    return EntityTypeId::TLaserTurret;
}

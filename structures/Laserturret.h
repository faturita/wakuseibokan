#ifndef LASERTURRET_H
#define LASERTURRET_H

#include "Turret.h"


class LaserTurret : public Turret
{
protected:
    bool firing=false;

public:
    LaserTurret();
    Vehicle* fire(dWorldID world, dSpaceID space);
    void drawModel(float yRot, float xRot, float x, float y, float z);
    void doControl();
};

#endif // LASERTURRET_H

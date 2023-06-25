#ifndef LASERBEAM_H
#define LASERBEAM_H

#include "Gunshot.h"

/**
 * Laser beams are rods that are not affected by gravity.  They last only for a few seconds and are used to verify
 * colisions.  If there is a colision, the beam has reached the target.
 *
 * @brief The LaserBeam class
 */
class LaserBeam : public Gunshot
{
protected:
    float range;
public:
    LaserBeam();
    ~LaserBeam();
    void init();
    void drawModel(float yRot, float xRot, float x, float y, float z);
    void drawModel();
    void doDynamics(dBodyID body);

    void embody(dWorldID world, dSpaceID space);
    void embody(dBodyID myBodySelf);

    int getType();
    EntityTypeId virtual getTypeId();
};

#endif // LASERBEAM_H

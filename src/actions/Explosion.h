#ifndef EXPLOSION_H
#define EXPLOSION_H

#include "Debris.h"

class Explosion : public Debris
{
public:
    Explosion();
    void expand(float height, float length, float width, float dense, dWorldID world, dSpaceID space);
};

#endif // EXPLOSION_H

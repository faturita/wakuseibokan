#ifndef DEBRIS_H
#define DEBRIS_H

#include "ArtilleryAmmo.h"


class Debris : public ArtilleryAmmo
{
public:
    Debris();
    void init();
    void drawModel(float yRot, float xRot, float x, float y, float z);
};

#endif // DEBRIS_H

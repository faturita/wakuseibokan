#ifndef RUNWAY_H
#define RUNWAY_H

#include "Structure.h"
#include "../units/Manta.h"


class Runway : public Structure
{
public:
    Runway(int faction);
    void init();
    void drawModel(float yRot, float xRot, float x, float y, float z);

    int getType();

    void taxi(Manta *m);
};

#endif // RUNWAY_H

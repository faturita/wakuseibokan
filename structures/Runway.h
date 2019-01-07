#ifndef RUNWAY_H
#define RUNWAY_H

#include "Structure.h"


class Runway : public Structure
{
public:
    Runway();
    void init();
    void drawModel(float yRot, float xRot, float x, float y, float z);
};

#endif // RUNWAY_H

#ifndef HANGAR_H
#define HANGAR_H

#include "Structure.h"

class Hangar : public Structure
{
public:
    Hangar();
    void init();
    void drawModel(float yRot, float xRot, float x, float y, float z);
};

#endif // HANGAR_H

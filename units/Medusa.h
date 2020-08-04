#ifndef MEDUSA_H
#define MEDUSA_H

#include "SimplifiedDynamicManta.h"

class Medusa : public SimplifiedDynamicManta
{

public:
    Medusa(int newfaction);

    void  init();

    int virtual getSubType();

    void drawModel(float yRot, float xRot, float x, float y, float z);

};

#endif // MEDUSA_H

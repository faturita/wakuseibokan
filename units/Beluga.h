#ifndef BELUGA_H
#define BELUGA_H

#include "Balaenidae.h"

class Beluga : public Balaenidae
{
public:
    Beluga(int faction);

    void drawModel(float yRot, float xRot, float x, float y, float z);
    void init();
};

#endif // BELUGA_H

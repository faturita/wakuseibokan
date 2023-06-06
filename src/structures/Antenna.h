#ifndef ANTENNA_H
#define ANTENNA_H

#include "Structure.h"

class Antenna : public Structure
{
public:
    Antenna(int faction);
    void init();
    void drawModel(float yRot, float xRot, float x, float y, float z);

    int getSubType() override;
    EntityTypeId virtual getTypeId();

};

#endif // ANTENNA_H

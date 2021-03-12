#ifndef RADAR_H
#define RADAR_H

#include "Structure.h"

class Radar : public Structure
{
public:
    Radar(int faction);
    void init();
    void drawModel(float yRot, float xRot, float x, float y, float z);

    int getSubType() override;
};

#endif // RADAR_H

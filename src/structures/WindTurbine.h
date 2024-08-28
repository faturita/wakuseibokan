#ifndef WINDTURBINE_H
#define WINDTURBINE_H

#include "Structure.h"

class WindTurbine : public Structure
{
public:
    WindTurbine(int faction);
    void init();
    void drawModel(float yRot, float xRot, float x, float y, float z);

    int getSubType() override;
    EntityTypeId virtual getTypeId();
};

#endif // WINDTURBINE_H

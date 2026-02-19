#ifndef WINDTURBINE_H
#define WINDTURBINE_H

#include "Structure.h"

class WindTurbine : public Structure
{
private:
    float production=1;
    float bladeAngle=0;

    void drawBlades();
public:
    WindTurbine(int faction);
    void init();
    void drawModel(float yRot, float xRot, float x, float y, float z);

    int getSubType() override;
    EntityTypeId virtual getTypeId();

    void getViewPort(Vec3f &Up, Vec3f &position, Vec3f &fw);

    void tick();
};

#endif // WINDTURBINE_H

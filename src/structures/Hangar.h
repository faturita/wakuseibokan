#ifndef HANGAR_H
#define HANGAR_H

#include "Structure.h"

class Hangar : public Structure
{
public:
    Hangar(int faction);
    void init();
    void drawModel(float yRot, float xRot, float x, float y, float z);

    int getSubType() override;
    EntityTypeId virtual getTypeId();
};

#endif // HANGAR_H

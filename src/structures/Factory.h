#ifndef FACTORY_H
#define FACTORY_H

#include "Structure.h"

class Factory : public Structure
{
public:
    Factory(int faction);
    void init();
    void drawModel(float yRot, float xRot, float x, float y, float z);

    int getSubType() override;
    EntityTypeId virtual getTypeId();
};

#endif // FACTORY_H

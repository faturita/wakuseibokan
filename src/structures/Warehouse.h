#ifndef WAREHOUSE_H
#define WAREHOUSE_H


#include "Structure.h"

class Warehouse : public Structure
{
public:
    Warehouse(int faction);
    void init();
    void drawModel(float yRot, float xRot, float x, float y, float z);

    int virtual getSubType();
    EntityTypeId virtual getTypeId();
};

#endif // WAREHOUSE_H

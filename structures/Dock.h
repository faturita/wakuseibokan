#ifndef DOCK_H
#define DOCK_H

#include "Structure.h"

class Dock : public Structure
{
public:
    Dock(int faction);
    void init();
    void drawModel(float yRot, float xRot, float x, float y, float z);

    int getSubType() override;
};

#endif // DOCK_H

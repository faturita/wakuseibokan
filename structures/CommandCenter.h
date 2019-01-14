#ifndef COMMANDCENTER_H
#define COMMANDCENTER_H


#include "Structure.h"

class CommandCenter : public Structure
{
public:
    CommandCenter();
    void init();
    void drawModel(float yRot, float xRot, float x, float y, float z);
};

#endif // COMMANDCENTER_H

#ifndef DEBRIS_H
#define DEBRIS_H

#include "ArtilleryAmmo.h"


class Debris : public ArtilleryAmmo
{
protected:
    GLuint texture;

public:
    Debris();
    void init();
    void init(Vec3f dimensions);
    void drawModel(float yRot, float xRot, float x, float y, float z);
    EntityTypeId virtual getTypeId();
    void setTexture(const GLuint &value);
};

#endif // DEBRIS_H

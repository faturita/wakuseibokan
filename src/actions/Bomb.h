#ifndef BOMB_H
#define BOMB_H

#include "Gunshot.h"


class Bomb : public Gunshot
{
protected:
    float range;

    bool ond;                   // Used for runonce.

public:
    Bomb(int faction);
    ~Bomb();
    void init();
    void drawModel(float yRot, float xRot, float x, float y, float z);
    void doMaterial();
    int getType();
    EntityTypeId virtual getTypeId();

    void getViewPort(Vec3f &Up, Vec3f &position, Vec3f &viewforward);

};


#endif // BOMB_H

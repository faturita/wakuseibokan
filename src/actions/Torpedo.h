#ifndef TORPEDO_H
#define TORPEDO_H

#include "Gunshot.h"

class Torpedo : public Gunshot
{
protected:
    float range;

    bool ond;                   // Used for runonce.

public:
    Torpedo(int faction);
    ~Torpedo();
    void init();
    void drawModel(float yRot, float xRot, float x, float y, float z);
    void drawModel();
    void doMaterial();

    void doDynamics(dBodyID body);

    void embody(dWorldID world, dSpaceID space);
    void embody(dBodyID myBodySelf);

    int getType();
    EntityTypeId virtual getTypeId();

    void release(Vec3f orientation);
    void doControl();

    void doControl(Controller controller);

    void doControl(struct controlregister conts);

    void setVisible(bool val);
};


#endif // TORPEDO_H

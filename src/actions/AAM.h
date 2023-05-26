#ifndef AAM_H
#define AAM_H

#include "Gunshot.h"

#include "../container.h"

class AAM : public Gunshot
{
protected:
    float range;

    float elevator = 0;
    float rudder = 0;

    std::vector<std::vector<float> > errserie;
    float et1=0,et2=0, et3=0;
    float ett1=0, ett2=0, ett3=0;
    float r1=0, r2=0, r3=0;
    float rt1=0, rt2=0, rt3=0;
    float midpointpitch = -5;

    float angularPos=0;

    bool ond;                   // Used for runonce.

    int a=0;

    Smoke smoke;

public:
    AAM(int faction);
    ~AAM();
    void init();
    void clean();
    void drawModel(float yRot, float xRot, float x, float y, float z);
    void drawModel();
    void doMaterial();

    void rotateBody(dBodyID body);
    void doDynamics(dBodyID body);

    void embody(dWorldID world, dSpaceID space);
    void embody(dBodyID myBodySelf);

    int getType();
    EntityTypeId getTypeId() override;

    void release(Vec3f orientation);
    void doControlFlipping(Vec3f target, float thrust);
    void doControl();

    void doControl(Controller controller);

    void doControl(struct controlregister conts);

    void setVisible(bool val);
};

#endif // AAM_H

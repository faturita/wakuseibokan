#ifndef BALAENIDAE_H
#define BALAENIDAE_H

#include "../weapons/Weapon.h"
#include "Vehicle.h"
#include "Manta.h"

class Balaenidae : public Vehicle
{
protected:
    float rudder;
    int offshoring = 0;
    Vec3f ap;
    std::vector<size_t> weapons;


public:

    static const int OFFSHORING = 1;
    static const int SAILING = 0;

    ~Balaenidae();
    Balaenidae(int newfaction);
    void virtual drawModel(float yRot, float xRot, float x, float y, float z);
    void init();
    void clean();
    int getType();
    int virtual getSubType();
    void virtual embody(dWorldID world, dSpaceID space);
    void virtual embody(dBodyID myBodySelf);
    dSpaceID embody_in_space(dWorldID world, dSpaceID space);

    void doControl(Controller controller);
    void doControl(struct controlregister regs);
    void virtual doControl();

    void getViewPort(Vec3f &Up, Vec3f &position, Vec3f &forward);
    void doDynamics();


    void doDynamics(dBodyID body);

    void drawModel();

    void offshore();

    Vehicle* spawn(dWorldID  world,dSpaceID space,int type, int number);

    void launch(Manta* m);
    void taxi(Manta *m);

    Vehicle* fire(int weapon, dWorldID world, dSpaceID space);

    std::vector<size_t> getWeapons();
    void addWeapon(size_t w);

    virtual EntityTypeId getTypeId();
};

#endif // BALAENIDAE_H

#ifndef BALAENIDAE_H
#define BALAENIDAE_H

#include "../weapons/Weapon.h"
#include "Vehicle.h"
#include "Manta.h"

class Balaenidae : public Vehicle
{
protected:
    float rudder;
    std::vector<size_t> weapons;


public:

    ~Balaenidae();
    Balaenidae(int newfaction);
    void virtual drawModel(float yRot, float xRot, float x, float y, float z);
    void init();
    void clean();
    int getType();
    void damage(float damage) override;
    int virtual getSubType();
    void virtual embody(dWorldID world, dSpaceID space);
    void virtual embody(dBodyID myBodySelf);
    dSpaceID embody_in_space(dWorldID world, dSpaceID space);

    void doControl(Controller controller);
    void doControl(struct controlregister regs);
    void virtual doControl();
    void doControlDocking();
    void doControlDestination(bool notifyfinish=true);
    void doControlWaypoint();

    void getViewPort(Vec3f &Up, Vec3f &position, Vec3f &forward);
    void doDynamics();


    void doDynamics(dBodyID body);

    void offshore(Vec3f d);
    void readyForDock();

    Vehicle* spawn(dWorldID  world,dSpaceID space,int type, int number);

    void launch(Manta* m);
    void taxi(Manta *m);

    Vehicle* fire(int weapon, dWorldID world, dSpaceID space);

    std::vector<size_t> getWeapons();
    void addWeapon(size_t w);

    virtual EntityTypeId getTypeId();

    float virtual getEnergyConsumption();

    std::vector<Vec3f> getVertices();
};

#endif // BALAENIDAE_H

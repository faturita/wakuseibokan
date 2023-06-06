#ifndef COMMANDCENTER_H
#define COMMANDCENTER_H

#include <vector>
#include <queue>

#include "Structure.h"

enum ISLANDTYPES { DEFENSE_ISLAND, FACTORY_ISLAND, LOGISTICS_ISLAND };

class CommandCenter : public Structure
{
protected:

    // The command center is created from a plan.  The plan that creates the command center, sets which structures
    // need to be created (it created the queue).

    // The idea is that as long as the ticks are moving, each time a specific amount of time elapsed,
    // the command center will get something from the queue, will put it on the island, it will start a timer
    // on the structure.

    // So, each structure will now have a timer that starts and ticks with the simulation.
    // When the time elapses, the structure will be completed (and operational) and will be available to be used.

    // This is the only way structures are created in the game.

    // When the queue is empty the command center does nothing.
    std::queue<Structure*> productionqueue;

    bool isunderattack = false;
    unsigned long myTimer=0;

    int islandType;

public:
    static const int BUILDING_TIME = 400;


    CommandCenter(int faction, int islandType);
    void init();
    void drawModel(float yRot, float xRot, float x, float y, float z);
    void getViewPort(Vec3f &Up, Vec3f &position, Vec3f &forward);

    int getType();
    int getSubType() override;
    EntityTypeId virtual getTypeId();

    void restart();
    void setUnderAttack(bool isunderattack);
    bool isUnderAttack();
    void setTimer(unsigned long timer);
    unsigned long getTimer();

    int getIslandType() const;
};

#endif // COMMANDCENTER_H

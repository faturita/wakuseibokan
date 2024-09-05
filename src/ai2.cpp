#include <assert.h>

#ifdef __linux
#include <bsd/stdlib.h>
#elif __APPLE__
#endif

#include "weapons/CarrierArtillery.h"
#include "weapons/CarrierTurret.h"
#include "profiling.h"
#include "engine.h"
#include "ai2.h"

extern  Controller controller;

/* dynamics and collision objects */

extern container<Vehicle*> entities;
extern std::vector<BoxIsland*> islands;
extern std::vector<Message> messages;
extern std::vector<TrackRecord>   track;

extern dWorldID world;
extern dSpaceID space;


class DockingQAction : public QAction
{   
public:
    void apply(int faction)
    {
        Vehicle *b = findCarrier(faction);

        if (b && b->getPower()<1001.0 && b->getAutoStatus() != AutoStatus::DOCKING)
        {
            //BoxIsland *enemyis = findNearestIsland(b->getPos(), false, faction);

            BoxIsland *is = findNearestIsland(b->getPos());

            if (is)
            {
                std::vector<size_t> str = is->getStructures();

                for(size_t id=0;id<str.size();id++)
                {

                    dout << entities[str[id]]->getName() << ":" << entities[str[id]]->getSubType() << std::endl;
                    if (entities[str[id]]->getSubType() == VehicleSubTypes::DOCK)
                    {
                        Dock *d = (Dock*)entities[str[id]];

                        if (d->getFaction() == faction)
                        {
                            b->setAutoStatus(AutoStatus::DOCKING);
                            b->setDestination(d->getPos()-d->getForward().normalize()*400);
                            b->enableAuto();
                            dout << "DOCKING!" << std::endl;
                        }
                    } 
                }

            }
            assert( true || !"No island around. Crazy thing.");
        }
    }
};

class RefuelQAction : public QAction
{   
public:
    void apply(int faction)
    {

    }
};



class IslandIsFree : public Condition
{
public:
    IslandIsFree(int faction, State s, State sprime) : Condition(faction,s,sprime)
    {

    }

    State evaluate(const State current )
    {
        Vehicle *b = findCarrier(faction);

        if (b && b->getPower()<1001.0)
        {
            //BoxIsland *enemyis = findNearestIsland(b->getPos(), false, faction);

            BoxIsland *is = findNearestIsland(b->getPos());

            if (is)
            {
                std::vector<size_t> str = is->getStructures();

                if (str.size() == 0)
                    return sprime;

            }

        }
    }
};


class EnoughFuelForNextOperation : public Condition
{
public:
    EnoughFuelForNextOperation(int faction, State s, State sprime) : Condition(faction,s,sprime)
    {

    }
    
    State evaluate(const State current )
    {
        Vehicle *b = findCarrier(faction);

        if (b && b->getPower()>1001.0)
        {
            return sprime;
        }
    }
};



class DockedCondition : public Condition
{
public:
    DockedCondition(int faction, State s, State sprime) : Condition(faction,s,sprime)
    {

    }
    State evaluate(const State current )
    {
        Vehicle *b = findCarrier(faction);

        if (b && b->getStatus() == SailingStatus::DOCKED)
        {
            return sprime;
        }

        return s;
    }
};


Player::Player(int faction)
{
    Player::faction = faction;


    for(int i=0;i<25;i++) qactions[i] = new QAction();
    for(int i=0;i<25;i++) conditions[i] = new Condition(faction, State::DOCKING,State::DOCKED);

    conditions[0] = new DockedCondition(faction, State::DOCKING,State::DOCKED);
    conditions[1] = new IslandIsFree(faction,State::DOCKING,State::IDLE);
    conditions[2] = new EnoughFuelForNextOperation(faction,State::DOCKING, State::IDLE);


    qactions[(int)State::IDLE] = new QAction();
    qactions[(int)State::DOCKING] = new DockingQAction();
    qactions[(int)State::DOCKED] = new RefuelQAction();
}

State Player::getCurrentState()
{
    return state;
}


void Player::playFaction(unsigned long timer)
{
    // Check for enemies nearby and shift strategy if they are present.
    //state = interruption->apply(state,faction,timeevent,timer);

    dout << "Status:" << (int)state << std::endl;

    // Fire the action according to the state.
    //state = qactions[state]->apply(state,faction,timeevent,timer);

    qactions[(int)state]->apply(faction);

    for(int i=0;i<25;i++)
        state = conditions[i]->evaluate(state);

}

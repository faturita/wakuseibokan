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
    bool evaluate(int faction, const State current )
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
                    return true;

            }

        }
        return false;
    }
};


class EnoughFuelForNextOperation : public Condition
{
public:
    
    bool evaluate(int faction, const State current )
    {
        Vehicle *b = findCarrier(faction);

        if (b && b->getPower()>1001.0)
        {
            return true;
        }

        return false;
    }
};

class NotEnoughFuelForNextOperation : public Condition
{
public:
    bool evaluate(int faction, const State current )
    {
        Vehicle *b = findCarrier(faction);

        if (b && b->getPower()<1001.0)
        {
            return true;
        }

        return false;
    }
};

// The condition should return true or false and this must be implemented in classes because it is code
// The evaluator and the transition from one state to the other can be just one class because it is always the same based on input.

class DockedCondition : public Condition
{
public:
    bool evaluate(int faction, const State current )
    {
        Vehicle *b = findCarrier(faction);

        if (b && b->getStatus() == SailingStatus::DOCKED)
        {
            return true;
        }

        return false;
    }
};


Player::Player(int faction)
{
    Player::faction = faction;


    for(int i=0;i<25;i++) qactions[i] = new QAction();
    for(int i=0;i<25;i++) transitions[i] = new Transition(State::IDLE,State::IDLE,Condition());

    transitions[0] = new Transition(State::IDLE,State::DOCKING,NotEnoughFuelForNextOperation());
    transitions[1] = new Transition(State::DOCKING,State::DOCKED,DockedCondition());
    transitions[2] = new Transition(State::IDLE,State::IDLE,EnoughFuelForNextOperation());



    qactions[(int)State::IDLE] = new QAction();
    qactions[(int)State::DOCKING] = new DockingQAction();
    qactions[(int)State::DOCKED] = new RefuelQAction();
    
    state = State::IDLE;
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
        state = transitions[i]->transit(faction,state);

}

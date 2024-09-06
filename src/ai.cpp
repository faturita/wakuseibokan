#include <assert.h>

#ifdef __linux
#include <bsd/stdlib.h>
#elif __APPLE__
#endif

#include "weapons/CarrierArtillery.h"
#include "weapons/CarrierTurret.h"
#include "profiling.h"
#include "engine.h"
#include "ai.h"

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
        Vehicle *b = findCarrier(faction);

        if (b && b->getStatus() == SailingStatus::DOCKED)
        {
            BoxIsland *is = findNearestIsland(b->getPos());

            if (is)
            {
                std::vector<size_t> str = is->getStructures();

                for(size_t id=0;id<str.size();id++)
                {
                    if (entities[str[id]]->getSubType() == VehicleSubTypes::DOCK)
                    {
                        Dock *d = (Dock*)entities[str[id]];

                        if (d->getFaction() == faction)
                        {
                            dout << "Refueling!" << std::endl;
                            refuel(d);
                            departure(d);
                            
                        }
                    } 
                }
            }
        }
    }
};

class ApproachFreeIslandQAction : public QAction
{   
    void apply(int faction)
    {
        Vehicle *b = findCarrier(faction);

        if (b)
        {
            BoxIsland *is = findNearestEmptyIsland(b->getPos());

            if (is)
            {
                Vec3f vector = (b->getPos()) - (is->getPos());

                // @FIXME: Solve the range
                if (vector.magnitude()>4000.0)
                {

                    vector = vector.normalize();

                    Vec3f finaldestiny = getRandomCircularSpot(is->getPos()+Vec3f(3500.0f * vector),150.0);

                    b->goTo(finaldestiny);
                    b->enableAuto();
                }
            } 
        }

    }
};


class ApproachEnemyIslandQAction : public QAction
{   
    void apply(int faction)
    {
        Vehicle *b = findCarrier(faction);

        if (b)
        {
            BoxIsland *is = findNearestEnemyIsland(b->getPos(),false, faction,100 kmf);

            if (!is)
            {
                // Closest enemy island is not found, lets check for an empty island.
                // @FIXME: Reset internal statues.
                return;
            }

            Vec3f vector = (b->getPos()) - (is->getPos());

            vector = vector.normalize();

            Vec3f approachaddress = getRandomCircularSpot(is->getPos()+Vec3f(12500.0f * vector),400.0);

            b->goTo(approachaddress);
            b->enableAuto();
        }
    }
};

class InvadeIslandQAction : public QAction
{
    void apply(int faction)
    {
        Vehicle *b = findCarrier(faction);

        if (b)
        {
            BoxIsland *is = findNearestIsland(b->getPos());

            // Check if the island is still free
            if (is->getCommandCenter())
            {
                return;
            }

            if ((b->getPos()-is->getPos()).magnitude()<5000.0 && b->getAutoStatus() != AutoStatus::DESTINATION)
            {

                // @FIXME: Find the closest walrus to the carrier or to the island.
                Walrus *w = findNearestWalrus(faction,is->getPos(),10000.0);

                if (!w)
                {
                    w = spawnWalrus(space,world,b);
                    w->goTo(getRandomCircularSpot(is->getPos(),200.0));
                    w->enableAuto();
                }

                dout << "Walrus status:" << (int)w->getAutoStatus() << std::endl;

                if (w->getAutoStatus() == AutoStatus::IDLE)
                {
                    if ((is->getPos()-w->getPos()).magnitude()>300.0)
                    {   
                        w->goTo(getRandomCircularSpot(is->getPos(),200.0));
                        w->enableAuto();
                    }
                    else
                    {
                        if ( w->getIsland() == is)
                        {
                            // Capture island
                            assert( ( is != NULL && w->getIsland() != NULL ) || !"The island and the Walrus' island are both null. This should not happen.");
                            int which = (rand() % 3);
                            captureIsland(is,w->getFaction(),which,space, world);

                            w->goTo(getRandomCircularSpot(b->getPos(),200.0));
                            w->enableAuto();

                        }
                    }
                }
            }
        }
    }
};



// ===================================== Conditions =====================================
class NoNearbyFreeIsland : public Condition
{
public:
    bool evaluate(int faction) override
    {
        Vehicle *b = findCarrier(faction);

        if (b)
        {
            BoxIsland *is = findNearestEmptyIsland(b->getPos());

            if (!is)
            {
                return true;
            }
        }

        return false;
    }
};

class FreeIslandIsClose : public Condition
{
public:
    bool evaluate(int faction) override
    {
        Vehicle *b = findCarrier(faction);

        if (b)
        {
            BoxIsland *is = findNearestEmptyIsland(b->getPos());

            if (is && (is->getPos()-b->getPos()).magnitude()<10000.0)
            {
                return true;
            }
        }

        return false;
    }
};

class IslandIsFree : public Condition
{
public:
    bool evaluate(int faction) override
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
    
    bool evaluate(int faction) override
    {
        Vehicle *b = findCarrier(faction);

        if (b && b->getPower()>998.0)
        {
            return true;
        }

        return false;
    }
};

class NotEnoughFuelForNextOperation : public EnoughFuelForNextOperation
{
public:
    bool evaluate(int faction) override
    {
        return !EnoughFuelForNextOperation::evaluate(faction);
    }
};

// The condition should return true or false and this must be implemented in classes because it is code
// The evaluator and the transition from one state to the other can be just one class because it is always the same based on input.

class DockedCondition : public Condition
{
public:
    bool evaluate(int faction) override
    {
        Vehicle *b = findCarrier(faction);

        if (b && b->getStatus() == SailingStatus::DOCKED)
        {
            return true;
        }

        return false;
    }
};

class CarrierIsSailing : public Condition
{
public:
    bool evaluate(int faction) override
    {
        Vehicle *b = findCarrier(faction);

        if (b && b->getStatus() == SailingStatus::SAILING)
        {
            return true;
        }

        return false;
    }
};

class CarrierHasArrived : public Condition
{
public:
    bool evaluate(int faction) override
    {
        Vehicle *b = findCarrier(faction);

        //dout << "CarrierHasArrived  " << (int)b->getAutoStatus() << std::endl;

        if (b && b->arrived())
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
    for(int i=0;i<25;i++) transitions[i] = new Transition(State::IDLE,State::IDLE,new Condition());

    transitions[0] = new Transition(State::IDLE,State::DOCKING,new NotEnoughFuelForNextOperation());
    transitions[1] = new Transition(State::DOCKING,State::DOCKED,new DockedCondition());
    transitions[2] = new Transition(State::IDLE,State::IDLE,new EnoughFuelForNextOperation());
    transitions[3] = new Transition(State::DOCKED,State::IDLE,new CarrierIsSailing());
    transitions[4] = new Transition(State::IDLE,State::INVADEISLAND,new FreeIslandIsClose());
    transitions[5] = new Transition(State::IDLE,State::APPROACHENEMYISLAND,new NoNearbyFreeIsland());
    transitions[6] = new Transition(State::IDLE,State::APPROACHFREEISLAND,new IslandIsFree());
    transitions[7] = new Transition(State::APPROACHFREEISLAND,State::INVADEISLAND,new CarrierHasArrived());
    transitions[8] = new Transition(State::APPROACHENEMYISLAND,State::INVADEISLAND,new CarrierHasArrived());


    qactions[(int)State::IDLE] = new QAction();
    qactions[(int)State::DOCKING] = new DockingQAction();
    qactions[(int)State::DOCKED] = new RefuelQAction();
    qactions[(int)State::APPROACHFREEISLAND] = new ApproachFreeIslandQAction();
    qactions[(int)State::APPROACHENEMYISLAND] = new ApproachEnemyIslandQAction();
    qactions[(int)State::INVADEISLAND] = new InvadeIslandQAction();


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
    qactions[(int)state]->apply(faction);


    for(int i=0;i<25;i++)
        state = transitions[i]->transit(faction,state);

}

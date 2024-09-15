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

        if (b && b->getPower()<500.0 && b->getAutoStatus() != AutoStatus::DOCKING)
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
                            b->ready();
                            b->setDestination(d->getPos()-d->getForward().normalize()*400);
                            b->setAutoStatus(AutoStatus::DOCKING);
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
                            // Check if refueling is enough...
                            if ( (getIslandCargo(is,CargoTypes::POWERFUEL)+b->getPower()) > 500)
                            {
                                dout << "Refueling!" << std::endl;
                                collect(d);
                                refuel(d);
                                departure(d);
                            } 
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
                if (b->getAutoStatus() == AutoStatus::IDLE && vector.magnitude()>4000.0)
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

class ResetQAction : public QAction
{   
    void apply(int faction)
    {
        Vehicle *b = findCarrier(faction);

        if (b)
        {
            b->stop();
            b->ready();
            b->setAutoStatus(AutoStatus::IDLE);
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

            if (is)
            {
                // Closest enemy island is not found, lets check for an empty island.
                // @FIXME: Reset internal statues.

                Vec3f vector = (b->getPos()) - (is->getPos());

                if (b->getAutoStatus() == AutoStatus::IDLE && vector.magnitude()>4000.0)
                {
                    vector = vector.normalize();

                    Vec3f approachaddress = getRandomCircularSpot(is->getPos()+Vec3f(12500.0f * vector),400.0);

                    b->goTo(approachaddress);
                    b->enableAuto();
                }
            }
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
            if (!is->getCommandCenter())
            {
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
                    }

                    if ( w->getIsland() == is)
                    {
                        // Capture island
                        assert( ( is != NULL && w->getIsland() != NULL ) || !"The island and the Walrus' island are both null. This should not happen.");
                        int which = (rand() % 3);

                        static bool firstisland=true;

                        if (firstisland)
                        {
                            which = ISLANDTYPES::CAPITAL_ISLAND;  // The first island should be the capital island, all the types together.
                            firstisland = false;
                        }

                        captureIsland(is,w->getFaction(),which,space, world);

                        w->goTo(getRandomCircularSpot(b->getPos(),200.0));
                        w->enableAuto();

                    }
                }
            }
            else
            {
                // Check where is the walrus and dock it back.
                Walrus *w = findNearestWalrus(faction,b->getPos(),1000.0);

                if (w && (w->getPos()-b->getPos()).magnitude()<200.0)
                {
                    synchronized(entities.m_mutex)
                    {
                        dockWalrus(b);
                    }
                }

            }
        }
    }
};

class RendezvousQAction : public QAction
{
    void apply(int faction)
    {
        Vehicle *b = findCarrier(faction);

        if (!b)
            return;

        bool done1 = true;
        bool done2 = true;

        // Find walruses around the carrier and dock them (do not instruct the walruses to come back to the carrier)
        for(size_t i=entities.first();entities.hasMore(i);i=entities.next(i))
        {
            Vehicle *v=entities[i];
            if (v->getType() == WALRUS && v->getFaction() == faction)
            {
                if ((v->getPos()-b->getPos()).magnitude()<10000)   {

                    done1 = false;
                    Walrus *w = (Walrus*)v;
                    if (w)
                    {
                        synchronized(entities.m_mutex)
                        {
                            dockWalrus(b);
                        }
                    }
                }

            }
        }

        // Find all the mantas around, land them, and dock them.
        Manta *m1 = findManta(faction,FlyingStatus::ON_DECK);
        Manta *m2 = findNearestManta(FlyingStatus::FLYING,b->getFaction(),b->getPos(),30000);
        Manta *m3 = findNearestManta(FlyingStatus::HOLDING,b->getFaction(), b->getPos(),30000);

        Manta *m = findMantaByOrder(faction, CONQUEST_ISLAND);

        if (m)
        {
            landManta(b,m);
            b->stop();
        }


        if (m2)
        {
            // Updating the current carrier postion to Manta so that it improves landing.
            //m2->setDestination(b->getPos());
            m2->setAttitude(b->getForward());
        } else
        {
            if (m3)
            {
                if (m3->getAutoStatus() != AutoStatus::LANDING)
                    landManta(b,m3);
                b->stop();
            }
        }


        if (m1)
        {
            synchronized(entities.m_mutex)
            {
                dockManta();
            }
        }
    }
};

// ===================================== Conditions =====================================
class AllUnitsDocked : public Condition
{
public:
    bool evaluate(int faction) override
    {
        Vehicle *b = findCarrier(faction);

        if (!b)
            return false;

        bool done1 = true;
        bool done2 = true;

        // Find walruses around and dock them.
        for(size_t i=entities.first();entities.hasMore(i);i=entities.next(i))
        {
            Vehicle *v=entities[i];
            if (v->getType() == WALRUS && v->getFaction() == faction && v->getSubType()!= VehicleSubTypes::CARGOSHIP)
            {
                if ((v->getPos()-b->getPos()).magnitude()<10000)   {

                    done1 = false;
                }

            }
        }

        // Find all the mantas around, land them, and dock them.
        Manta *m1 = findManta(faction,FlyingStatus::ON_DECK);
        Manta *m2 = findNearestManta(FlyingStatus::FLYING,b->getFaction(),b->getPos(),30000);
        Manta *m3 = findNearestManta(FlyingStatus::HOLDING,b->getFaction(), b->getPos(),30000);

        Manta *m = findMantaByOrder(faction, CONQUEST_ISLAND);

        if (m)
        {
            done2=false;
        }
        if (m2)
        {
            done2 = false;
        } else
        {
            if (m3)
            {
                done2 = false;
            }
        }

        return (done1 && done2);

    }
};


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

class ClosestIslandIsFriendly : public Condition
{
public:
    bool evaluate(int faction) override
    {
        Vehicle *b = findCarrier(faction);

        if (b)
        {
            BoxIsland *is = findNearestFriendlyIsland(b->getPos(),false,faction,10000.0);

            if (is && (is->getPos()-b->getPos()).magnitude()<10000.0)
            {
                return true;
            }
        }

        return false;
    }
};

class ClosestIslandIsFree : public Condition
{
public:
    bool evaluate(int faction) override
    {
        Vehicle *b = findCarrier(faction);

        if (b)
        {
            BoxIsland *is =findNearestIsland(b->getPos());

            if (is && (is->getPos()-b->getPos()).magnitude()<10000.0)
            {
                Structure *sc = is->getCommandCenter();
                if (!sc)
                {
                    return true;
                }
            }
        }

        return false;
    }
};

class ClosestIslandIsEnemy : public Condition
{
public:
    bool evaluate(int faction) override
    {
        Vehicle *b = findCarrier(faction);

        if (b)
        {
            BoxIsland *is =findNearestIsland(b->getPos());

            if (is && (is->getPos()-b->getPos()).magnitude()<10000.0)
            {
                Structure *sc = is->getCommandCenter();
                if (sc && sc->getFaction() != faction)
                {
                    return true;
                }
            }
        }

        return false;
    }
};

class ClosestFarAwayIslandIsFree : public Condition
{
public:
    bool evaluate(int faction) override
    {
        Vehicle *b = findCarrier(faction);

        dout << "ClosestFarAwayIslandIsFree" << std::endl;  

        // Do I have power to get to the next island
        if (b && b->getPower()>500.0)
        {
            //BoxIsland *enemyis = findNearestIsland(b->getPos(), false, faction);

            BoxIsland *freeis   =    findNearestEmptyIsland(b->getPos());
            BoxIsland *enemyis  =    findNearestEnemyIsland(b->getPos(),false, faction);


            dout << freeis << std::endl;
            dout << enemyis << std::endl;
            dout << "Free island:" << (freeis->getPos()-b->getPos()).magnitude() << std::endl;

            if (freeis && !enemyis)
                return true;

            if (enemyis && !freeis)
                return false;

            if (freeis && enemyis)
            {
                if ((freeis->getPos()-b->getPos()).magnitude() > 40000.0 && (freeis->getPos()-b->getPos()).magnitude() < (enemyis->getPos()-b->getPos()).magnitude())
                {
                    return true;
                }
            }

        }
        return false;
    }
};

class ClosestFarAwayIslandIsEnemy : public Condition
{
public:
    bool evaluate(int faction) override
    {
        Vehicle *b = findCarrier(faction);

        // Do I have power to get to the next island
        if (b && b->getPower()>500.0)
        {
            //BoxIsland *enemyis = findNearestIsland(b->getPos(), false, faction);

            BoxIsland *freeis   =    findNearestEmptyIsland(b->getPos());
            BoxIsland *enemyis  =    findNearestEnemyIsland(b->getPos(),false, faction);

            if (!freeis && enemyis)
                return true;

            if (!enemyis && freeis)
                return false;

            if (freeis && enemyis)
            {
                if ((enemyis->getPos()-b->getPos()).magnitude() > 40000.0 && (enemyis->getPos()-b->getPos()).magnitude() < (freeis->getPos()-b->getPos()).magnitude())
                {
                    return true;
                }
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

        if (b && b->getPower()>500.0)
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

class CarrierHasArrivedToFreeIsland : public ClosestIslandIsFree
{
public:
    bool evaluate(int faction) override
    {
        Vehicle *b = findCarrier(faction);

        //dout << "CarrierHasArrived  " << (int)b->getAutoStatus() << std::endl;

        if (b && b->arrived())
        {
            return ClosestIslandIsFree::evaluate(faction);
        }

        return false;
    }
};

class CarrierHasArrivedToEnemyIsland : public ClosestIslandIsEnemy
{
public:
    bool evaluate(int faction) override
    {
        Vehicle *b = findCarrier(faction);

        //dout << "CarrierHasArrived  " << (int)b->getAutoStatus() << std::endl;

        if (b && b->arrived())
        {
            return ClosestIslandIsEnemy::evaluate(faction);
        }

        return false;
    }
};


Player::Player(int faction)
{
    Player::faction = faction;


    for(int i=0;i<25;i++) qactions[i] = new QAction();
    for(int i=0;i<25;i++) transitions[i] = new Transition(State::IDLE,State::IDLE,new Condition());

    transitions[0] = new Transition(State::IDLE,State::DOCKING,new NotEnoughFuelForNextOperation());            // Power < 500
    transitions[1] = new Transition(State::DOCKING,State::DOCKED,new DockedCondition());                        // SailingStatus::DOCKED
    transitions[2] = new Transition(State::IDLE,State::IDLE,new EnoughFuelForNextOperation());                  // Power > 500
    transitions[3] = new Transition(State::DOCKED,State::IDLE,new CarrierIsSailing());                          // SailingStatus::SAILING 
    transitions[4] = new Transition(State::IDLE,State::INVADEISLAND,new ClosestIslandIsFree());                 // <10000.0, no command center
    transitions[5] = new Transition(State::IDLE,State::APPROACHENEMYISLAND,new ClosestFarAwayIslandIsEnemy());  // >40000.0, closer than any other empty island
    transitions[6] = new Transition(State::IDLE,State::APPROACHFREEISLAND,new ClosestFarAwayIslandIsFree());    // >40000.0, closer than any other enemy island
    transitions[7] = new Transition(State::APPROACHFREEISLAND,State::INVADEISLAND,new CarrierHasArrivedToFreeIsland());     // arrived(), dst_status == REACHED
    transitions[8] = new Transition(State::APPROACHENEMYISLAND,State::INVADEISLAND,new CarrierHasArrivedToEnemyIsland());    // arrived(), dst_status == REACHED
    transitions[9] = new Transition(State::INVADEISLAND,State::RENDEZVOUS,new ClosestIslandIsFriendly());       // <10000.0, command center
    transitions[10] = new Transition(State::RENDEZVOUS,State::IDLE,new AllUnitsDocked());                       // No units around


    qactions[(int)State::IDLE] = new ResetQAction();
    qactions[(int)State::DOCKING] = new DockingQAction();
    qactions[(int)State::DOCKED] = new RefuelQAction();
    qactions[(int)State::APPROACHFREEISLAND] = new ApproachFreeIslandQAction();
    qactions[(int)State::APPROACHENEMYISLAND] = new ApproachEnemyIslandQAction();
    qactions[(int)State::INVADEISLAND] = new InvadeIslandQAction();
    qactions[(int)State::RENDEZVOUS] = new RendezvousQAction();


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

    if (timer % 1000 == 0)
        dout << "Status:" << (int)state << std::endl;

    // Fire the action according to the state.
    qactions[(int)state]->apply(faction);

    //dout << "Statuses -------" << std::endl;
    for(int i=0;i<25;i++)
    {
        //dout << "Status:" << (int)state << std::endl;
        state = transitions[i]->transit(faction,state);
    }

}

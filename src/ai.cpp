#include <assert.h>

#ifdef __linux
#include <bsd/stdlib.h>
#elif __APPLE__
#endif

#include <iostream>
#include <map>
#include <vector>

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

extern unsigned long timer;

extern dWorldID world;
extern dSpaceID space;

// Create a global dictionary to store the required fuel per faction and also the next island to approach.
// @FIXME: It will make more sense to have this as a member of the Player class, and all the other classes nested in the Player class.
struct NextOperation
{
    BoxIsland *nextIsland;
    float requiredFuel;
};
std::map<int, NextOperation> nextOperationPerFaction;



float RANGE[] = {40000.0, 20000.0, 6000.0};

std::string getStateMachineName(State state)
{
    switch (state)
    {
        case State::IDLE: return "IDLE";
        case State::DOCKING: return "DOCKING";
        case State::DOCKED: return "DOCKED";
        case State::APPROACHFREEISLAND: return "APPROACHFREEISLAND";
        case State::APPROACHENEMYISLAND: return "APPROACHENEMYISLAND";
        case State::APPROACHFRIENDLYISLAND: return "APPROACHFRIENDLYISLAND";
        case State::INVADEISLAND: return "INVADEISLAND";
        case State::RENDEZVOUS: return "RENDEZVOUS";
        case State::BALLISTICATTACK: return "BALLISTICATTACK";
        case State::AIRBORNEATTACK: return "AIRBORNEATTACK";
        case State::AIRDEFENSE: return "AIRDEFENSE";
        default: return "UNKNOWN";
    }
}

class AirDefenseQAction : public QAction
{
    void apply(int faction)
    {
        Vehicle *b = findCarrier(faction);

        if (b)
        {
            std::vector<size_t> enemies = findNearestEnemyVehicles(faction,-1, b->getPos(),DEFENSE_RANGE);

            Vehicle *v = NULL;
            if (enemies.size()>0)
            {
                v = entities[enemies[0]];
            }

            if (v)
            {
                Vec3f Po = b->getPos();
                Vec3f Pf = v->getPos();
                Vec3f T = Pf-Po;

                for (size_t i: enemies)
                {
                    if (entities[i]->getType() == CARRIER)
                    {
                        b->disableAuto();
                        b->setThrottle(0.0);
                        b->setDestination(b->getPos()-T.normalize()*1000);
                        b->enableAuto();
                    }
                }
            }
        }
    }
};


class DockingQAction : public QAction
{   
public:
    void apply(int faction)
    {
        Vehicle *b = findCarrier(faction);

        if (b && b->getPower()<nextOperationPerFaction[faction].requiredFuel && b->getAutoStatus() != AutoStatus::DOCKING)
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
                            if ( (getIslandCargo(is,CargoTypes::POWERFUEL)+b->getPower()) > nextOperationPerFaction[faction].requiredFuel)
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

class ApproachFreeIslandQAction : public QAction
{   
    void apply(int faction)
    {
        Vehicle *b = findCarrier(faction);

        if (b)
        {
            BoxIsland *is = nextOperationPerFaction[faction].nextIsland;

            if (is)
            {
                Vec3f vector = (b->getPos()) - (is->getPos());

                // @FIXME: Solve the range
                if (b->getAutoStatus() == AutoStatus::IDLE && vector.magnitude()>RANGE[2])
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

class ApproachFriendlyIslandQAction : public QAction
{   
    void apply(int faction)
    {
        Vehicle *b = findCarrier(faction);

        if (b)
        {
            BoxIsland *is = nextOperationPerFaction[faction].nextIsland;

            if (is)
            {
                Vec3f vector = (b->getPos()) - (is->getPos());

                // @FIXME: Solve the range
                if (b->getAutoStatus() == AutoStatus::IDLE && vector.magnitude()>RANGE[2])
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
            BoxIsland *is = nextOperationPerFaction[faction].nextIsland;

            if (is)
            {

                Vec3f vector = (b->getPos()) - (is->getPos());

                if (b->getAutoStatus() == AutoStatus::IDLE && vector.magnitude()>RANGE[1])
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
                if ((b->getPos()-is->getPos()).magnitude()<10000.0 && b->getAutoStatus() != AutoStatus::DESTINATION)
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
                        int typeofisland = (rand() % 3);

                        int ownislands = countNumberOfIslands(controller.faction);
                        
                        if (ownislands==0)
                            typeofisland = ISLANDTYPES::CAPITAL_ISLAND;


                        captureIsland(is,w->getFaction(),typeofisland,space, world);

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

class BallisticAttackQAction : public QAction
{   
    TSequencer T;

    void start()
    {
        T[0] = 1;
    }

    void tick()
    {
        T = T + T.sign() * 1;
    }

    void apply(int faction)
    {
        tick();

        Vehicle *b = findCarrier(faction);

        if (b)
        {
            BoxIsland *is = findNearestIsland(b->getPos());

            dout << "T-:" << (int)((T[0])) << std::endl;

            // @FIXME If it has available missiles, then fire them.

            if (T[0] == 100)
            {
                Missile *a = (Missile*) b->fire(0,world, space);

                size_t i = CONTROLLING_NONE;
                if (a)
                {
                    i = entities.push_back(a,a->getGeom());

                    CommandCenter *c = (CommandCenter*)is->getCommandCenter();

                    if (c)
                    {
                        a->goTo(c->getPos());
                        a->enableAuto();

                        if (a->getType()==CONTROLABLEACTION)
                        {
                            // @NOTE: The switch to see the missile only happens if the controlling faction can do it.
                            switchControl(i);

                        }
                    }
                }
            }

            // @NOTE: Should check how many missiles can the carrier shoot.
            // @FIXME: Parametrize the magic number
            if (T[0] >= 1500)
            {
                if (is)
                {
                    CommandCenter *c = (CommandCenter*)is->getCommandCenter();

                    if (c)
                    {
                        if (T[0] == 1600)
                        {
                            // Launch mantas to attack the island.

                            Manta *m = findMantaByOrder(faction, ATTACK_ISLAND);

                            if (!m)
                            {
                                size_t idx = 0;
                                Manta *m = spawnManta(space,world,b, idx);

                                m->setOrder(ATTACK_ISLAND);
                            }
                    
                        }
                    }
                }
            }
        }
    }
};

class AirborneAttackQAction : public QAction
{   
    TSequencer T;

    void start()
    {
        T[0] = 1;
        T[1] = 1;
    }

    void tick()
    {
        T = T + T.sign() * 1;
    }

    void apply(int faction)
    {
        tick();

        Vehicle *b = findCarrier(faction);

        if (!b)
            return ;


        Manta *m1 = findManta(faction,FlyingStatus::ON_DECK);
        //Manta *m2 = findNearestManta(FlyingStatus::FLYING,b->getFaction(),b->getPos(),30000);
        //Manta *m3 = findNearestManta(FlyingStatus::HOLDING,b->getFaction(), b->getPos(),30000);
        Manta *m4 = findMantaByOrder(faction, ATTACK_ISLAND);

        if (m1)
        {
            if (T[1]==200)
            {
                launchManta(b);
            }
        }

        if (m4)
        {
            BoxIsland *is = findNearestIsland(b->getPos());

            CommandCenter *c = (CommandCenter*)is->getCommandCenter();

            if (c && T[1]>400 && m4->getAutoStatus() != AutoStatus::ATTACK)
            {
                m4->attack(c->getPos());
                m4->enableAuto();
            }

            if (!c && T[1]>400 && m4->getAutoStatus() != AutoStatus::IDLE) // Get back to carrier....
            {
                m4->doHold(is->getPos(),100.0); // @FIXME Check the power.
                m4->enableAuto() ;                    
            } 
        } else {
            // Launch a new manta.
            Manta *m = findMantaByOrder(faction, ATTACK_ISLAND);

            if (!m)
            {
                size_t idx = 0;
                Manta *m = spawnManta(space,world,b, idx);

                m->setOrder(ATTACK_ISLAND);
            }
            T[1] = 1;  // Reset the counter
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
private:
    float range;
public:
    ClosestIslandIsFriendly()
    {
        this->range = 10000.0;
    }
    ClosestIslandIsFriendly(float range)
    {
        this->range = range;
    }
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
private:
    float range;
public:
    ClosestIslandIsFree()
    {
        this->range = 10000.0;
    }
    ClosestIslandIsFree(float range)
    {
        this->range = range;
    }
    bool evaluate(int faction) override
    {
        Vehicle *b = findCarrier(faction);

        if (b)
        {
            BoxIsland *is =findNearestIsland(b->getPos());

            //std::cout << "Distance:" << (is->getPos()-b->getPos()).magnitude() << std::endl;

            if (is && (is->getPos()-b->getPos()).magnitude()<range)
            {
                Structure *sc = is->getCommandCenter();

                //std::cout << sc << std::endl;

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
private:
    float range;
public:
    ClosestIslandIsEnemy()
    {
        this->range = 15000.0;
    }
    ClosestIslandIsEnemy(float range)
    {
        this->range = range;
    }
    bool evaluate(int faction) override
    {
        Vehicle *b = findCarrier(faction);

        if (b)
        {
            BoxIsland *is =findNearestIsland(b->getPos());

            //std::cout << "Distance:" << (is->getPos()-b->getPos()).magnitude() << std::endl;

            if (is && (is->getPos()-b->getPos()).magnitude()<range)
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
        if (b && b->getPower()>nextOperationPerFaction[faction].requiredFuel)
        {

            BoxIsland *is   =    nextOperationPerFaction[faction].nextIsland;

            if (is)
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

class ClosestFarAwayIslandIsEnemy : public Condition
{
public:
    bool evaluate(int faction) override
    {
        Vehicle *b = findCarrier(faction);

        dout << "ClosestFarAwayIslandIsEnemy" << std::endl;  

        // Do I have power to get to the next island
        if (b && b->getPower()>nextOperationPerFaction[faction].requiredFuel)
        {

            BoxIsland *is   =    nextOperationPerFaction[faction].nextIsland;

            if (is)
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


class ClosestFarAwayIslandIsFriendly : public Condition
{
public:
    bool evaluate(int faction) override
    {
        Vehicle *b = findCarrier(faction);

        dout << "ClosestFarAwayIslandIsFriendly" << std::endl;  

        // Do I have power to get to the next island
        if (b && b->getPower()>nextOperationPerFaction[faction].requiredFuel)
        {

            BoxIsland *is   =    nextOperationPerFaction[faction].nextIsland;

            if (is)
            {
                Structure *sc = is->getCommandCenter();
                if (sc && sc->getFaction() == faction)
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

        if (b && nextOperationPerFaction[faction].nextIsland)
        {
            // Get the next island to approach
            BoxIsland *is = nextOperationPerFaction[faction].nextIsland;

            Vec3f destination = is->getPos();

            float distance = (destination-b->getPos()).magnitude();
            float requiredfuel = distance * (  1000.0 / 254000.0 );

            std::cout << "Required fuel for next operation:" << requiredfuel << std::endl;

            assert ( requiredfuel <= 1000.0 || !"The required fuel is too high. This should not happen.");

            nextOperationPerFaction[faction].requiredFuel = (requiredfuel + 50.0);

            if (nextOperationPerFaction[faction].requiredFuel  > 1000.0)
                nextOperationPerFaction[faction].requiredFuel  = 1000.0;

            
            char msg[256];
            Message mg;
            mg.faction = b->getFaction();
            sprintf(msg, "Required fuel for next operation %5.2f.", nextOperationPerFaction[faction].requiredFuel );
            mg.msg = std::string(msg); mg.timer = timer;
            messages.insert(messages.begin(), mg);


            if (b->getPower()>nextOperationPerFaction[faction].requiredFuel )
            {
                return true;
            }

            return false;
        }

        return true;
    }
};

class NotEnoughFuelForNextOperation : public EnoughFuelForNextOperation
{
public:
    bool evaluate(int faction) override
    {
        Vehicle *b = findCarrier(faction);

        if (b && nextOperationPerFaction[faction].nextIsland)
            return !EnoughFuelForNextOperation::evaluate(faction);

        return false;
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
    CarrierHasArrivedToFreeIsland() : ClosestIslandIsFree()
    {
    }
    CarrierHasArrivedToFreeIsland(float range) : ClosestIslandIsFree(range)
    {
    }
    bool evaluate(int faction) override
    {
        Vehicle *b = findCarrier(faction);

        //dout << "CarrierHasArrived  " << (int)b->getAutoStatus() << std::endl;

        if (b && b->arrived())
        {
            nextOperationPerFaction[faction].requiredFuel  = 1000.0;
            return ClosestIslandIsFree::evaluate(faction);
        }

        return false;
    }
};

class CarrierHasArrivedToFriendlyIsland : public ClosestIslandIsFriendly
{
public:
    CarrierHasArrivedToFriendlyIsland() : ClosestIslandIsFriendly()
    {
    }
    CarrierHasArrivedToFriendlyIsland(float range) : ClosestIslandIsFriendly(range)
    {
    }
    bool evaluate(int faction) override
    {
        Vehicle *b = findCarrier(faction);

        //std::cout << "CarrierHasArrived  " << (int)b->getAutoStatus() << "."  << (int)b->dst_status << std::endl;

        if (b && b->arrived())
        {
            nextOperationPerFaction[faction].requiredFuel  = 1000.0;
            return ClosestIslandIsFriendly::evaluate(faction);
        }

        return false;
    }
};

class CarrierHasArrivedToEnemyIsland : public ClosestIslandIsEnemy
{
public:
    CarrierHasArrivedToEnemyIsland() : ClosestIslandIsEnemy()
    {
    }
    CarrierHasArrivedToEnemyIsland(float range) : ClosestIslandIsEnemy(range)
    {
    }
    bool evaluate(int faction) override
    {
        Vehicle *b = findCarrier(faction);

        //dout << "CarrierHasArrived  " << (int)b->getAutoStatus() << std::endl;

        if (b && b->arrived())
        {
            nextOperationPerFaction[faction].requiredFuel  = 1000.0;
            return ClosestIslandIsEnemy::evaluate(faction);
        }

        return false;
    }
};

class UnitsDeployedForAttack : public ClosestIslandIsEnemy
{
public:
    bool evaluate(int faction) override
    {
        Vehicle *b = findCarrier(faction);

        if (!b)
            return false;

        Manta *m1 = findManta(faction,FlyingStatus::ON_DECK);
        Manta *m2 = findNearestManta(FlyingStatus::FLYING,faction,b->getPos(),30000);
        Manta *m3 = findNearestManta(FlyingStatus::HOLDING,faction,b->getPos(),30000);
        Manta *m4 = findMantaByOrder(faction, ATTACK_ISLAND);

        if (m1 || m2 || m3 || m4)
        {
            return ClosestIslandIsEnemy::evaluate(faction);
        }

        return false;
    }
};

class DefCon : public Condition
{   
public:
    bool evaluate(int faction) override
    {
        Vehicle *b = findCarrier(faction);

        if (!b) return false;

        // @NOTE: Currently allows only to defend from only one vehicle around at the same time.
        std::vector<size_t> enemies = findNearestEnemyVehicles(faction,-1, b->getPos(),DEFENSE_RANGE);

        Vehicle *v = NULL;
        if (enemies.size()>0)
        {
            v = entities[enemies[0]];
        }

        if (v)
        {
            Vec3f Po = b->getPos();
            Vec3f Pf = v->getPos();
            Vec3f T = Pf-Po;

            for (size_t i: enemies)
            {
                if (entities[i]->getType() == CARRIER)
                {
                    b->disableAuto();
                    b->setThrottle(0.0);
                }
            }

            if (b->getHealth()<500.0)
            {
                // @NOTE: The idea is to perfom an evasive action and move the carrier away tangential to the island encircle.
                BoxIsland *is = findNearestIsland(b->getPos());
                Vec3f target = is->getPos() - Po;
                target = target.normalize();
                target = target.cross(Vec3f(0.0,1.0,0.0));
                b->setDestination( Po + target * 10000.0);
                b->enableAuto();

            }

            float distance = T.magnitude();

            Vec3f F = b->getForward();
            F = F.normalize();
            T = T.normalize();
            float e = _acos( T.dot(F) );
            float signn = T.cross(F)[1];

            if(Beluga* lb = dynamic_cast<Beluga*>(b))
            {
                if (distance>500 && lb->getWeapons().size()>0 && entities[lb->getWeapons()[0]] != NULL)
                {

                    if (Pf[1]>50.0 || (Pf[1]<50.0 && signn<0))
                    {

                        CarrierTurret* t = (CarrierTurret*)entities[lb->getWeapons()[0]];

                        if (t) {
                            Vehicle *action = t->aimAndFire(world, space, v->getPos());

                            if (action)
                            {
                                entities.push_at_the_back(action, action->getGeom());
                            }
                        }
                    }


                    if (Pf[1]<50.0 && signn<0 && e>PI/6.0)
                    {

                        CarrierArtillery* at = (CarrierArtillery*)entities[lb->getWeapons()[3]];

                        if (at) {
                            Vehicle *action = at->aimAndFire(world, space, v->getPos());

                            if (action)
                            {
                                ((Gunshot*)action)->setOrigin(b->getBodyID());
                                entities.push_at_the_back(action, action->getGeom());
                            }
                        }
                    }

                    if (Pf[1]>50.0 || (Pf[1]<50.0 && signn>0))
                    {
                        CarrierTurret* t2 = (CarrierTurret*)entities[lb->getWeapons()[1]];

                        if (t2)
                        {
                            Vehicle *action = t2->aimAndFire(world, space, v->getPos());

                            if (action)
                            {
                                entities.push_at_the_back(action, action->getGeom());
                            }
                        }
                    }

                    if (Pf[1]<50.0 && signn>0 && e>PI/6.0)
                    {

                        CarrierArtillery* at = (CarrierArtillery*)entities[lb->getWeapons()[2]];

                        if (at) {
                            Vehicle *action = at->aimAndFire(world, space, v->getPos());

                            if (action)
                            {
                                ((Gunshot*)action)->setOrigin(b->getBodyID());
                                entities.push_at_the_back(action, action->getGeom());
                            }
                        }
                    }

                    CarrierLauncher* ct = (CarrierLauncher*)entities[lb->getWeapons()[4]];

                    if (ct) {

                        Vec3f firingloc = ct->getPos();
                        float elevation = -5;
                        float azimuth = getAzimuth(v->getPos()-firingloc);
                        ct->setForward(toVectorInFixedSystem(0,0,1,azimuth, -elevation));

                        Vehicle *action = NULL;

                        if (v->getType() == WALRUS && signn<0 && e>PI/6.0)
                        {
                            ct->water();
                            action = ct->fire(0,world, space);
                        } else if (v->getType() == COLLISIONABLE || v->getType() == CONTROL || v->getType() == CARRIER)   {
                            ct->ground();
                            action = ct->fire(0,world, space);
                        } else {
                            ct->air();
                            action = ct->fire(0,world, space);
                        }

                        if (action)
                        {
                            entities.push_at_the_back(action, action->getGeom());
                            action->goTo(v->getPos());
                            action->enableAuto();

                            // @FIXME: This is to avoid the carrier being damaged by missiles.
                            ((Gunshot*)action)->setOrigin(lb->getBodyID());


                            auto lambda = [](dGeomID sender,dGeomID recv) {

                                Vehicle *snd = entities.find(sender);
                                Vehicle *rec = entities.find(recv);

                                if (snd != NULL && rec != NULL)
                                {
                                    //printf ("Updating....\n");
                                    rec->setDestination(snd->getPos());
                                    return false;
                                }
                                else
                                {
                                    //printf ("End");
                                    return false;
                                }


                            };

                            TrackRecord val;
                            std::get<0>(val) = v->getGeom();
                            std::get<1>(val) = action->getGeom();
                            std::get<2>(val) = lambda;
                            track.push_back(val);
                        }
                    }




                }
            }
            else if(Balaenidae* lb = dynamic_cast<Balaenidae*>(b))
            {

                if (distance>500 && lb->getWeapons().size()>0 && entities[lb->getWeapons()[0]] != NULL)
                {


                    CarrierTurret* t = (CarrierTurret*)entities[lb->getWeapons()[0]];
                    if (t)
                    {
                        Vehicle *action = t->aimAndFire(world, space, v->getPos());

                        if (action)
                        {
                            entities.push_at_the_back(action, action->getGeom());
                        }
                    }

                    if (Pf[1]<50.0 && e<PI/6.0)
                    {
                        CarrierArtillery* at = (CarrierArtillery*)entities[lb->getWeapons()[1]];

                        if (at) {
                            Vehicle *action = at->aimAndFire(world, space, v->getPos());

                            if (action)
                            {
                                ((Gunshot*)action)->setOrigin(b->getBodyID());
                                entities.push_at_the_back(action, action->getGeom());
                            }
                        }
                    }
                }
            }
        }

        return false;
    }
};

class EngageDefCon : public Condition
{
    TSequencer T;

    void start()
    {
        T[0] = 1;
        T[1] = 1;
    }

    void tick()
    {
        T = T + T.sign() * 1;
    }

    public:
    bool runNavalDefense(int faction)
    {
        Vehicle *b = findCarrier(faction);

        if (!b) return false;

        Vehicle *v = findNearestEnemyVehicle(faction,b->getPos(),DEFENSE_RANGE);

        std::cout << "Enemy found at " << v->getPos() << std::endl;

        if (v) 
        {
            // @FIXME Check why this is actually working with the faulty function findWalrusByOrder
            Walrus *w1 = findWalrusByOrder(faction,1);

            if (!w1)
            {
                T[1] = 1;
                std::cout << "Spawn ! " << std::endl;
                w1 = spawnWalrus(space,world,b);
                w1->enableAuto();
            }

            for(int i=1;i<5;i++)
            {
                Walrus *w1 = findWalrusByOrder(faction,i);

                if (w1) continue;

                if (T[1]==100)
                {
                    T[1] = 1;
                    dout << "Spawn ! " << std::endl;
                    w1 = spawnWalrus(space,world,b);
                    w1->enableAuto();
                }
            }
            if (T[1]==300)
                for(int i=1;i<5;i++)
                {
                    Walrus *w = findWalrusByOrder(faction,i);
                    if (w) {w->attack(v->getPos());}
                }
        }
        else
        {
            for(int i=1;i<5;i++)
            {
                Walrus *w = findWalrusByOrder(faction,i);
                if (w) w->goTo(b->getPos());

            }
        }

        return false;

    }
    bool runAirDefense(int faction)
    {
        Vehicle *b = findCarrier(faction);

        if (!b) return false;

        Vehicle *v = findNearestEnemyVehicle(faction,b->getPos(),DEFENSE_RANGE);

        if (v) {
            Manta *m = findMantaByOrder(faction, DEFEND_CARRIER);

            if (m)
            {
                m->doHold(m->getPos(),350);
            }
            T[0] = 1;
            return false;
        }

        Manta *m = findMantaByOrder(faction, DEFEND_CARRIER);

        if (!m)
        {
            size_t idx;
            Manta *m = spawnManta(space,world,b,idx);
            m->setOrder(DEFEND_CARRIER);
            T[0] = 1;
        }


        if (T[0]==300)
        {
            launchManta(b);
        }

        if (T[0]==400)
        {
            Manta *m = findMantaByOrder(faction, DEFEND_CARRIER);

            if (m && v)
            {
                m->dogfight(v->getPos());
                m->enableAuto();
            }
        }
        if (T[0]>405)
        {
            Manta *m = findMantaByOrder(faction, DEFEND_CARRIER);

            if (m && v)
            {
                m->dogfight(v->getPos());
            } 
        }   
        return false;     
    }

    bool evaluate(int faction) override
    {
        Vehicle *b = findCarrier(faction);

        if (!b) return false;

        tick();

        // @NOTE: Currently allows only to defend from only one vehicle around at the same time.
        std::vector<size_t> enemies = findNearestEnemyVehicles(faction,-1, b->getPos(),DEFENSE_RANGE);

        Vehicle *v = NULL;
        if (enemies.size()>0)
        {
            v = entities[enemies[0]];
        }

        if (v && (v->getType() == CARRIER || v->getType() == WALRUS))
        {
            Walrus *w1 = findWalrusByOrder(faction,1);

            if (!w1)
            {
                T[1] = 1;
                std::cout << "Spawn ! " << std::endl;
                w1 = spawnWalrus(space,world,b);
                w1->enableAuto();
            }

            for(int i=1;i<5;i++)
            {
                Walrus *w1 = findWalrusByOrder(faction,i);

                if (w1) continue;

                if (T[1]==100)
                {
                    T[1] = 1;
                    dout << "Spawn ! " << std::endl;
                    w1 = spawnWalrus(space,world,b);
                    w1->enableAuto();
                }
            }
            if (T[1]==500)
                for(int i=1;i<5;i++)
                {
                    Walrus *w = findWalrusByOrder(faction,i);
                    w->enableAuto();
                    if (w) {w->attack(v->getPos());}
                }
        }

        if (v && v->getType() == MANTA)
        {
            return runAirDefense(faction);
        } 
        return false;   
    }
};

Player::Player(int faction)
{
    Player::faction = faction;


    for(int i=0;i<25;i++) qactions[i] = new QAction();
    for(int i=0;i<25;i++) transitions[i] = new Transition(State::IDLE,State::IDLE,new Condition());
    for(int i=0;i<25;i++) interruptions[i] = new Interruption(State::IDLE,new Condition());

    transitions[0] = new Transition(State::IDLE,State::DOCKING,new NotEnoughFuelForNextOperation());            // Power < REQUIRED_FUEL
    transitions[1] = new Transition(State::DOCKING,State::DOCKED,new DockedCondition());                        // SailingStatus::DOCKED
    //transitions[2] = new Transition(State::IDLE,State::IDLE,new EnoughFuelForNextOperation());                  // Power > REQUIRED_FUEL
    transitions[3] = new Transition(State::DOCKED,State::IDLE,new CarrierIsSailing());                          // SailingStatus::SAILING 
    transitions[4] = new Transition(State::IDLE,State::INVADEISLAND,new ClosestIslandIsFree());                 // <10000.0, no command center
    transitions[5] = new Transition(State::IDLE,State::BALLISTICATTACK,new ClosestIslandIsEnemy());             // <10000.0, command center
    transitions[6] = new Transition(State::IDLE,State::APPROACHENEMYISLAND,new ClosestFarAwayIslandIsEnemy());  // >40000.0, closer than any other empty island
    transitions[7] = new Transition(State::IDLE,State::APPROACHFREEISLAND,new ClosestFarAwayIslandIsFree());    // >40000.0, closer than any other enemy island
    transitions[8] = new Transition(State::IDLE,State::APPROACHFRIENDLYISLAND,new ClosestFarAwayIslandIsFriendly());  
    
    transitions[9] = new Transition(State::APPROACHFREEISLAND,State::INVADEISLAND,new CarrierHasArrivedToFreeIsland());     // arrived(), dst_status == REACHED
    transitions[10] = new Transition(State::APPROACHENEMYISLAND,State::BALLISTICATTACK,new CarrierHasArrivedToEnemyIsland(RANGE[1]));    // arrived(), dst_status == REACHED
    transitions[11] = new Transition(State::APPROACHFRIENDLYISLAND,State::IDLE,new CarrierHasArrivedToFriendlyIsland());       // arrived(), dst_status == REACHED
    transitions[12] = new Transition(State::BALLISTICATTACK,State::APPROACHFREEISLAND,new ClosestIslandIsFree(RANGE[1])); 
    transitions[13] = new Transition(State::BALLISTICATTACK,State::AIRBORNEATTACK, new UnitsDeployedForAttack());                  // No enemies around
    transitions[14] = new Transition(State::AIRBORNEATTACK,State::APPROACHFREEISLAND,new ClosestIslandIsFree(RANGE[1]));                  // No enemies around
    transitions[15] = new Transition(State::INVADEISLAND,State::RENDEZVOUS,new ClosestIslandIsFriendly());       // <10000.0, command center
    transitions[16] = new Transition(State::RENDEZVOUS,State::IDLE,new AllUnitsDocked());                       // No units around

    interruptions[0] = new Interruption(State::AIRDEFENSE,new DefCon());                       // No units around
    interruptions[1] = new Interruption(State::AIRDEFENSE,new EngageDefCon());                       // No units around

    qactions[(int)State::IDLE] = new ResetQAction();
    qactions[(int)State::DOCKING] = new DockingQAction();
    qactions[(int)State::DOCKED] = new RefuelQAction();
    qactions[(int)State::APPROACHFREEISLAND] = new ApproachFreeIslandQAction();
    qactions[(int)State::APPROACHENEMYISLAND] = new ApproachEnemyIslandQAction();
    qactions[(int)State::APPROACHFRIENDLYISLAND] = new ApproachFriendlyIslandQAction();
    qactions[(int)State::INVADEISLAND] = new InvadeIslandQAction();
    qactions[(int)State::RENDEZVOUS] = new RendezvousQAction();
    qactions[(int)State::BALLISTICATTACK] = new BallisticAttackQAction();
    qactions[(int)State::AIRBORNEATTACK] = new AirborneAttackQAction();

    qactions[(int)State::AIRDEFENSE] = new QAction();


    state = State::IDLE;
}

State Player::getCurrentState()
{
    return state;
}


void Player::playFaction(unsigned long timer)
{
    State stateprime;
    // Check for enemies nearby and shift strategy if they are present.
    //state = interruption->apply(state,faction,timeevent,timer);

    state = interruptions[0]->transit(faction,state);
    state = interruptions[1]->transit(faction,state);

    if (timer % 1000 == 0)
        std::cout << "Faction:" << faction << "-" << "Status:" << getStateMachineName(state) << "(" << (int)state << ")" << std::endl;

    // Fire the action according to the state.
    qactions[(int)state]->apply(faction);

    //dout << "Statuses -------" << std::endl;
    for(int i=0;i<25;i++)
    {
        //dout << "Status:" << (int)state << std::endl;
        stateprime = transitions[i]->transit(faction,state);
        if (stateprime != state)
        {
            state = stateprime;
            qactions[(int)state]->start();
            break;
        }
    }
}

std::vector<std::vector<int>> mst ;

void Player::playStrategy(unsigned long timer)
{
    // @NOTE: Each carrier check the whole graph of islands and decide based on where the carrier is,
    // what is the next island that it will visit.

    if (state == State::IDLE)
    {
        Vehicle *b = findCarrier(faction);


        if (!b)
            return;


        size_t start = findIndexOfNearestIsland(b->getPos());   // Where I am
        BoxIsland *is = islands[start];

        assert(is != NULL || !"The nearest island is NULL. This should not happen.");

        if (!(is->getCommandCenter()) || (is->getCommandCenter()->getFaction() != faction))
        { 
            nextOperationPerFaction[faction].nextIsland = is; // Not free

        }
        else
        {
            // Closest island is friendly, decide where to go next.  Here is the strategy.

            if (mst.size() == 0)
            {
                // Create the minimum spanning tree of the islands.
                // This is done only once.
                dout << "Creating MST..." << std::endl;
                mst = createIslandGraphMST();

                for (size_t i = 0; i < mst.size(); ++i) {
                    std::cout << "Island " << i << " (" << islands[i]->getName() << ") is connected to: ";
                    for (int neighbor : mst[i]) {
                        std::cout << " [" << neighbor << "]" << " (" << islands[neighbor]->getName() << ") " << (islands[neighbor]->getPos() - islands[i]->getPos()).magnitude();
                    }
                    std::cout << std::endl;
                }
            }
            

            size_t end = start;  // Default to the current island if no empty island is found
            // Find index of nearest EMPTY island
            BoxIsland *destiny = findNearestEmptyIsland(b->getPos());

            if (!destiny) {
                std::cout << "No empty island found. Will attack enemy islands." << std::endl;
                destiny = findNearestEnemyIsland(b->getPos(), 3000 kmf);
            }


            // @FIXME: destiny can be null when all the islands are taken.
            //.  Pick free (it will attack enemy islands in the middle)
            //.  Start attacking enemy islands purposedly
            //.  Find the enemy carrier and destroy it.


            for(size_t i = 0; i < islands.size(); ++i) {
                if (islands[i] == destiny) {
                    std::cout << "Nearest empty island is: " << destiny->getName() << " at index " << i << std::endl;
                    end = i;  // Update the destination index
                }
            }

            std::vector<int> path = getShortestIslandPathMST(start, end);

            if (!path.empty()) {
                std::cout << "Shortest path: ";
                for (int idx : path) {
                    std::cout << idx << " (" << islands[idx]->getName() << ") ";
                }
                std::cout << std::endl;
            } else {
                std::cout << "No path found!" << std::endl;
                assert(false || !"No path found. Island is unreachable.");
            }

            nextOperationPerFaction[faction].nextIsland = islands[path[1]];  // Set the second island in the path as the next operation target (0 is where I am)
        }
        if (nextOperationPerFaction[faction].nextIsland && faction == GREEN_FACTION )
            std::cout << "Faction:" << faction << "-" << "Next Operation:" << nextOperationPerFaction[faction].nextIsland->getName() << std::endl;
    }

}


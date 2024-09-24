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

extern unsigned long timer;

extern dWorldID world;
extern dSpaceID space;

float REQUIRED_FUEL;

float RANGE[] = {40000.0, 20000.0, 6000.0};

class DockingQAction : public QAction
{   
public:
    void apply(int faction)
    {
        Vehicle *b = findCarrier(faction);

        if (b && b->getPower()<REQUIRED_FUEL && b->getAutoStatus() != AutoStatus::DOCKING)
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
                            if ( (getIslandCargo(is,CargoTypes::POWERFUEL)+b->getPower()) > REQUIRED_FUEL)
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
            BoxIsland *is = findNearestEnemyIsland(b->getPos(),false, faction,1000 kmf);

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

            std::cout << "Distance:" << (is->getPos()-b->getPos()).magnitude() << std::endl;

            if (is && (is->getPos()-b->getPos()).magnitude()<range)
            {
                Structure *sc = is->getCommandCenter();

                std::cout << sc << std::endl;

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

            std::cout << "Distance:" << (is->getPos()-b->getPos()).magnitude() << std::endl;

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
        if (b && b->getPower()>REQUIRED_FUEL)
        {
            //BoxIsland *enemyis = findNearestIsland(b->getPos(), false, faction);

            BoxIsland *freeis   =    findNearestEmptyIsland(b->getPos());
            BoxIsland *enemyis  =    findNearestEnemyIsland(b->getPos(),false, faction);


            dout << freeis << std::endl;
            dout << enemyis << std::endl;

            if (!freeis && !enemyis)
                return false;

            if (freeis)
                dout << "Free island:" << (freeis->getPos()-b->getPos()).magnitude() << std::endl;

            if (freeis && !enemyis)
                return true;

            if (enemyis && !freeis)
                return false;

            if (freeis && enemyis)
            {
                if ((freeis->getPos()-b->getPos()).magnitude() > RANGE[0] && (freeis->getPos()-b->getPos()).magnitude() < (enemyis->getPos()-b->getPos()).magnitude())
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
        if (b && b->getPower()>REQUIRED_FUEL)    
        {
            //BoxIsland *enemyis = findNearestIsland(b->getPos(), false, faction);

            BoxIsland *freeis   =    findNearestEmptyIsland(b->getPos());
            BoxIsland *enemyis  =    findNearestEnemyIsland(b->getPos(),false, faction);

            if (!freeis && !enemyis)
                return false;

            if (!freeis && enemyis)
                return true;

            if (!enemyis && freeis)
                return false;

            if (freeis && enemyis)
            {
                if ((enemyis->getPos()-b->getPos()).magnitude() > RANGE[0] && (enemyis->getPos()-b->getPos()).magnitude() < (freeis->getPos()-b->getPos()).magnitude())
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

        if (b)
        {
            BoxIsland *freeis   =    findNearestEmptyIsland(b->getPos());
            BoxIsland *enemyis  =    findNearestEnemyIsland(b->getPos(),false, faction);

            Vec3f destination;

            if (!freeis && !enemyis)
                return false;

            if (freeis && !enemyis)
            {
                destination = freeis->getPos();
            }
            else
            if (enemyis && !freeis)
            {
                destination = enemyis->getPos();
            }
            else
            {
                if ((freeis->getPos()-b->getPos()).magnitude() < (enemyis->getPos()-b->getPos()).magnitude())
                {
                    destination = freeis->getPos();
                }
                else
                {
                    destination = enemyis->getPos();
                }
            }


            float distance = (destination-b->getPos()).magnitude();
            float requiredfuel = distance * (  1000.0 / 254000.0 );

            assert ( requiredfuel <= 1000.0 || !"The required fuel is too high. This should not happen.");

            REQUIRED_FUEL = (requiredfuel + 50.0);

            if (REQUIRED_FUEL > 1000.0)
                REQUIRED_FUEL = 1000.0;

            std::cout << "Required fuel for next operation:" << requiredfuel << std::endl;


            char msg[256];
            Message mg;
            mg.faction = b->getFaction();
            sprintf(msg, "Required fuel for next operation %5.2f.", REQUIRED_FUEL);
            mg.msg = std::string(msg); mg.timer = timer;
            messages.insert(messages.begin(), mg);


            if (b->getPower()>REQUIRED_FUEL)
            {
                return true;
            }

            return false;
        }
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
            REQUIRED_FUEL = 1000.0;
            return ClosestIslandIsFree::evaluate(faction);
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
                        REQUIRED_FUEL = 1000.0;
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


Player::Player(int faction)
{
    Player::faction = faction;


    for(int i=0;i<25;i++) qactions[i] = new QAction();
    for(int i=0;i<25;i++) transitions[i] = new Transition(State::IDLE,State::IDLE,new Condition());

    transitions[0] = new Transition(State::IDLE,State::DOCKING,new NotEnoughFuelForNextOperation());            // Power < REQUIRED_FUEL
    transitions[1] = new Transition(State::DOCKING,State::DOCKED,new DockedCondition());                        // SailingStatus::DOCKED
    //transitions[2] = new Transition(State::IDLE,State::IDLE,new EnoughFuelForNextOperation());                  // Power > REQUIRED_FUEL
    transitions[3] = new Transition(State::DOCKED,State::IDLE,new CarrierIsSailing());                          // SailingStatus::SAILING 
    transitions[4] = new Transition(State::IDLE,State::INVADEISLAND,new ClosestIslandIsFree());                 // <10000.0, no command center
    transitions[5] = new Transition(State::IDLE,State::BALLISTICATTACK,new ClosestIslandIsEnemy());             // <10000.0, command center
    transitions[6] = new Transition(State::IDLE,State::APPROACHENEMYISLAND,new ClosestFarAwayIslandIsEnemy());  // >40000.0, closer than any other empty island
    transitions[7] = new Transition(State::IDLE,State::APPROACHFREEISLAND,new ClosestFarAwayIslandIsFree());    // >40000.0, closer than any other enemy island
    transitions[8] = new Transition(State::APPROACHFREEISLAND,State::INVADEISLAND,new CarrierHasArrivedToFreeIsland());     // arrived(), dst_status == REACHED
    transitions[9] = new Transition(State::APPROACHENEMYISLAND,State::BALLISTICATTACK,new CarrierHasArrivedToEnemyIsland(RANGE[1]));    // arrived(), dst_status == REACHED
    transitions[10] = new Transition(State::BALLISTICATTACK,State::APPROACHFREEISLAND,new ClosestIslandIsFree(RANGE[1])); 
    transitions[11] = new Transition(State::BALLISTICATTACK,State::AIRBORNEATTACK, new UnitsDeployedForAttack());                  // No enemies around
    transitions[12] = new Transition(State::AIRBORNEATTACK,State::APPROACHFREEISLAND,new ClosestIslandIsFree(RANGE[1]));                  // No enemies around
    transitions[13] = new Transition(State::INVADEISLAND,State::RENDEZVOUS,new ClosestIslandIsFriendly());       // <10000.0, command center
    transitions[14] = new Transition(State::RENDEZVOUS,State::IDLE,new AllUnitsDocked());                       // No units around


    qactions[(int)State::IDLE] = new ResetQAction();
    qactions[(int)State::DOCKING] = new DockingQAction();
    qactions[(int)State::DOCKED] = new RefuelQAction();
    qactions[(int)State::APPROACHFREEISLAND] = new ApproachFreeIslandQAction();
    qactions[(int)State::APPROACHENEMYISLAND] = new ApproachEnemyIslandQAction();
    qactions[(int)State::INVADEISLAND] = new InvadeIslandQAction();
    qactions[(int)State::RENDEZVOUS] = new RendezvousQAction();
    qactions[(int)State::BALLISTICATTACK] = new BallisticAttackQAction();
    qactions[(int)State::AIRBORNEATTACK] = new AirborneAttackQAction();


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

    if (timer % 1000 == 0)
        std::cout << "Status:" << (int)state << std::endl;

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

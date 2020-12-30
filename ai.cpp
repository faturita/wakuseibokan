#include <assert.h>
#include "engine.h"
#include "ai.h"

extern  Controller controller;

/* dynamics and collision objects */

extern container<Vehicle*> entities;
extern std::vector<BoxIsland*> islands;
extern std::vector<Message> messages;

extern dWorldID world;
extern dSpaceID space;

int DefCon::apply(int state, int faction, unsigned long &timeevent, unsigned long timer)
{
    Vehicle *b = findCarrier(faction);

    if (!b) return state;

    Vehicle *v = findNearestEnemyVehicle(faction,b->getPos(),DEFENSE_RANGE);

    if (v && (v->getType() == CARRIER || v->getType() == WALRUS) && state != 20 && state != 21)
    {
        // Shift to the state to send walruses to destroy the enemy carrier.
        Walrus *w = findNearestWalrus(faction,b->getPos(),DEFENSE_RANGE);

        if (!w) {timeevent = timer;return 20;}
    }

    if (v && v->getType() == MANTA && state != 22 && state != 23)
    {
        // Shift to the state to send walruses to destroy the enemy carrier.
        Manta *m = findMantaByOrder(faction,DEFEND_CARRIER);

        if (m) { m->dogfight(v->getPos()); }
        else {timeevent=timer;return 22;}
    }
    return state;
}



int AirDefense::apply(int state, int faction, unsigned long &timeevent, unsigned long timer)
{
    Vehicle *b = findCarrier(faction);

    if (!b) return state;

    Vehicle *v = findNearestEnemyVehicle(faction,b->getPos(),DEFENSE_RANGE);

    if (!v) {
        Manta *m = findMantaByOrder(faction, DEFEND_CARRIER);

        if (m)
        {
            m->doHold(m->getPos(),350);

            timeevent=timer;return 4;
        }
    }

    if (timer==(timeevent + 200))
    {
        Manta *m = findMantaByOrder(faction, DEFEND_CARRIER);

        if (!m)
        {
            size_t idx;
            Manta *m = spawnManta(space,world,b,idx);
            m->setOrder(DEFEND_CARRIER);
        }

    }

    if (timer==(timeevent + 300))
    {
        launchManta(b);
    }

    if (timer==(timeevent + 400))
    {
        Manta *m = findMantaByOrder(faction, DEFEND_CARRIER);

        if (m && v)
        {
            m->dogfight(v->getPos());
            m->enableAuto();
        }
    }
    if (timer>(timeevent + 405))
    {
        Manta *m = findMantaByOrder(faction, DEFEND_CARRIER);

        if (m && v)
        {
            m->dogfight(v->getPos());
        } else {
            timeevent = timer;
        }
    }
    return state;
}




int NavalDeffense::apply(int state, int faction, unsigned long &timeevent, unsigned long timer)
{
    Vehicle *b = findCarrier(faction);

    if (!b) return state;

    Vehicle *v = findNearestEnemyVehicle(faction,b->getPos(),DEFENSE_RANGE);

    std::cout << "read" << v->getPos() << std::endl;
    Walrus* w1 = spawnWalrus(space,world,b);

    Walrus* w2 = spawnWalrus(space,world,b);

    //w1->attack(b->getPos());
    w1->enableAuto();

    //w2->attack(b->getPos());
    w2->enableAuto();


    timeevent=timer;return 21;

    return state;
}




int NavalDeffending::apply(int state, int faction, unsigned long &timeevent, unsigned long timer)
{
    Vehicle *b = findCarrier(faction);

    if (!b) return state;

    Vehicle *v = findNearestEnemyVehicle(faction,b->getPos(),DEFENSE_RANGE);

    Walrus *w1 = findWalrusByOrder(faction,1);
    Walrus *w2 = findWalrusByOrder(faction,2);

    if (v)
    {
        if (w1) w1->attack(v->getPos());
        if (w2) w2->attack(v->getPos());

        if (!w1 && !w2) {timeevent=timer;return 20;}
    }
    else
    {
        timeevent= timer;return 3;
    }
    return state;
}




int ApproachEnemyIsland::apply(int state, int faction, unsigned long &timeevent, unsigned long timer)
{
    Vehicle *b = findCarrier(faction);

    if (b)
    {
        BoxIsland *is = findNearestEnemyIsland(b->getPos(),false, faction,100 kmf);

        if (!is)
        {
            // Closest enemy island is not found, lets check for an empty island.
            return 0;
        }

        Vec3f vector = (b->getPos()) - (is->getPos());

        vector = vector.normalize();

        b->setDestination(is->getPos()+Vec3f(12500.0f * vector));
        b->enableAuto();
        timeevent=timer;return 10;
    }
    return state;
}


int ApproachAnyEnemyIsland::apply(int state, int faction, unsigned long &timeevent, unsigned long timer)
{
    Vehicle *b = findCarrier(faction);

    if (b)
    {
        BoxIsland *is = findNearestEnemyIsland(b->getPos(),false, faction,1000000 kmf);

        if (!is)
        {
            // Closest enemy island is not found, lets check for an empty island.
            return 0;
        }

        Vec3f vector = (b->getPos()) - (is->getPos());

        vector = vector.normalize();

        b->setDestination(is->getPos()+Vec3f(12500.0f * vector));
        b->enableAuto();
        timeevent=timer;return 10;
    }
    return state;
}




int ApproachingEnemyIsland::apply(int state, int faction, unsigned long &timeevent, unsigned long timer)
{
    Vehicle *b = findCarrier(faction);

    if (b)
    {
        if (!b->isAuto())
        {
            printf("Carries has arrived to destination.\n");

            //Walrus* w = spawnWalrus(space,world,b);
            //w->setDestination(is->getPos());
            //w->enableAuto();
            timeevent = timer; return 11;

        }
    }
    return state;
}




int BallisticAttack::apply(int state, int faction, unsigned long &timeevent, unsigned long timer)
{
    Vehicle *b = findCarrier(faction);

    if (b)
    {
        BoxIsland *is = findNearestEnemyIsland(b->getPos(),false, faction);

        std::cout << "T-:" << (int)((timeevent+100)-timer) << std::endl;

        // @FIXME If it has available missiles, then fire them.

        if (timer==(timeevent + 100))
        {
            std::cout << "FIRE!" << std::endl;

            Missile *a = (Missile*) b->fire(world, space);

            size_t i = CONTROLLING_NONE;
            if (a)
            {
                i = entities.push_back(a,a->getGeom());

                CommandCenter *c = (CommandCenter*)is->getCommandCenter();

                a->setDestination(c->getPos());

                a->enableAuto();

                if (a->getType()==CONTROLABLEACTION)
                {
                    // @FIXME: Shoot only if the faction of the user controlling is the same as this one.
                    switchControl(entities.indexOf(i));

                }
            }

        }

        // @NOTE: Should check how many missiles can the carrier shoot.
        if (timer==(timeevent + 1300))
        {
            if (is)
            {
                CommandCenter *c = (CommandCenter*)is->getCommandCenter();

                if (c)
                {
                    timeevent = timer;return 12;
                }
                else
                {
                    timeevent=timer;return 0;
                }
            }

        }
    }
    return state;
}




int AirborneAttack::apply(int state, int faction, unsigned long &timeevent, unsigned long timer)
{
    Vehicle *b = findCarrier(faction);

    if (b)
    {
        if (timer==(timeevent + 200))
        {
            Manta *m = findMantaByOrder(faction, ATTACK_ISLAND);

            if (!m)
            {
                size_t idx = 0;
                Manta *m = spawnManta(space,world,b, idx);

                m->setOrder(ATTACK_ISLAND);
            }
        }

        if (timer==(timeevent + 300))
        {
            // @FIXME I need to register which manta is around.
            Manta *m = launchManta(b);              // This works because it launchs the manta on deck
        }

        if (timer==(timeevent + 400))
        {
            BoxIsland *i = findNearestIsland(b->getPos());

            //Manta *m = findNearestManta(Manta::FLYING,faction,b->getPos());

            Manta *m = findMantaByOrder(faction, ATTACK_ISLAND);

            assert ( m != NULL );

            CommandCenter *c = (CommandCenter*)i->getCommandCenter();

            if (m)
            {
                if (!c)
                {
                    m->setDestination(b->getPos());
                    m->enableAuto() ;                    // @FIXME: This needs to be performed all the time while manta is landing.
                    return 0;
                } else
                {

                    m->attack(c->getPos());
                    m->enableAuto();
                }
            }
        }


        if (timer>(timeevent + 400))
        {
            BoxIsland *i = findNearestIsland(b->getPos());

            //Manta *m = findNearestManta(Manta::FLYING, faction,b->getPos());

            Manta *m = findMantaByOrder(faction, ATTACK_ISLAND);

            if (!m)
            {

                timeevent = timer;
                return 11;
            }

            CommandCenter *c = (CommandCenter*)i->getCommandCenter();

            if (!c)
            {
                // Command Center is destroyed.

                if (m)
                {
                    m->setDestination(Vec3f(i->getPos()[0],1000,i->getPos()[2]));
                    m->enableAuto();
                }


                return 0;
            }
        }
    }
    return state;
}




int ApproachFreeIsland::apply(int state, int faction, unsigned long &timeevent, unsigned long timer)
{
    Vehicle *b = findCarrier(faction);

    if (b)
    {
        //BoxIsland *enemyis = findNearestIsland(b->getPos(), false, faction);

        BoxIsland *is = findNearestEmptyIsland(b->getPos());

        if (is)
        {
            Vec3f vector = (b->getPos()) - (is->getPos());

            vector = vector.normalize();

            b->setDestination(is->getPos()+Vec3f(3500.0f * vector));
            b->enableAuto();
            timeevent = timer; return 1;
        } else {
            timeevent = timer; return 13;
        }
    }

    return state;
}




int InvadeIsland::apply(int state, int faction, unsigned long &timeevent, unsigned long timer)
{
    Vehicle *b = findCarrier(faction);

    if (b)
    {
        BoxIsland *is = findNearestIsland(b->getPos());

        // Check if the island is still free
        if (is->getCommandCenter() && is->getCommandCenter()->getFaction()!=faction)
        {
            timeevent=timer;return 13;
        }

        if (!b->isAuto())
        {
            printf("Carrier has arrived to destination.\n");

            Walrus *w = findWalrus(faction);

            if (!w)
            {
                w = spawnWalrus(space,world,b);
            }
            w->setDestination(is->getPos());
            w->enableAuto();

            timeevent = timer;return 2;

        }
    }

    return state;
}



int CaptureIsland::apply(int state, int faction, unsigned long &timeevent, unsigned long timer)
{
    // Need to check if walrus arrived to island.
    Walrus *w = findWalrus(faction);

    if (w)
    {
        BoxIsland *is = findNearestEmptyIsland(w->getPos());

        // @FIXME: It may happen that the walrus is wandering do nothing never de-autopilot and this is endless.
        if (!w->isAuto() || ( w->getIsland() == is && timer>(timeevent + 1000))  )
        {
            // @FIXME: Decide which island to create.

            assert( ( is != NULL && w->getIsland() != NULL ) || !"The island and the Walrus' island are both null. This should not happen.");
            int which = (rand() % 3);
            captureIsland(is,w->getFaction(),which,space, world);
            timeevent = timer; return 3;
        }
    } else
    {
        // Walrus has been destroyed.
        timeevent = timer; return 1;
    }

    return state;
}




int ReturnToCarrier::apply(int state, int faction, unsigned long &timeevent, unsigned long timer)
{
    Vehicle *b = findCarrier(faction);

    // @FIXME Do something when the carrier is destroyed.
    if (!b)
    {
        return state;
    }

    bool found = false;

    // Send walruses of my faction which are in range and send them closer to the carrier.
    for(size_t i=entities.first();entities.hasMore(i);i=entities.next(i))
    {
        Vehicle *v=entities[i];
        if (v->getType() == WALRUS && v->getFaction() == faction)
        {
            if ((v->getPos()-b->getPos()).magnitude()<IN_RANGE)   {
                Walrus *w = (Walrus*)v;
                Vec3f t = b->getPos() - w->getPos();
                t = t.normalize();

                Vec3f up = Vec3f(0,1,0);

                t = t.cross(up);
                t = t.normalize();

                w->setDestination(b->getPos()+t*300);
                w->enableAuto();
                found = true;
            }

        }
    }


    if (found)
    {
        // Dock the walruses.
        timeevent=timer;return 4;
    } else {
        // There are no more walruses so I can departure to the next island.
        //timeevent=timer;return 9;
    }

    return state;
}




int DockBack::apply(int state, int faction, unsigned long &timeevent, unsigned long timer)
{
    Vehicle *b = findCarrier(faction);

    if (!b)
        return state;

    bool done1 = true;
    bool done2 = true;

    // Find walruses around and dock them.
    for(size_t i=entities.first();entities.hasMore(i);i=entities.next(i))
    {
        Vehicle *v=entities[i];
        if (v->getType() == WALRUS && v->getFaction() == faction)
        {
            if ((v->getPos()-b->getPos()).magnitude()<10000)   {

                done1 = false;
                Walrus *w = (Walrus*)v;
                if (!w->isAuto())
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
    Manta *m1 = findManta(faction,Manta::ON_DECK);
    Manta *m2 = findNearestManta(Manta::FLYING,b->getFaction(),b->getPos(),30000);
    Manta *m3 = findNearestManta(Manta::HOLDING,b->getFaction(), b->getPos(),30000);

    if (m2)
    {
        // Updating the current carrier postion to Manta so that it improves landing.
        //m2->setDestination(b->getPos());
        //m2->setAttitude(b->getForward());
        done2 = false;
    } else
    {
        if (m3)
        {
            landManta(b,m3);
            done2 = false;
        }
    }


    if (m1)
    {
        synchronized(entities.m_mutex)
        {
            dockManta();
        }
    }

    if (done1 && done2)
    {
        // There are no walrus so I can departure to the next island.
        timeevent=timer;return 9;
    }

    return state;
}

Player::Player(int faction)
{
    Player::faction = faction;

    /**auto lambda = [](float a, float b) {
        return (std::abs(a) < std::abs(b));
    };**/

    interruption = new DefCon();

    for(int i=0;i<25;i++) qactions[i] = new QAction();

    qactions[22] = new AirDefense();
    qactions[20] = new NavalDeffense();
    qactions[21] = new NavalDeffending();
    qactions[9]  = new ApproachEnemyIsland();
    qactions[10] = new ApproachingEnemyIsland();
    qactions[11] = new BallisticAttack();
    qactions[12] = new AirborneAttack();
    qactions[13] = new ApproachAnyEnemyIsland();
    qactions[0] = new ApproachFreeIsland();
    qactions[1] = new InvadeIsland();
    qactions[2] = new CaptureIsland();
    qactions[3] = new ReturnToCarrier();
    qactions[4] = new DockBack();

    state = 9;
    timeevent = 0;

}


int Player::pickQAction()
{
    return state;
}


void Player::playFaction(unsigned long timer)
{
    // Check for enemies nearby and shift strategy if they are present.
    state = interruption->apply(state,faction,timeevent,timer);

    //std::cout << "Status:" << state << std::endl;

    // Fire the action according to the state.
    state = qactions[state]->apply(state,faction,timeevent,timer);

}

/* ============================================================================
**
** AI - Wakuseiboukan - Around 2020
**
** Copyright (C) 2014  Rodrigo Ramele
**
** For personal, educationnal, and research purpose only, this software is
** provided under the Gnu GPL (V.3) license. To use this software in
** commercial application, please contact the author.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License V.3 for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
**
** This is a general state machine, on top of all the others that drive any
** individual unit. It is raw.  I've found that it is much more difficult to
** do something generic, nice, that actually works.
** When you need something workable you have to cook up a lot of details.
** Player represent an AI that tries to behave like a human player.  This has
** been a mantra on this project. Hence each class is a state in the sequence
** of steps that you need to perform to win the game, and each transition
** represent some action.  The idea was to model like state-action diagram,
** able for use in a RL model.
** ========================================================================= */


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

// This runs all the time, on every frame.  It checks whether or not there is a threat nearby the carrier.
// @FIXME: We should do the same for every unit on existence.
// @FIXME: We could extend that, including the buildings from each island.
// @FIXME: Finally, we need a system to read information from sensors to sense the world.
// @NOTE: We need to be aware that there could be a moment when the carrier has no weapons.
int DefCon::apply(int state, int faction, unsigned long &timeevent, unsigned long timer)
{
    Vehicle *b = findCarrier(faction);

    if (!b) return state;

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
                                return true;
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


// This is a state machine to grab a Manta and send it towards any incoming airplane.
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



// Spawn walruses to defend the carrier of incoming boats.
int NavalDefense::apply(int state, int faction, unsigned long &timeevent, unsigned long timer)
{
    Vehicle *b = findCarrier(faction);

    if (!b) return state;

    Vehicle *v = findNearestEnemyVehicle(faction,b->getPos(),DEFENSE_RANGE);

    dout << "read" << v->getPos() << std::endl;

    // @FIXME Check why this is actually working with the faulty function findWalrusByOrder
    Walrus *w1 = findWalrusByOrder(faction,1);

    if (!w1)
    {
        set(timer);
        dout << "Spawn ! " << std::endl;
        w1 = spawnWalrus(space,world,b);
        w1->enableAuto();
    }

    for(int i=1;i<5;i++)
    {
        Walrus *w1 = findWalrusByOrder(faction,i);

        if (w1) continue;

        if (delay(timer,100))
        {
            set(timer);
            dout << "Spawn ! " << std::endl;
            w1 = spawnWalrus(space,world,b);
            w1->enableAuto();
        }

        timeevent=timer;return state;
    }


    timeevent=timer;return 21;

}



// Control the walruses and instruct them to defend the carrier.
int NavalDefending::apply(int state, int faction, unsigned long &timeevent, unsigned long timer)
{
    Vehicle *b = findCarrier(faction);

    if (!b) return state;

    // @NOTE: Currently allows only to defend from only one vehicle around at the same time.
    Vehicle *v = findNearestEnemyVehicle(faction,b->getPos(),DEFENSE_RANGE);


    if (v)
    {
        bool all = true;
        for(int i=1;i<5;i++)
        {
            Walrus *w = findWalrusByOrder(faction,i);
            if (w) {w->attack(v->getPos());all=false;}

        }

        if (all) {timeevent=timer;return 20;} // No More walruses, spawn more.
    }
    else
    {
        for(int i=1;i<5;i++)
        {
            Walrus *w = findWalrusByOrder(faction,i);
            if (w) w->goTo(b->getPos());

        }
        timeevent= timer;return 3;
    }

    return state;
}

// Find a closer enemy island and attack it.
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

        Vec3f approachaddress = getRandomCircularSpot(is->getPos()+Vec3f(12500.0f * vector),400.0);

        b->goTo(approachaddress);
        b->enableAuto();
        timeevent=timer;return 10;
    }
    return state;
}


// Find any enemy island
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

        b->goTo(is->getPos()+Vec3f(12500.0f * vector));
        b->enableAuto();
        timeevent=timer;return 10;
    }
    return state;
}



// Travelling towards the designated island.
int ApproachingEnemyIsland::apply(int state, int faction, unsigned long &timeevent, unsigned long timer)
{
    Vehicle *b = findCarrier(faction);

    if (b)
    {
        if (!b->isAuto())
        {
            CLog::Write(CLog::Debug,"Carries has arrived to destination.\n");

            //Walrus* w = spawnWalrus(space,world,b);
            //w->setDestination(is->getPos());
            //w->enableAuto();
            timeevent = timer; return 11;

        }
    }
    return state;
}



// Missile attack
int BallisticAttack::apply(int state, int faction, unsigned long &timeevent, unsigned long timer)
{
    Vehicle *b = findCarrier(faction);

    if (b)
    {
        BoxIsland *is = findNearestIsland(b->getPos());

        dout << "T-:" << (int)((timeevent+100)-timer) << std::endl;

        // @FIXME If it has available missiles, then fire them.

        if (timer==(timeevent + 100))
        {
            dout << "FIRE!" << std::endl;

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



// Spawn mantas and attack the island.
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
                    m->goTo(b->getPos());
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

            //Manta *m = findNearestManta(FlyingStatus::FLYING, faction,b->getPos());

            Manta *m = findMantaByOrder(faction, ATTACK_ISLAND);

            if (!m)
            {
                // Launch a new airplane.
                timeevent = timer;
                return 11;
            }

            CommandCenter *c = (CommandCenter*)i->getCommandCenter();

            if (!c)
            {
                // Command Center is destroyed.

                if (m)
                {
                    ((SimplifiedDynamicManta*)m)->hold();
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

            Vec3f finaldestiny = getRandomCircularSpot(is->getPos()+Vec3f(3500.0f * vector),150.0);

            b->goTo(finaldestiny);
            b->enableAuto();
            timeevent = timer; return 1;
        } else {
            timeevent = timer; return 13;
        }
    }

    return state;
}




int AirborneInvadeIsland::apply(int state, int faction, unsigned long &timeevent, unsigned long timer)
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

        if (!b->isAuto() && timer==(timeevent + 800))
        {
            Manta *m = findMantaByOrder(faction, CONQUEST_ISLAND);

            if (!m)
            {
                m = spawnCephalopod(space,world,b);
            }
            //m->setDestination(is->getPos());
            //m->enableAuto();

            assert( m!=NULL || !"The carrier seems to have the wrong faction.");

            m->setOrder(CONQUEST_ISLAND);

        }


        if (timer==(timeevent + 1200))
        {
            Manta *m = launchManta(b);              // This works because it launchs the manta on deck
        }

        if (timer==(timeevent + 1600))
        {
            Manta *m = findMantaByOrder(faction, CONQUEST_ISLAND);

            if (m)
            {
                m->goTo(is->getPos());
                m->enableAuto();
            }
        }

        if (timer>(timeevent + 1600) && timer<(timeevent + 3500))
        {
            Manta *m = findMantaByOrder(faction, CONQUEST_ISLAND);

            if (!m)
            {
                timeevent = timer;
                return state;
            }

            Vec3f d = m->getPos() - is->getPos();
            CLog::Write(CLog::Debug,"Magnitude: %10.5f\n", d.magnitude());

            Cephalopod *c = (Cephalopod*)m;
            if (c->getIsland() == NULL && d.magnitude()<800)
            {
                // The drone is instructed to drop to the floor.
                c->drop();
            }
        }

        if (timer>(timeevent + 3800))
        {
            Manta *m = findMantaByOrder(faction, CONQUEST_ISLAND);

            if (!m)
            {
                timeevent = timer;
                return state;
            }

            Cephalopod *c = (Cephalopod*)m;
            if (c->getIsland() != NULL)
            {
                // @FIXME: Decide which island to create
                int which = (arc4random() % 3);
                captureIsland(c,is,m->getFaction(),which,space, world);
                //landManta(b,c);

                m->goTo(b->getPos());
                m->enableAuto();

                timeevent = timer;
                return 4;
            }
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
            CLog::Write(CLog::Debug,"Carrier has arrived to destination.\n");

            Walrus *w = findWalrus(faction);

            if (!w)
            {
                w = spawnWalrus(space,world,b);
            }
            w->goTo(getRandomCircularSpot(is->getPos(),200.0));
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
            Vec3f d = w->getPos() - is->getPos();

            if (d.magnitude() < 1800.0)     // This is connected to the Terrain, and to how the Walrus actually moves.
            {
                // @FIXME: Decide which island to create.
                assert( ( is != NULL && w->getIsland() != NULL ) || !"The island and the Walrus' island are both null. This should not happen.");
                int which = (rand() % 3);
                captureIsland(is,w->getFaction(),which,space, world);
                timeevent = timer; return 3;
            }
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

    // @FIXME Do something when the carrier is destroyed (perhaps walruses can become rogue squadrons destroying everything they found).
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

                w->goTo(b->getPos()+t*150);             // @FIXME: This should be just on the back of the carrier.
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
        // Updating the current carrier postion to Manta so that it improves landing.
        //m2->setDestination(b->getPos());
        m2->setAttitude(b->getForward());
        done2 = false;
    } else
    {
        if (m3)
        {
            if (m3->getAutoStatus() != AutoStatus::LANDING)
                landManta(b,m3);
            b->stop();
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
    qactions[20] = new NavalDefense();
    qactions[21] = new NavalDefending();
    qactions[9]  = new ApproachEnemyIsland();
    qactions[10] = new ApproachingEnemyIsland();
    qactions[11] = new BallisticAttack();
    qactions[12] = new AirborneAttack();
    qactions[13] = new ApproachAnyEnemyIsland();
    qactions[0] = new ApproachFreeIsland();

    if (faction == BLUE_FACTION)
        qactions[1] = new AirborneInvadeIsland();
    else
        qactions[1] = new InvadeIsland();

    qactions[2] = new CaptureIsland();
    qactions[3] = new ReturnToCarrier();
    qactions[4] = new DockBack();

    state = 4;
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

    //dout << "Status:" << state << std::endl;

    // Fire the action according to the state.
    state = qactions[state]->apply(state,faction,timeevent,timer);

}

#include "engine.h"
#include "profiling.h"

extern  Controller controller;

/* dynamics and collision objects */


extern container<Vehicle*> entities;

extern std::vector<BoxIsland*> islands;

extern std::vector<Message> messages;

// SYNC
Vehicle* gVehicle(dGeomID geom)
{
    Vehicle *v = entities.find(geom);

    return v;
}

void gVehicle(Vehicle* &v1, Vehicle* &v2, Structure* &s1, Structure* &s2, dGeomID g1, dGeomID g2)
{

    Vehicle *vehicle = entities.find(g1);
    if (vehicle && dGeomGetBody(g1) && vehicle->getGeom() == g1)
    {
        v1 = vehicle;
    }
    if (vehicle && vehicle->getGeom() == g1 && vehicle->getType() >= COLLISIONABLE)
    {
        s1 = (Structure*)vehicle;
    }

    vehicle = entities.find(g2);
    if (vehicle && dGeomGetBody(g2))
    {
        v2 = vehicle;
    }
    if (vehicle && vehicle->getGeom() == g2 && vehicle->getType() >= COLLISIONABLE)
    {
        s2 = (Structure*)vehicle;
    }

}

// SYNC
bool stranded(Vehicle *carrier, Island *island)
{
    if (island && carrier && carrier->getType() == CARRIER && carrier->getStatus() != Balaenidae::OFFSHORING)
    {
        Balaenidae *b = (Balaenidae*)carrier;

        b->offshore();
        controller.reset();
        b->stop();
        b->damage(100);
        b->setControlRegisters(controller.registers);
        b->setStatus(Balaenidae::OFFSHORING);
        b->disableAuto();  // Check the controller for the enemy aircraft.  Force field is disabled for destiny island.
        char str[256];
        Message mg;
        mg.faction = b->getFaction();
        sprintf(str, "Carrier has stranded on %s.", island->getName().c_str());
        mg.msg = std::string(str);
        messages.insert(messages.begin(), mg);
    }
}

bool departed(Vehicle *walrus)
{
    if (walrus && walrus->getType() == WALRUS && walrus->getStatus() == Walrus::ROLLING)
    {
        Walrus *w = (Walrus*)walrus;

        w->setStatus(Walrus::OFFSHORING);
        BoxIsland *island = w->getIsland();
        w->setIsland(NULL);
        char str[256];
        Message mg;
        sprintf(str, "Walrus %d has departed from %s.", NUMBERING(w->getNumber()), island->getName().c_str());
        mg.msg = std::string(str);
        mg.faction = w->getFaction();
        messages.insert(messages.begin(), mg);
    }
    return true;
}

// SYNC
bool arrived(Vehicle *walrus, Island *island)
{
    if (island && walrus && walrus->getType() == WALRUS && walrus->getStatus() == Walrus::SAILING)
    {
        Walrus *w = (Walrus*)walrus;

        w->setStatus(Walrus::INSHORING);
        w->setIsland((BoxIsland*)island);
        char str[256];
        Message mg;
        sprintf(str, "Walrus %d has arrived to %s.", NUMBERING(w->getNumber()), island->getName().c_str());
        mg.msg = std::string(str);
        mg.faction = w->getFaction();
        messages.insert(messages.begin(), mg);
    }
    return true;
}

// SYNC
bool landed(Vehicle *manta, Island *island)
{
    if (manta && island && manta->getType() == MANTA)
    {
        if (manta->getStatus() == Manta::FLYING || manta->getStatus() == Manta::HOLDING)
        {
            if (entities[controller.controllingid] == manta)
                controller.reset();
            AdvancedManta *s = (AdvancedManta*)manta;
            struct controlregister c;
            c.thrust = 0.0f;
            c.pitch = 0.0f;
            s->setControlRegisters(c);
            s->setThrottle(0.0f);
            s->setStatus(Manta::LANDED);

            char str[256];
            Message mg;
            mg.faction = manta->getFaction();
            sprintf(str, "Manta %d has landed on Island %s.", NUMBERING(s->getNumber()), island->getName().c_str());
            mg.msg = std::string(str);
            messages.insert(messages.begin(), mg);
        }
    }
    return true;
}

// SYNC
bool isMineFire(Vehicle* vehicle, Gunshot *g)
{
    return (g->getOrigin() == vehicle->getBodyID());
}

// SYNC
bool rayHit(Vehicle *vehicle, LaserRay *l)
{
    vehicle->damage(2);
    return true;
}



// SYNC
bool hit(Vehicle *vehicle, Gunshot *g)
{
    /**   Sending too many messages hurts FPS
    if (vehicle && vehicle->getType() == CARRIER)
    {
        messages.insert(messages.begin(), std::string("Carrier is under fire!"));
    }
    if (vehicle && vehicle->getType() == MANTA)
    {
        messages.insert(messages.begin(), std::string("Manta is under fire!"));
    }
    if (vehicle && vehicle->getType() == WALRUS)
    {
        messages.insert(messages.begin(), std::string("Walrus is under fire!"));
    }
    **/

    // @NOTE: Gunshot are a source of inestability.  I added some extra checks and to avoid tweakings I hide bullets after hitting targets.
    g->setVisible(false);

    // Dont hit me
    if (g->getOrigin() != vehicle->getBodyID())
    {
        if (vehicle->getType() == MANTA)
        {
            vehicle->damage(g->getDamage()*25);
        } else
            vehicle->damage(g->getDamage());
        return false;
    }
    return true;
}

bool hit(Structure* structure, Gunshot *g)
{
    static Island *b = NULL;

    if (structure)
    {
        if (structure && structure->island && b != structure->island)
        {
            char str[256];
            Message mg;
            mg.faction = BOTH_FACTION;
            sprintf(str, "Island %s under attack!", structure->island->getName().c_str());
            mg.msg = std::string(str);
            messages.insert(messages.begin(), mg);
            b = structure->island;
        }

        //bullethit();
        g->setVisible(false);
        structure->damage(g->getDamage());
    }
}

// SYNC
bool releasecontrol(Vehicle* vehicle)
{
    if (vehicle && vehicle->getType() == MANTA)
    {
        if (vehicle->getStatus() != Manta::ON_DECK && vehicle->getStatus() != Manta::TACKINGOFF)
        {
            controller.reset();

            AdvancedManta *s = (AdvancedManta*)vehicle;
            struct controlregister c;
            c.thrust = 0.0f;
            c.pitch = 0.0f;
            s->setControlRegisters(c);
            s->setThrottle(0.0f);
            s->setStatus(Manta::ON_DECK);
            s->inert = true;

            Message mg;
            mg.faction = s->getFaction();
            char str[256];
            sprintf(str, "Manta %d has landed on Aircraft.", NUMBERING(s->getNumber()));
            mg.msg = std::string(str);
            messages.insert(messages.begin(), mg);
        }
    }
    return true;
}

void  releasecontrol(dGeomID geom)
{
    synchronized(entities.m_mutex)
    {
        Vehicle* vehicle = gVehicle(geom);
        releasecontrol(vehicle);
    }
}


// SYNC
bool  isType(Vehicle *vehicle, int type)
{
    bool result = false;
    if (vehicle)
    {
        if (vehicle->getType() == type)
        {
            result = true;
        }
    }
    return result;
}


bool  isType(dGeomID geom, int type)
{
    bool result = false;
    synchronized(entities.m_mutex)
    {
        Vehicle *vehicle = gVehicle(geom);
        result = isType(vehicle,type);
    }
    return result;
}


bool isType(dGeomID geom, int types[], int length)
{
    bool result = false;
    synchronized(entities.m_mutex)
    {
        Vehicle *vehicle = gVehicle(geom);

        for(int s=0;s<length;s++)
            if (isType(vehicle,types[s])) result=true;
    }
    return result;

}


bool  isManta(dGeomID geom)
{
    return isType(geom,MANTA);
}

bool  isManta(Vehicle* vehicle)
{
    return isType(vehicle,MANTA);
}

bool  isCarrier(dGeomID geom)
{
    return isType(geom, CARRIER);
}

bool  isCarrier(Vehicle* vehicle)
{
    return isType(vehicle,CARRIER);
}

bool  isWalrus(Vehicle* vehicle)
{
    return isType(vehicle, WALRUS);
}

bool  isAction(dGeomID body)
{
    return isType(body, ACTION) || isType(body, VehicleTypes::CONTROLABLEACTION);
}

bool  isAction(Vehicle* vehicle)
{
    return isType(vehicle, ACTION) || isType(vehicle, VehicleTypes::CONTROLABLEACTION);
}

// SYNC
bool isRay(dGeomID o)
{
    for(size_t i=entities.first();entities.hasMore(i);i=entities.next(i))
    {
        Vehicle *v=entities[i];
        if (v->getGeom() == o && v->getType()==RAY)
        {
            return true;
        }
    }
    return false;
}

bool  isRunway(Structure* s)
{
    if (s && s->getType()==LANDINGABLE)
    {
        return true;
    }
    return false;
}

bool  isRunway(dGeomID candidate)
{
    for(size_t i=entities.first();entities.hasMore(i);i=entities.next(i))
    {
        Vehicle *vehicle = entities[i];
        if (vehicle && vehicle->getGeom() == candidate && vehicle->getType()==LANDINGABLE)
        {
            return true;
        }
    }
}

Island* getIsland(dGeomID candidate)
{
    for (int j=0;j<islands.size();j++)
    {
        if (candidate == islands[j]->getGeom())
            return islands[j];
    }
    return NULL;
}

bool  isIsland(dGeomID candidate)
{
    for (int j=0;j<islands.size();j++)
    {
        if (candidate == islands[j]->getGeom())
        {
            return true;
        }
    }
    return false;
}


// @FIXME Check the island !
CommandCenter* findCommandCenter(Island *island)
{
    for(size_t i=entities.first();entities.hasMore(i);i=entities.next(i))
    {
        Vehicle *v=entities[i];
        if (v->getType() == CONTROL)
        {
            Structure *s = (Structure*)v;
            if (s->island == island)
                return (CommandCenter*)s;
        }
    }
    return NULL;
}

// SYNC
bool  groundcollisions(Vehicle *vehicle)
{
    if (vehicle)
    {
        if (vehicle->getSpeed()>10 and vehicle->getType() == MANTA)
        {
            //explosion();
            AdvancedManta *s = (AdvancedManta*)vehicle;
            struct controlregister c;
            c.thrust = 0.0f;
            c.pitch = 0.0f;
            s->setControlRegisters(c);
            s->setThrottle(0.0f);
            s->damage(1);
        }
    }
    return true;
}

void groundcollisions(dGeomID geom)
{
    synchronized(entities.m_mutex)
    {
        Vehicle *vehicle = gVehicle(geom);
        groundcollisions(vehicle);
    }
}

Manta* findMantaByNumber(size_t &pos, int number)
{
    for(size_t i=entities.first();entities.hasMore(i);i=entities.next(i))
    {
        Vehicle *v=entities[i];
        if (v->getType() == MANTA && ((Manta*)v)->getNumber() == number-1)
        {
            pos = i+1;  // @FIXME Risky
            pos = entities.indexOf(i);
            return (Manta*)v;
        }
    }
    return NULL;
}

Manta* findNearestManta(int status, int faction, Vec3f l, float threshold)
{
    int nearesti=-1;
    float closest = 0;

    for(size_t i=entities.first();entities.hasMore(i);i=entities.next(i))
    {
        Vehicle *v=entities[i];
        if (v->getType() == MANTA && v->getStatus() == status && v->getFaction() == faction)
        {
            if (((v->getPos()-l).magnitude()<closest || closest == 0) && ((v->getPos()-l).magnitude()<threshold) )  {
                closest = (v->getPos()-l).magnitude();
                nearesti = i;
            }

        }
    }
    if (nearesti<0)
        return NULL;
    else
        return (Manta*)entities[nearesti];

}


Manta* findMantaByOrder(int faction, int order)
{
    for(size_t i=entities.first();entities.hasMore(i);i=entities.next(i))
    {
        Vehicle *v=entities[i];
        if (v->getType() == MANTA && v->getOrder() == order && v->getFaction() == faction)
        {
            return (Manta*)v;
        }
    }
    return NULL;
}

Manta* findManta(int faction, int status, Vec3f around)
{
    for(size_t i=entities.first();entities.hasMore(i);i=entities.next(i))
    {
        Vehicle *v=entities[i];
        if (v->getType() == MANTA && v->getStatus() == status && v->getFaction() == faction && (around-v->getPos()).magnitude() < CLOSE_RANGE)
        {
            return (Manta*)v;
        }
    }
    return NULL;
}

Manta* findManta(int faction, int status)
{
    for(size_t i=entities.first();entities.hasMore(i);i=entities.next(i))
    {
        Vehicle *v=entities[i];
        if (v->getType() == MANTA && v->getStatus() == status && v->getFaction() == faction)
        {
            return (Manta*)v;
        }
    }
    return NULL;
}

Manta* findManta(int faction)
{
    for(size_t i=entities.first();entities.hasMore(i);i=entities.next(i))
    {
        Vehicle *v=entities[i];
        if (v->getType() == MANTA && v->getFaction() == faction)
        {
            return (Manta*)v;
        }
    }
    return NULL;
}

Walrus* findNearestWalrus(int faction, Vec3f l, float threshold = 100000 kmf)
{
    int nearesti=-1;
    float closest = 0;

    for(size_t i=entities.first();entities.hasMore(i);i=entities.next(i))
    {
        Vehicle *v=entities[i];
        if (v->getType() == WALRUS && v->getFaction() == faction)
        {
            if (((v->getPos()-l).magnitude()<closest || closest == 0) && ((v->getPos()-l).magnitude()<threshold) )  {
                closest = (v->getPos()-l).magnitude();
                nearesti = i;
            }

        }
    }
    if (nearesti<0)
        return NULL;
    else
        return (Walrus*)entities[nearesti];

}

Walrus* findWalrusByNumber(size_t &pos, int number)
{
    for(size_t i=entities.first();entities.hasMore(i);i=entities.next(i))
    {
        Vehicle *v=entities[i];
        if (v->getType() == WALRUS  && ((Walrus*)v)->getNumber() == number - 1)
        {
            pos = i+1;   // @FIXME Fix this risky numbering system.
            pos = entities.indexOf(i);
            return (Walrus*)v;
        }
    }
    return NULL;
}


Walrus* findWalrusByOrder(int faction, int order)
{
    int ordr = 0;
    for(size_t i=entities.first();entities.hasMore(i);i=entities.next(i))
    {
        Vehicle *v=entities[i];
        if (v->getType() == WALRUS && v->getFaction() == faction)
        {
            if ((++ordr)==order)
                return (Walrus*)v;
        }
    }
    return NULL;
}

Walrus* findWalrus(int faction)
{
    for(size_t i=entities.first();entities.hasMore(i);i=entities.next(i))
    {
        Vehicle *v=entities[i];
        if (v->getType() == WALRUS && v->getFaction() == faction)
        {
            return (Walrus*)v;
        }
    }
    return NULL;
}

Walrus* findWalrus(int status, int faction, int order)
{
    int ordr = 0;
    for(size_t i=entities.first();entities.hasMore(i);i=entities.next(i))
    {
        Vehicle *v=entities[i];
        if (v->getType() == WALRUS && v->getFaction() == faction && v->getStatus() == status)
        {
            if ((++ordr)==order)
                return (Walrus*)v;
        }
    }
    return NULL;
}

Walrus* findWalrus(int status, int faction)
{
    return findWalrus(status,faction,1);
}

Vehicle* findCarrier(int faction)
{
    for(size_t i=entities.first();entities.hasMore(i);i=entities.next(i))
    {
        Vehicle *v=entities[i];
        if (v->getType() == CARRIER && v->getFaction() == faction)
        {
            return v;
        }
    }
    return NULL;
}

int findNextNumber(int type)
{
    int numbers[256];
    memset(numbers,0,sizeof(int)*256);

    for(size_t i=entities.first();entities.hasMore(i);i=entities.next(i))
    {
        Vehicle *v=entities[i];
        if (type == MANTA && v->getType() == type)
        {
            Manta *m = (Manta*)v;
            numbers[m->getNumber()] = 1;
        }
        if (type == WALRUS && v->getType() == type)
        {
            Walrus *m = (Walrus*)v;
            numbers[m->getNumber()] = 1;
        }
    }
    for(int i=0;i<256;i++)
    {
        if (numbers[i]==0)
            return i;
    }
    assert(!"No more available numbers !!!!!");
}

Vehicle* findNearestEnemyVehicle(int friendlyfaction,int type, Vec3f l, float threshold)
{
    int nearesti = -1;
    float closest = threshold;
    for(size_t i=entities.first();entities.hasMore(i);i=entities.next(i))
    {
        Vehicle *v=entities[i];
        if (v &&
                ( (type == -1 && v->getType() != ACTION && v->getType() != CONTROLABLEACTION && v->getType() != RAY) || (v->getType() == type) )
                && v->getFaction()!=friendlyfaction)   // Fix this.
        {
            if ((v->getPos()-l).magnitude()<closest) {
                closest = (v->getPos()-l).magnitude();
                nearesti = i;
            }
        }
    }
    if (nearesti < 0)
        return NULL;
    else
        return entities[nearesti];
}
Vehicle* findNearestEnemyVehicle(int friendlyfaction,Vec3f l, float threshold)
{
    return findNearestEnemyVehicle(friendlyfaction,-1,l,threshold);
}

BoxIsland* findNearestIsland(Vec3f Po)
{
    int nearesti = 0;
    float closest = 0;
    for(int i=0;i<islands.size();i++)
    {
        BoxIsland *b = islands[i];
        Vec3f l(b->getX(),0.0f,b->getZ());

        if ((l-Po).magnitude()<closest || closest ==0) {
            closest = (l-Po).magnitude();
            nearesti = i;
        }
    }

    return islands[nearesti];
}


BoxIsland* findNearestEmptyIsland(Vec3f Po)
{
    int nearesti = -1;
    float closest = 0;
    for(int i=0;i<islands.size();i++)
    {
        BoxIsland *b = islands[i];
        Vec3f l(b->getX(),0.0f,b->getZ());

        Structure *d = b->getCommandCenter();

        if (!d )
        {
            if ( ((l-Po).magnitude()<closest || closest ==0) ) {
                closest = (l-Po).magnitude();
                nearesti = i;
            }
        }
    }

    if (nearesti<0)
        return NULL;

    return islands[nearesti];
}


Antenna* findAntennaFromIsland(BoxIsland *is)
{
    std::vector<size_t> strs = is->getStructures();

    for(size_t i=0;i<strs.size();i++)
    {
        Structure *s = (Structure*)entities[strs[i]];
        if (s->getSubType()==VehicleSubTypes::ANTENNA)
        {
            return (Antenna*)s;
        }
    }

    return NULL;
}

BoxIsland* findNearestFriendlyIsland(Vec3f Po, bool empty, int friendlyfaction, float threshold)
{
    int nearesti = -1;
    float closest = 0;
    for(int i=0;i<islands.size();i++)
    {
        BoxIsland *b = islands[i];
        Vec3f l(b->getX(),0.0f,b->getZ());

        Structure *d = b->getCommandCenter();

        if ((!d && empty) || (d && !empty && (d->getFaction()==friendlyfaction || friendlyfaction == -1)) )
        {
            if ( ((l-Po).magnitude()<closest || closest ==0)  && (l-Po).magnitude()<threshold) {
                closest = (l-Po).magnitude();
                nearesti = i;
            }
        }
    }

    if (nearesti<0)
        return NULL;

    return islands[nearesti];
}

BoxIsland* findNearestEnemyIsland(Vec3f Po, bool empty, int friendlyfaction, float threshold)
{
    int nearesti = -1;
    float closest = 0;
    for(size_t i=0;i<islands.size();i++)
    {
        BoxIsland *b = islands[i];
        Vec3f l(b->getX(),0.0f,b->getZ());

        Structure *d = b->getCommandCenter();

        if ((!d && empty) || (d && !empty && (d->getFaction()!=friendlyfaction || friendlyfaction == -1)) )
        {
            if ( ((l-Po).magnitude()<closest || closest ==0)  && (l-Po).magnitude()<threshold) {
                closest = (l-Po).magnitude();
                nearesti = i;
            }
        }
    }

    if (nearesti<0)
        return NULL;

    return islands[nearesti];
}

BoxIsland* findNearestEnemyIsland(Vec3f Po, bool empty, int friendlyfaction)
{
    return findNearestEnemyIsland(Po,empty,friendlyfaction,12000 kmf);
}


BoxIsland* findNearestEnemyIsland(Vec3f Po, bool empty)
{
    return findNearestEnemyIsland(Po,empty,-1);
}

BoxIsland* findIslandByName(std::string islandname)
{
    for (size_t j=0;j<islands.size();j++)
    {
        if (islandname == islands[j]->getName())
        {
            return islands[j];
        }
    }
    return NULL;
}


void list()
{
    for(size_t i=entities.first();entities.hasMore(i);i=entities.next(i))
    {
        CLog::Write(CLog::Debug,"[%d]: Body ID (%16p) Position (%d) Type: %d\n", i,(void*)entities[i]->getBodyID(), entities.indexOf(i), entities[i]->getType());
    }
}


/**
 * @brief When a vehicle is COMM_RANGE away from the carrier or from a friendly island, it will loose the connection link.
 * This imply that it will start getting damage points all the way up to destruction.
 * If there is no carrier, only the nearest friendly island can work for connection.
 * If a vehicle has a long range communication antenna his getSignal is 4 and is not affected.
 *
 * @TODO: This is the future purpose of the communication antenna...> Done !
 *
 * @param faction
 * @param space
 * @param world
 */
void commLink(int faction, dSpaceID space, dWorldID world)
{
    Vehicle *carrier = findCarrier(faction);

    Vec3f b(0,600 kmf, 0);

    // @NOTE We should decide here what to do.
    if (carrier)
        b = carrier->getPos();

    for(size_t i=entities.first();entities.hasMore(i);i=entities.next(i))
    {
        // Damage vehicles that are far away from the command link center.
        // Should check if the vehicle has some device to allow further control.
        if ((entities[i]->getType() == WALRUS || entities[i]->getType() == MANTA ) && (entities[i]->getFaction() == faction) )
        {
            if (entities[i]->getSignal()!=4)
            {
                if ((entities[i]->getPos() - b).magnitude() > COMM_RANGE)
                {
                    // Check if there is a nearby command center. @FIXME This should be a communication link instead.
                    BoxIsland *is = findNearestFriendlyIsland(entities[i]->getPos(),false,entities[i]->getFaction(),COMM_RANGE);

                    Antenna *a = NULL;

                    if (is)
                    {
                        a = findAntennaFromIsland(is);
                    }

                    if (!is || (is && !a))
                    {
                        if (entities[i]->getSignal()==3)
                        {
                            char msg[256];
                            Message mg;
                            sprintf(msg, "Vehicle is loosing connection.");
                            mg.faction = entities[i]->getFaction();
                            mg.msg = std::string(msg);
                            messages.insert(messages.begin(), mg);
                        }
                        entities[i]->setSignal(2);
                        entities[i]->damage(1);
                    }

                }
                else
                {
                    entities[i]->setSignal(3);                  // Put connection back to normal.
                }
            }
        }

    }
}



void defendIsland(unsigned long timer, dSpaceID space, dWorldID world)
{
    for (size_t j = 0; j < islands.size(); j++)
    {
        BoxIsland *island = islands[j];

        // First check whether the radar detect that there is some enemy vehicle close to the island center.

        Vehicle *target = NULL;

        if (CommandCenter *sc = (CommandCenter*)island->getCommandCenter())
        {
            target = findNearestEnemyVehicle(sc->getFaction(),island->getPos(), 3 * 3.6 kmf);

            //CLog::Write(CLog::Debug,"Found target %p\n",  target);

            Vehicle *b = target;

            if (!b)
            {
                if (sc->isUnderAttack())
                {
                    sc->setTimer(timer);
                    sc->setUnderAttack(false);
                }

                // Find flying aircrafts and land them.
                Manta *m = findMantaByOrder(sc->getFaction(), DEFEND_ISLAND);

                if (m)
                {
                    if (timer==(sc->getTimer() + 10))
                    {
                        m->setDestination(island->getPos());
                    }

                    if (timer==(sc->getTimer() + 1000))
                    {
                        std::vector<size_t> str = island->getStructures();

                        for(size_t i=0;i<str.size();i++)
                        {
                            if (entities[str[i]]->getFaction()==sc->getFaction())
                            {
                                // RTTI Stuff
                                if(Runway* lb = dynamic_cast<Runway*>(entities[str[i]]))
                                {
                                    landManta(lb, m) ;
                                }
                            }
                        }
                    }
                }


                continue;
            }
            if (!sc->isUnderAttack())
            {
                sc->setTimer(timer);
                sc->setUnderAttack(true);
            }

            // If here, this island is under attack.

            {
                if (timer>(sc->getTimer() + 500))                // Update position of the enemy.
                {

                    Manta *m = findMantaByOrder(sc->getFaction(), DEFEND_ISLAND);

                    if (m && m->getStatus() != Manta::LANDED)
                    {

                        if (!m->isAuto())
                        {
                            m->dogfight(b->getPos());
                            m->enableAuto();
                        }

                        m->dogfight(b->getPos());
                    }
                }
            }


            std::vector<size_t> str = island->getStructures();

            for(size_t i=0;i<str.size();i++)
            {
                if (entities[str[i]]->getFaction()==sc->getFaction())
                {
                    // RTTI Stuff
                    if(Artillery* lb = dynamic_cast<Artillery*>(entities[str[i]]))
                    {
                        if (b->getPos()[1]<60)   // 60 is island height.  Only aim to ground based units
                        {
                            // Find the vector between them, and the parameters for the turret to hit the vehicle, regardless of its random position.
                            Vec3f firingloc = lb->getFiringPort();

                            lb->elevation = -5;
                            lb->azimuth = getAzimuth((b->getPos())-(firingloc));

                            struct controlregister c;
                            c.pitch = 0.0;
                            c.roll = 0.0;
                            lb->setControlRegisters(c);
                            lb->setForward(toVectorInFixedSystem(0,0,1,lb->azimuth, -lb->elevation));

                            dout << lb <<  ":Azimuth: " << lb->azimuth << " Inclination: " << lb->elevation << std::endl;

                            Vehicle *action = (lb)->fire(world,space);

                            if (action != NULL)
                            {
                                entities.push_back(action,action->getGeom());
                                //gunshot();
                            }
                        }
                    } else
                    if(LaserTurret* lb = dynamic_cast<LaserTurret*>(entities[str[i]]))
                    {
                        // Find the vector between them, and the parameters for the turret to hit the vehicle, regardless of its random position.
                        Vec3f firingloc = lb->getFiringPort();

                        lb->elevation = getDeclination((b->getPos())-(firingloc));
                        lb->azimuth = getAzimuth((b->getPos())-(firingloc));
                        lb->setForward((b->getPos())-(firingloc));

                        struct controlregister c;
                        c.pitch = 0.0;
                        c.roll = 0.0;
                        lb->setControlRegisters(c);

                        dout << lb <<  ":Azimuth: " << lb->azimuth << " Inclination: " << lb->elevation << std::endl;

                        lb->setForward((b->getPos())-(firingloc));

                        Vehicle *action = (lb)->fire(world,space);

                        if (action != NULL)
                        {
                            entities.push_back(action,action->getGeom());
                            //gunshot();
                        }
                    } else
                    if(Turret* lb = dynamic_cast<Turret*>(entities[str[i]]))
                    {
                        // Find the vector between them, and the parameters for the turret to hit the vehicle, regardless of its random position.
                        Vec3f firingloc = lb->getFiringPort();

                        lb->elevation = getDeclination((b->getPos())-(firingloc));
                        lb->azimuth = getAzimuth((b->getPos())-(firingloc));
                        lb->setForward((b->getPos())-(firingloc));

                        struct controlregister c;
                        c.pitch = 0.0;
                        c.roll = 0.0;
                        lb->setControlRegisters(c);

                        dout << lb <<  ":Azimuth: " << lb->azimuth << " Inclination: " << lb->elevation << std::endl;

                        lb->setForward((b->getPos())-(firingloc));

                        Vehicle *action = (lb)->fire(world,space);

                        if (action != NULL)
                        {
                            entities.push_back(action,action->getGeom());
                            //gunshot();
                        }
                    } else
                    if(Launcher* lb = dynamic_cast<Launcher*>(entities[str[i]]))
                    {
                        // Increase the range a little bit.
                        Vehicle *target = findNearestEnemyVehicle(BLUE_FACTION,island->getPos(), 9 * 3.6 kmf);

                        if (!target)
                            return;

                        Vehicle *b = target;

                        Vec3f firingloc = lb->getPos();

                        dout << lb <<  ":Loc: " << firingloc << " Target: " << b->getPos() << std::endl;

                        lb->elevation = -5; // A little bit up.
                        lb->azimuth = getAzimuth((b->getPos())-(firingloc));

                        struct controlregister c;
                        c.pitch = 0.0;
                        c.roll = 0.0;
                        //lb->setControlRegisters(c);
                        lb->setForward(toVectorInFixedSystem(0,0,1,lb->azimuth, -lb->elevation));

                        dout << lb <<  ":Azimuth: " << lb->azimuth << " Inclination: " << lb->elevation << std::endl;

                        if (target->getType() == MANTA || target->getType() == WALRUS)
                        {
                            lb->air();
                        } else {
                            lb->ground();
                        }
                        Gunshot* action = (Gunshot*)(lb)->fire(world,space);

                        // @FIXME: AAM missiles guiding sucks.

                        if (action != NULL)
                        {
                            size_t l = entities.push_back(action,action->getGeom());
                            gunshot();

                            action->setDestination(target->getPos());
                            action->enableAuto();

                        }
                    } else
                    if(Runway* lb = dynamic_cast<Runway*>(entities[str[i]]))
                    {
                        // Launch airplanes to attack incoming mantas.
                        unsigned long timeevent = sc->getTimer();

                        static bool found = false;

                        if (timer==(timeevent + 200))
                        {
                            Manta *m = findMantaByOrder(lb->getFaction(), DEFEND_ISLAND);

                            Manta *ml = findManta(lb->getFaction(), Manta::LANDED, lb->getPos());

                            if (!m && !ml)
                            {

                                int mantNumber = findNextNumber(MANTA);
                                Vehicle *manta = (lb)->spawn(world,space,MANTA,mantNumber);

                                size_t l = entities.push_back(manta, manta->getGeom());

                                manta->setOrder(DEFEND_ISLAND);
                            }


                            // @FIXME: What should I do with mantas after the battle is finished?
                            // @FIXME: What happens when the runway is constructed after the attack has started (timeevent is old).

                        }

                        if (timer==(timeevent + 300))
                        {
                            Manta *m = launchManta(lb);
                            printf ("Medusa: %p\n", m);
                        }

                    }
                }
            }
        }
    }
}

void buildAndRepair(dSpaceID space, dWorldID world)
{
    for (size_t i = 0; i < islands.size(); i++)
    {
        BoxIsland *island = islands[i];

        if (island->getStructures().size()<14)
        {
            CommandCenter *c = findCommandCenter(island);
            if (c)
            {
                if (c->getTtl()<=0)
                {
                    // Structures can be rotated.  This is important for runways.
                    float prob = ((int)(rand() % 100 + 1))/100.0f;
                    Structure *s;

                    struct templatestructure
                    {
                        int subType=VehicleSubTypes::STRUCTURE;
                        float chance=0;
                        bool onlyonce=false;
                    };

                    std::vector<struct templatestructure> islandstructs;

                    if (c->getIslandType() == ISLANDTYPES::DEFENSE_ISLAND)
                    {
                        // Softmax, boltzman decay.
                        {struct templatestructure tp;tp.subType = VehicleSubTypes::LASERTURRET;tp.chance = 0.9;islandstructs.push_back(tp);}
                        {struct templatestructure tp;tp.subType = VehicleSubTypes::TURRET;tp.chance = 0.9;islandstructs.push_back(tp);}
                        {struct templatestructure tp;tp.subType = VehicleSubTypes::RUNWAY;tp.chance = 0.9;tp.onlyonce=true;islandstructs.push_back(tp);}
                        {struct templatestructure tp;tp.subType = VehicleSubTypes::ANTENNA;tp.chance = 0.85;tp.onlyonce=true;islandstructs.push_back(tp);}
                        {struct templatestructure tp;tp.subType = VehicleSubTypes::ARTILLERY;tp.chance = 0.4;tp.onlyonce=true;islandstructs.push_back(tp);}
                        {struct templatestructure tp;tp.subType = VehicleSubTypes::RADAR;tp.chance = 0.6;tp.onlyonce=true;islandstructs.push_back(tp);}
                        {struct templatestructure tp;tp.subType = VehicleSubTypes::STRUCTURE;tp.chance = 0.01;islandstructs.push_back(tp);}
                    } else if (c->getIslandType() == ISLANDTYPES::FACTORY_ISLAND){

                        // Factory island
                        {struct templatestructure tp;tp.subType = VehicleSubTypes::FACTORY;tp.chance = 0.2;islandstructs.push_back(tp);}
                        {struct templatestructure tp;tp.subType = VehicleSubTypes::WAREHOUSE;tp.chance = 0.7;islandstructs.push_back(tp);}
                        {struct templatestructure tp;tp.subType = VehicleSubTypes::DOCK;tp.chance = 0.7;tp.onlyonce=true;islandstructs.push_back(tp);}
                        {struct templatestructure tp;tp.subType = VehicleSubTypes::RUNWAY;tp.chance = 0.1;tp.onlyonce=true;islandstructs.push_back(tp);}
                        {struct templatestructure tp;tp.subType = VehicleSubTypes::ANTENNA;tp.chance = 0.45;tp.onlyonce=true;islandstructs.push_back(tp);}
                        {struct templatestructure tp;tp.subType = VehicleSubTypes::ARTILLERY;tp.chance = 0.02;tp.onlyonce=true;islandstructs.push_back(tp);}
                        {struct templatestructure tp;tp.subType = VehicleSubTypes::RADAR;tp.chance = 0.02;tp.onlyonce=true;islandstructs.push_back(tp);}
                        {struct templatestructure tp;tp.subType = VehicleSubTypes::STRUCTURE;tp.chance = 0.01;islandstructs.push_back(tp);}
                        {struct templatestructure tp;tp.subType = VehicleSubTypes::TURRET;tp.chance = 0.02;islandstructs.push_back(tp);}
                    } else {
                        // Logistics island
                        {struct templatestructure tp;tp.subType = VehicleSubTypes::WAREHOUSE;tp.chance = 0.7;islandstructs.push_back(tp);}
                        {struct templatestructure tp;tp.subType = VehicleSubTypes::DOCK;tp.chance = 0.7;islandstructs.push_back(tp);}
                        {struct templatestructure tp;tp.subType = VehicleSubTypes::RUNWAY;tp.chance = 0.9;tp.onlyonce=true;islandstructs.push_back(tp);}
                        {struct templatestructure tp;tp.subType = VehicleSubTypes::ANTENNA;tp.chance = 0.9;tp.onlyonce=true;islandstructs.push_back(tp);}
                        {struct templatestructure tp;tp.subType = VehicleSubTypes::ARTILLERY;tp.chance = 0.2;tp.onlyonce=true;islandstructs.push_back(tp);}
                        {struct templatestructure tp;tp.subType = VehicleSubTypes::RADAR;tp.chance = 0.2;tp.onlyonce=true;islandstructs.push_back(tp);}
                        {struct templatestructure tp;tp.subType = VehicleSubTypes::STRUCTURE;tp.chance = 0.01;islandstructs.push_back(tp);}
                        {struct templatestructure tp;tp.subType = VehicleSubTypes::TURRET;tp.chance = 0.25;islandstructs.push_back(tp);}

                    }

                    int which = (rand() % islandstructs.size());

                    CLog::Write(CLog::Debug,"Which %d prob %10.5f vs chances %10.5f  \n", which, prob, islandstructs[which].chance );
                    if (prob<islandstructs[which].chance)
                    {


                        bool ispresent = false;
                        std::vector<size_t> strs = island->getStructures();
                        for(size_t i=0;i<strs.size();i++)
                        {
                            if (entities[strs[i]]->getSubType() == islandstructs[which].subType)
                                ispresent = true;
                        }

                        if (!islandstructs[which].onlyonce || (islandstructs[which].onlyonce && !ispresent))
                        {

                            Structure *s = NULL;
                            switch (islandstructs[which].subType) {
                            case VehicleSubTypes::HANGAR:
                                    s = new Hangar(c->getFaction());
                                    break;
                            case VehicleSubTypes::WAREHOUSE:
                                s = new Warehouse(c->getFaction());
                                break;
                            case VehicleSubTypes::RUNWAY:
                                s = new Runway(c->getFaction());
                                break;
                            case VehicleSubTypes::LASERTURRET:
                                s = new LaserTurret(c->getFaction());
                                break;
                            case VehicleSubTypes::TURRET:
                                s = new Turret(c->getFaction());
                                break;
                            case VehicleSubTypes::LAUNCHER:
                                s = new Launcher(c->getFaction());
                                break;
                            case VehicleSubTypes::FACTORY:
                                s = new Factory(c->getFaction());
                                break;
                            case VehicleSubTypes::DOCK:
                                s = new Dock(c->getFaction());
                                break;
                            case VehicleSubTypes::ANTENNA:
                                s = new Antenna(c->getFaction());
                                break;
                            case VehicleSubTypes::RADAR:
                                s = new Radar(c->getFaction());
                                break;
                            case VehicleSubTypes::STRUCTURE:default:
                                s = new Structure(c->getFaction());
                                break;

                            }
                            island->addStructure(s,world);
                        }
                    }


                    //int x = (rand() % 2000 + 1); x -= 1000;
                    //int z = (rand() % 2000 + 1); z -= 1000;



                    //island->addStructure(s,x,z,space,world);

                    //entities.push_back(s);

                    if (8<=which && which<=10)
                    {
                        //Structure *s2 = new Hangar(c->getFaction());
                        //z-=550;
                        //island->addStructure(s2,s->getPos()[0]-island->getPos()[0],s->getPos()[2]-island->getPos()[2]-550,space,world);

                        //entities.push_back(s2);
                    }

                    c->restart();
                }

            }
        }

    }
}

Manta* spawnManta(dSpaceID space, dWorldID world,Vehicle *spawner, size_t &idx)
{
    Manta* m = findManta(spawner->getFaction(),Manta::ON_DECK, spawner->getPos());

    if (m)
    {
        char msg[256];
        Message mg;
        sprintf(msg, "%s","There is another aircraft on deck.");
        mg.faction = spawner->getFaction();
        mg.msg = std::string(msg);
        messages.insert(messages.begin(), mg);
        return NULL;
    }


    int mantaNumber = findNextNumber(MANTA);
    Vehicle *manta = (spawner)->spawn(world,space,MANTA,mantaNumber);
    if (manta != NULL)
    {
        idx = entities.push_back(manta, manta->getGeom());
        char msg[256];
        Message mg;
        mg.faction = manta->getFaction();
        sprintf(msg, "Manta %2d is ready to takeoff.",NUMBERING(mantaNumber));
        mg.msg = std::string(msg);
        messages.insert(messages.begin(), mg);
    }
    return (Manta*)manta;
}

Walrus* spawnWalrus(dSpaceID space, dWorldID world, Vehicle *spawner)
{
    int walrusNumber = findNextNumber(WALRUS);
    Vehicle *walrus = (spawner)->spawn(world,space,WALRUS,walrusNumber);
    if (walrus != NULL)
    {
        entities.push_back(walrus,walrus->getGeom());
        char msg[256];
        Message mg;
        mg.faction = walrus->getFaction();
        sprintf(msg, "Walrus %2d has been deployed.",NUMBERING(walrusNumber));
        mg.msg = std::string(msg);
        messages.insert(messages.begin(), mg);
    }
    return (Walrus*)walrus;
}

// SYNCJ
void dockWalrus(Vehicle *dock)
{

    for(size_t i=entities.first();entities.hasMore(i);i=entities.next(i))
    {
        // @FIXME: Only put back the walrus that is close to the carrier.
        //CLog::Write(CLog::Debug,"Type and ttl: %d, %d\n", vehicles[i]->getType(),vehicles[i]->getTtl());
        if (entities[i]->getType()==WALRUS && entities[i]->getStatus()==Walrus::SAILING &&
                entities[i]->getFaction()==dock->getFaction() && (dock->getPos()-entities[i]->getPos()).magnitude()<DOCK_RANGE)
        {
            char msg[256];
            Message mg;
            mg.faction = entities[i]->getFaction();
            int numb = ((Walrus*)entities[i])->getNumber();
            sprintf(msg, "Walrus %d is now back on deck.",NUMBERING(numb));
            mg.msg = std::string(msg);
            messages.insert(messages.begin(), mg);

            dBodyDisable(entities[i]->getBodyID());
            entities.erase(entities[i]->getGeom());
        }
    }
}

// SYNC
void dockManta()
{
    for(size_t i=entities.first();entities.hasMore(i);i=entities.next(i))
    {
        //CLog::Write(CLog::Debug,"Type and ttl: %d, %d\n", vehicles[i]->getType(),vehicles[i]->getTtl());
        if (entities[i]->getType()==MANTA && entities[i]->getStatus()==Manta::ON_DECK)
        {
            char str[256];
            Message mg;
            mg.faction = entities[i]->getFaction();
            sprintf(str, "Manta %2d is now on bay.",NUMBERING(((Manta*)entities[i])->getNumber()));
            mg.msg = std::string(str);

            messages.insert(messages.begin(), mg);
            //CLog::Write(CLog::Debug,"Eliminating....\n");
            dBodyDisable(entities[i]->getBodyID());
            entities.erase(entities[i]->getGeom());
        }
    }
}


void landManta(Vehicle *landplace)
{
    // Auto control
    Manta *m = findManta(landplace->getFaction(),Manta::HOLDING);

    landManta(landplace, m);
}

void landManta(Vehicle *landplace, Manta *m)
{
    if (landplace->getType() == CARRIER || landplace->getType() == LANDINGABLE)
    {
        if (m)
        {
            m->setDestination(landplace->getPos());             // @FIXME: This needs to be performed all the time while manta is landing.
            m->setAttitude(landplace->getForward());
            m->land();
            m->enableAuto();
        }
    }
}

Manta* taxiManta(Vehicle *v)
{
    Manta* m = NULL;
    if (v->getType()==CARRIER)
    {
        Balaenidae *r = (Balaenidae*)v;
        m = findManta(r->getFaction(),Manta::ON_DECK, v->getPos());
        if (m)
        {
            r->taxi(m);
            char msg[256];
            Message mg;
            mg.faction = m->getFaction();
            sprintf(msg,"Manta %2d is ready for launch.",NUMBERING(m->getNumber()));
            mg.msg = std::string(msg);
            messages.insert(messages.begin(), mg);
        }
    } else if (v->getType()==LANDINGABLE )
    {
        Runway *r = (Runway*)v;
        m = findManta(r->getFaction(),Manta::LANDED, v->getPos());
        if (m)
        {
            r->taxi(m);

        }
    }
    return m;
}

Manta* launchManta(Vehicle *v)
{
    if (v->getType() == CARRIER)
    {
        Balaenidae *b = (Balaenidae*)v;
        Manta *m = findManta(v->getFaction(),Manta::ON_DECK);
        if (m)
        {
            b->launch(m);
            char msg[256];
            Message mg;
            mg.faction = b->getFaction();
            sprintf(msg, "Manta %2d has been launched.", NUMBERING(m->getNumber()));
            mg.msg = std::string(msg);
            messages.insert(messages.begin(), mg);
            takeoff();
        }
        return m;
    } else if (v->getType() == LANDINGABLE)
    {
        Runway *r = (Runway*)v;

        // Need to find the manta that is actually in this island.
        Manta *m = findManta(v->getFaction(), Manta::LANDED, r->getPos());
        if (m)
        {
            r->launch(m);
            BoxIsland *is = findNearestEnemyIsland(r->getPos(),false, -1);

            assert( is != NULL || !"A Runway structure should be on an island.");

            char msg[256];
            Message mg;
            mg.faction = r->getFaction();
            sprintf(msg, "Medusa %2d is departing from %s.", NUMBERING(m->getNumber()), is->getName().c_str());
            mg.msg = std::string(msg);
            messages.insert(messages.begin(), mg);
            takeoff();
        }
        return m;
    }

    assert(!"This should not happen.  A manta should be launched either from a Runway or a Carrier.");

    return NULL;
}

void wipeEnemyStructures(BoxIsland *island, int faction)
{
    std::vector<size_t> strs = island->getStructures();

    for(size_t i=0;i<strs.size();i++)
    {
        if (faction != entities[strs[i]]->getFaction())
        {
            entities[strs[i]]->damage(10000);
        }
    }
}

void captureIsland(BoxIsland *island, int faction, int typeofisland, dSpaceID space, dWorldID world)
{
    captureIsland(NULL,island,faction,typeofisland,space,world);
}

void captureIsland(Vehicle *b, BoxIsland *island, int faction, int typeofisland, dSpaceID space, dWorldID world)
{
    Structure *s = NULL;
    if (b)
    {
        Vec3f vector = b->getForward();
        vector = vector.normalize();
        Vec3f p = b->getPos()+Vec3f(70 * vector);       // Length of the command center and a little bit more.

        // @FIXME: Check if size do not goes beyond island boundaries.
        if ((p[0]-island->getX())>1800 || (p[0]-island->getX())<-1800 || (p[2]-island->getZ())>1800 || (p[2]-island->getZ())<-1800 )
        {
            CLog::Write(CLog::Debug,"Not enough room for command center.");
            return;  // Cancel the commandcenter creation, there is no room.
        }

        s = island->addStructure(new CommandCenter(faction,typeofisland),p[0]-island->getX(),p[2]-island->getZ(),0,world);
    }
    else
    {
        // Just build the command center anywhere.
        s = island->addStructure(new CommandCenter(faction,typeofisland),world);
    }

    if (s)
    {
        char msg[256];
        Message mg;
        mg.faction = BOTH_FACTION;
        sprintf(msg, "Island %s is now under control of %s.", island->getName().c_str(),FACTION(faction));
        mg.msg = std::string(msg);
        messages.insert(messages.begin(),  mg);

        wipeEnemyStructures(island,faction);
    }
}

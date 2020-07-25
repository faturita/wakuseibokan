#include "engine.h"

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
        mg.faction = w->getFaction();
        sprintf(str, "Walrus has arrived to %s.", island->getName().c_str());
        messages.insert(messages.begin(), mg);
    }
    return true;
}

// SYNC
bool landed(Vehicle *manta, Island *island)
{
    if (manta && island && manta->getType() == MANTA)
    {
        if (manta->getStatus() == Manta::FLYING)
        {
            char str[256];
            Message mg;
            mg.faction = manta->getFaction();
            sprintf(str, "Manta has landed on Island %s.", island->getName().c_str());
            mg.msg = std::string(str);
            messages.insert(messages.begin(), mg);

            controller.reset();
            SimplifiedDynamicManta *s = (SimplifiedDynamicManta*)manta;
            struct controlregister c;
            c.thrust = 0.0f;
            c.pitch = 0.0f;
            s->setControlRegisters(c);
            s->setThrottle(0.0f);
            s->setStatus(Manta::LANDED);
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
        if (b && structure && structure->island && b != structure->island)
        {
            char str[256];
            Message mg;
            mg.faction = structure->getFaction();
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

            SimplifiedDynamicManta *s = (SimplifiedDynamicManta*)vehicle;
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
            SimplifiedDynamicManta *s = (SimplifiedDynamicManta*)vehicle;
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

BoxIsland* findNearestIsland(Vec3f Po, bool empty, int friendlyfaction)
{
    return findNearestIsland(Po,empty,friendlyfaction,12000 kmf);
}

BoxIsland* findNearestIsland(Vec3f Po, bool empty, int friendlyfaction, float threshold)
{
    int nearesti = -1;
    float closest = 0;
    for(int i=0;i<islands.size();i++)
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

BoxIsland* findIslandByName(std::string islandname)
{
    for (int j=0;j<islands.size();j++)
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
        printf("[%d]: Body ID (%16p) Position (%d) Type: %d\n", i,(void*)entities[i]->getBodyID(), entities.indexOf(i), entities[i]->getType());
    }
}


void commLink(int faction, dSpaceID space, dWorldID world)
{
    Vehicle *b = findCarrier(faction);

    // @NOTE We should decide here what to do.
    if (!b) return;

    for(size_t i=entities.first();entities.hasMore(i);i=entities.next(i))
    {
        // Damage vehicles that are far away from the command link center.
        // Should check if the vehicle has some device to allow further control.
        if ((entities[i]->getType() == WALRUS || entities[i]->getType() == MANTA ) && (entities[i]->getFaction() == faction) )
        {
            if (entities[i]->getSignal()!=4)
            {
                if ((entities[i]->getPos() - b->getPos()).magnitude() > COMM_RANGE)
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
    for (int i = 0; i < islands.size(); i++)
    {
        BoxIsland *island = islands[i];

        // First check whether the radar detect that there is some enemy vehicle close to the island center.

        Vehicle *target = NULL;

        if (CommandCenter *sc = (CommandCenter*)island->getCommandCenter())
        {
            target = findNearestEnemyVehicle(sc->getFaction(),island->getPos(), 3 * 3.6 kmf);

            //printf("Found target %p\n",  target);

            Vehicle *b = target;

            if (!b)
            {
                sc->setUnderAttack(false);
                continue;
            }
            if (!sc->isUnderAttack())
            {
                sc->setTimer(timer);
                sc->setUnderAttack(true);
            }

            // If here, this island is under attack.

            std::vector<size_t> str = island->getStructures();

            for(int i=0;i<str.size();i++)
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

                            std::cout << lb <<  ":Azimuth: " << lb->azimuth << " Inclination: " << lb->elevation << std::endl;

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

                        std::cout << lb <<  ":Azimuth: " << lb->azimuth << " Inclination: " << lb->elevation << std::endl;

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

                        std::cout << lb <<  ":Azimuth: " << lb->azimuth << " Inclination: " << lb->elevation << std::endl;

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

                        std::cout << lb <<  ":Loc: " << firingloc << " Target: " << b->getPos() << std::endl;

                        lb->elevation = -5; // A little bit up.
                        lb->azimuth = getAzimuth((b->getPos())-(firingloc));

                        struct controlregister c;
                        c.pitch = 0.0;
                        c.roll = 0.0;
                        //lb->setControlRegisters(c);
                        lb->setForward(toVectorInFixedSystem(0,0,1,lb->azimuth, -lb->elevation));

                        std::cout << lb <<  ":Azimuth: " << lb->azimuth << " Inclination: " << lb->elevation << std::endl;

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
                            size_t i = entities.push_back(action,action->getGeom());
                            gunshot();

                            action->setDestination(target->getPos());
                            action->enableAuto();

                        }
                    } /**else
                    if(Runway* lb = dynamic_cast<Runway*>(entities[str[i]]))
                    {
                        // Launch airplanes to attack incoming mantas.
                        unsigned long timeevent = sc->getTimer();

                        if (timer==(timeevent + 200))
                        {
                            Vehicle *manta = (lb)->spawn(world,space,MANTA,9);

                            //@FIXME : add the manta but before that fix the problem with the findXXX methods that are too slow.
                            //entities.push_back(manta);

                        }

                        if (timer==(timeevent + 300))
                        {
                            // @FIXME I need to register which manta is around.
                            launchManta(b);
                        }

                        if (timer==(timeevent + 400))
                        {
                            Manta *m = findNearestManta(Manta::FLYING,lb->getFaction(),lb->getPos());

                            if (m)
                            {
                                m->dogfight(b->getPos());
                                m->enableAuto();
                            }
                        }
                        if (timer>(timeevent + 4000005))
                        {

                            Manta *m = findNearestManta(Manta::FLYING,lb->getFaction(),b->getPos());

                            if (m)
                            {
                                m->dogfight(b->getPos());
                            } else {
                                timeevent = timer;
                            }
                        }

                    }**/
                }
            }
        }
    }
}

void buildAndRepair(dSpaceID space, dWorldID world)
{
    for (int i = 0; i < islands.size(); i++)
    {
        BoxIsland *island = islands[i];

        if (island->getStructures().size()<8)
        {
            CommandCenter *c = findCommandCenter(island);
            if (c)
            {
                if (c->getTtl()<=0)
                {
                    // @TODO Structures should be rotated also.  Now they all have the same orientation.
                    int which = (rand() % 30 + 1);
                    Structure *s;


                    if (1<=which && which<=5)
                        s = new LaserTurret(c->getFaction());
                    else if (6<=which && which<=7)
                        s = new Structure(c->getFaction());
                    else if (8<=which && which<=10)
                        s = new Runway(c->getFaction());
                    else if (11<=which && which<=13)
                        s = new Warehouse(c->getFaction());
                    else if (14<=which && which<=20)
                        s = new Turret(c->getFaction());
                    else if (21<=which && which<=23)
                        s = new Artillery(c->getFaction());
                    else if (24<=which && which<=26)
                        s = new Launcher(c->getFaction());
                    else
                        s = new Warehouse(c->getFaction());


                    //int x = (rand() % 2000 + 1); x -= 1000;
                    //int z = (rand() % 2000 + 1); z -= 1000;

                    island->addStructure(s,world);

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

Manta* spawnManta(dSpaceID space, dWorldID world,Vehicle *spawner)
{
    int mantaNumber = findNextNumber(MANTA);
    Vehicle *manta = (spawner)->spawn(world,space,MANTA,mantaNumber);
    if (manta != NULL)
    {
        entities.push_back(manta, manta->getGeom());
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
        //printf("Type and ttl: %d, %d\n", vehicles[i]->getType(),vehicles[i]->getTtl());
        if (entities[i]->getType()==WALRUS && entities[i]->getStatus()==Walrus::SAILING &&
                entities[i]->getFaction()==dock->getFaction() && (dock->getPos()-entities[i]->getPos()).magnitude()<DOCK_RANGE)
        {
            int walrusNumber = ((Walrus*)entities[i])->getNumber()+1;
            char msg[256];
            Message mg;
            mg.faction = entities[i]->getFaction();
            sprintf(msg, "Walrus %d is now back on deck.",walrusNumber+1);
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
        //printf("Type and ttl: %d, %d\n", vehicles[i]->getType(),vehicles[i]->getTtl());
        if (entities[i]->getType()==MANTA && entities[i]->getStatus()==Manta::ON_DECK)
        {
            char str[256];
            Message mg;
            mg.faction = entities[i]->getFaction();
            sprintf(str, "Manta %2d is now on bay.",((Manta*)entities[i])->getNumber());
            mg.msg = std::string(str);

            messages.insert(messages.begin(), mg);
            //printf("Eliminating....\n");
            dBodyDisable(entities[i]->getBodyID());
            entities.erase(entities[i]->getGeom());
        }
    }
}


void landManta(Vehicle *carrier)
{
    // Auto control
    if (carrier->getType() == CARRIER)
    {
        Balaenidae *b = (Balaenidae*)carrier;
        Manta *m = findManta(carrier->getFaction(),Manta::HOLDING);

        if (m)
        {
            m->setDestination(b->getPos());             // @FIXME: This needs to be performed all the time while manta is landing.
            m->setAttitude(b->getForward());
            m->land();
            m->enableAuto();
        }
    }
}


void launchManta(Vehicle *v)
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
            sprintf(msg, "Manta %2d has been launched.", m->getNumber()+1);
            mg.msg = std::string(msg);
            messages.insert(messages.begin(), mg);
            takeoff();
        }
    } else if (v->getType() == LANDINGABLE)
    {
        Runway *r = (Runway*)v;
        Manta *m = findManta(v->getFaction(),Manta::LANDED);
        if (m)
        {
            r->launch(m);
            takeoff();
        }
    }
}

void wipeEnemyStructures(BoxIsland *island)
{
    std::vector<size_t> strs = island->getStructures();

    int faction = island->getCommandCenter()->getFaction();

    for(int i=0;i<strs.size();i++)
    {
        if (faction != entities[strs[i]]->getFaction())
        {
            entities[strs[i]]->damage(10000);
        }
    }
}

void captureIsland(BoxIsland *island, int faction, dSpaceID space, dWorldID world)
{

    Structure *s = island->addStructure(new CommandCenter(faction),world);

    if (s)
    {
        char msg[256];
        Message mg;
        mg.faction = s->getFaction();
        sprintf(msg, "Island %s is now under control of %s.", island->getName().c_str(),FACTION(faction));
        mg.msg = std::string(msg);
        messages.insert(messages.begin(),  mg);

        wipeEnemyStructures(island);
    }
}


void playFaction(unsigned long timer, int faction, dSpaceID space, dWorldID world)
{
    static int statuses[2] = {9,9};
    static unsigned long timeevent = 0;

    // @FIXME: This is SO wrong.
    int status = statuses[faction-1];

    int action=-1;

    {
        Vehicle *b = findCarrier(faction);

        if (!b) return;

        Vehicle *v = findNearestEnemyVehicle(faction,b->getPos(),8000);

        if (v && (v->getType() == CARRIER || v->getType() == WALRUS) && status != 20 && status != 21)
        {
            // Shift to the state to send walruses to destroy the enemy carrier.

            status = 20;timeevent = timer;
        }

        if (v && v->getType() == MANTA && status != 22 && status != 23)
        {
            // Shift to the state to send walruses to destroy the enemy carrier.

            status = 22;timeevent=timer;
        }
    }

    switch (status) {
    case 22:
    {
        Vehicle *b = findCarrier(faction);

        if (!b) return;
        Vehicle *v = findNearestEnemyVehicle(faction,b->getPos(),8000);

        if (!v) {
            Manta *m = findNearestManta(Manta::FLYING,faction,b->getPos());

            if (m)
            {
                landManta(b);
            }
            status=9;timeevent=timer;
        }

        if (timer==(timeevent + 200))
        {
            Manta *m = spawnManta(space,world,b);

        }

        if (timer==(timeevent + 300))
        {
            // @FIXME I need to register which manta is around.
            launchManta(b);
        }

        if (timer>(timeevent + 400))
        {

            Manta *m = findNearestManta(Manta::FLYING,faction,b->getPos());

            if (m)
            {
                m->attack(v->getPos());
                m->enableAuto();
            } else {
                timeevent = timer;
            }
        }



        break;
    }
    case 20:
    {
        Vehicle *b = findCarrier(faction);

        if (!b) return;

        Vehicle *v = findNearestEnemyVehicle(faction,b->getPos(),8000);

        Walrus* w1 = spawnWalrus(space,world,b);

        Walrus* w2 = spawnWalrus(space,world,b);

        w1->attack(v->getPos());
        w1->enableAuto();

        w2->attack(v->getPos());
        w2->enableAuto();

        status=21;timeevent=timer;

        break;
    }
    case 21:
    {
        Vehicle *b = findCarrier(faction);
        Vehicle *v = findNearestEnemyVehicle(faction,b->getPos(),8000);

        Walrus *w1 = findWalrus(Walrus::SAILING,faction);

        if (v)
        {
            w1->attack(v->getPos());
        }
        else
        {
            if (w1) dockWalrus(b);
            w1 = findWalrus(Walrus::SAILING, faction);
            if (w1) dockWalrus(b);

            status = 9;timeevent= timer;
        }


        break;
    }
    case 9:
    {
        Vehicle *b = findCarrier(faction);

        if (b)
        {
            BoxIsland *is = findNearestIsland(b->getPos(),false, faction,100 kmf);

            if (!is)
            {
                // Closest enemy island is not found, lets check for an empty island.
                status=0;
                break;
            }

            Vec3f vector = (b->getPos()) - (is->getPos());

            vector = vector.normalize();

            b->setDestination(is->getPos()+Vec3f(12500.0f * vector));
            b->enableAuto();
            status = 10;
        }

        break;
    }
    case 10:
    {
        Vehicle *b = findCarrier(faction);

        if (b)
        {
            BoxIsland *is = findNearestIsland(b->getPos(),false, faction);

            if (!b->isAuto())
            {
                printf("Carries has arrived to destination.\n");

                //Walrus* w = spawnWalrus(space,world,b);
                //w->setDestination(is->getPos());
                //w->enableAuto();
                status = 11;timeevent = timer;

            }
        }

        break;
    }
    case 11:
    {
        Vehicle *b = findCarrier(faction);

        if (b)
        {
            BoxIsland *is = findNearestIsland(b->getPos(),false, faction);

            // If it has available missiles, then fire them.

            if (timer==(timeevent + 100))
            {
                Missile *a = (Missile*) b->fire(world, space);

                size_t i = CONTROLLING_NONE;
                if (a)
                {
                    i = entities.push_back(a, a->getGeom());

                    CommandCenter *c = (CommandCenter*)is->getCommandCenter();

                    a->setDestination(c->getPos());

                    a->enableAuto();

                    if (a->getType()==CONTROLABLEACTION)
                    {
                        switchControl(entities.indexOf(i));

                    }
                }

            }

            if (timer==(timeevent + 600))
            {
                CommandCenter *c = (CommandCenter*)is->getCommandCenter();

                if (c)
                {
                    status=12;timeevent = timer;
                }
                else
                {
                    status=0;timeevent=timer;
                }

            }

        }

        break;
    }
    case 12:
    {
        Vehicle *b = findCarrier(faction);

        if (b)
        {
            if (timer==(timeevent + 200))
            {
                Manta *m = spawnManta(space,world,b);

            }

            if (timer==(timeevent + 300))
            {
                // @FIXME I need to register which manta is around.
                launchManta(b);
            }

            if (timer==(timeevent + 400))
            {
                BoxIsland *i = findNearestIsland(b->getPos());

                Manta *m = findNearestManta(Manta::FLYING,faction,b->getPos());

                CommandCenter *c = (CommandCenter*)i->getCommandCenter();

                if (!c)
                {
                    status = 0;
                    dockManta();
                } else
                {

                    m->attack(c->getPos());
                    m->enableAuto();
                }
            }


            if (timer>(timeevent + 400))
            {
                BoxIsland *i = findNearestIsland(b->getPos());

                Manta *m = findNearestManta(Manta::FLYING, faction,b->getPos());

                if (!m)
                {
                    status = 11;
                    timeevent = timer;
                    break;
                }

                CommandCenter *c = (CommandCenter*)i->getCommandCenter();

                if (!c)
                {
                    // Command Center is destroyed.

                    // @FIXME: find the right manta, the one which is closer.
                    Manta *m = findNearestManta(Manta::FLYING, faction,b->getPos());


                    if (m)
                    {
                        m->setDestination(Vec3f(i->getPos()[0],1000,i->getPos()[2]));
                        m->enableAuto();
                    }


                    status = 0;
                }
            }
        }

        break;
    }
    case 0: //find nearest island.
    {
        Vehicle *b = findCarrier(faction);

        if (b)
        {
            //BoxIsland *enemyis = findNearestIsland(b->getPos(), false, faction);

            BoxIsland *is = findNearestEmptyIsland(b->getPos());

            Vec3f vector = (b->getPos()) - (is->getPos());

            vector = vector.normalize();

            b->setDestination(is->getPos()+Vec3f(3500.0f * vector));
            b->enableAuto();
            status = 1;
        }

        break;
    }
    case 1:
    {
        Vehicle *b = findCarrier(faction);

        if (b)
        {
            BoxIsland *is = findNearestEmptyIsland(b->getPos());

            if (!b->isAuto())
            {
                printf("Carries has arrived to destination.\n");

                Walrus* w = spawnWalrus(space,world,b);
                w->setDestination(is->getPos());
                w->enableAuto();
                status = 2;timeevent = timer;

                Manta *m = findNearestManta(Manta::HOLDING, faction,b->getPos());

                if (m)
                {
                    landManta(b);
                } else {
                    m = findNearestManta(Manta::FLYING, faction,b->getPos());

                    if (m)
                    {
                        landManta(b);
                    }
                }

            }
        }

        break;
    }
    case 2:
    {
        // Need to check if walrus arrived to island.
        Walrus *w = findWalrus(faction);

        if (w)
        {
            BoxIsland *is = findNearestEmptyIsland(w->getPos());

            // @FIXME: It may happen that the walrus is wandering do nothing never de-autopilot and this is endless.
            if (!w->isAuto() || ( w->getIsland() == is && timer>(timeevent + 1000))  )
            {
                captureIsland(is,w->getFaction(),space, world);
                status=3;
            }
        } else
        {
            // Walrus has been destroyed.
            status = 1;
        }

        break;
    }
    case 3:
    {
        Vehicle *b = findCarrier(faction);
        // @FIXME I should find the one that actually went to the island.
        Walrus *w = findWalrus(faction);

        if (w)
        {
            w->setDestination(b->getPos()+Vec3f(0.0f,0.0f,-50.0f));
            w->enableAuto();
            status = 4;
        } else {
            // There are no walrus so I can departure to the next island.
            status = 9;
        }
        break;
    }
    case 4:
    {
        Vehicle *b = findCarrier(faction);
        Walrus *w = findWalrus(faction);

        Manta *m = findManta(faction,Manta::ON_DECK);

        if (w)
        {
            if (b && w && !(w->isAuto()))
            {
                synchronized(entities.m_mutex)
                {
                    dockWalrus(b);
                    if (m)
                    {
                        dockManta();
                    }
                }
                status=9;
            }
        } else {
            // There are no walrus so I can departure to the next island.
            status = 9;
        }

        break;
    }
    default:
        break;

    }

    statuses[faction-1] = status;

}

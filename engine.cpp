#include "engine.h"

extern  Controller controller;

/* dynamics and collision objects */


extern container<Vehicle*> entities;

extern std::vector<BoxIsland*> islands;

extern std::vector<std::string> messages;

// SYNC
Vehicle* gVehicle(dBodyID body)
{
    for(size_t i=entities.first();entities.exists(i);i=entities.next(i))
    {
        Vehicle *vehicle = entities[i];
        if (vehicle->getBodyID() == body)
        {
            return vehicle;
        }
    }
    return NULL;
}

Vehicle* gVehicle(dGeomID geom)
{
    for(size_t i=entities.first();entities.exists(i);i=entities.next(i))
    {
        Vehicle *vehicle = entities[i];
        if (vehicle->getGeom() == geom)
        {
            return vehicle;
        }
    }
    return NULL;
}

void gVehicle(Vehicle* &v1, Vehicle* &v2, dBodyID b1, dBodyID b2, Structure* &s1, Structure* &s2, dGeomID g1, dGeomID g2)
{
    for(size_t i=entities.first();entities.exists(i);i=entities.next(i))
    {
        Vehicle *vehicle = entities[i];
        if (vehicle->getBodyID() == b1)
        {
            v1 = vehicle;
        }
        if (vehicle->getBodyID() == b2)
        {
            v2 = vehicle;
        }
        if (vehicle->getGeom() == g1 && vehicle->getType() >= COLLISIONABLE)
        {
            s1 = (Structure*)vehicle;
        }
        if (vehicle->getGeom() == g2 && vehicle->getType() >= COLLISIONABLE)
        {
            s2 = (Structure*)vehicle;
        }

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
        b->doControl(controller);
        b->setStatus(Balaenidae::OFFSHORING);
        char str[256];
        sprintf(str, "Carrier has stranded on %s.", island->getName().c_str());
        messages.insert(messages.begin(), str);
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
        sprintf(str, "Walrus has departed from %s.", island->getName().c_str());
        messages.insert(messages.begin(), str);
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
        sprintf(str, "Walrus has arrived to %s.", island->getName().c_str());
        messages.insert(messages.begin(), str);
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
            sprintf(str, "Manta has landed on Island %s.", island->getName().c_str());
            messages.insert(messages.begin(), str);

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

    // Dont hit me
    if (g->getOrigin() != vehicle->getBodyID())
    {
        vehicle->damage(2);
        return false;
    }
    return true;
}

bool hit(Structure* structure)
{
    static Island *b = NULL;

    if (structure)
    {
        if (b != structure->island)
        {
            char str[256];
            sprintf(str, "Island %s under attack!", structure->island->getName().c_str());
            messages.insert(messages.begin(), str);
            b = structure->island;
        }

        structure->damage(2);
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
            messages.insert(messages.begin(), std::string("Manta has landed on Aircraft."));
        }
    }
    return true;
}

void  releasecontrol(dBodyID body)
{
    synchronized(entities.m_mutex)
    {
        Vehicle* vehicle = gVehicle(body);
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


bool  isType(dBodyID body, int type)
{
    bool result = false;
    synchronized(entities.m_mutex)
    {
        Vehicle *vehicle = gVehicle(body);
        result = isType(vehicle,type);
    }
    return result;
}


bool isType(dBodyID body, int types[], int length)
{
    bool result = false;
    synchronized(entities.m_mutex)
    {
        Vehicle *vehicle = gVehicle(body);

        for(int s=0;s<length;s++)
            if (isType(vehicle,types[s])) result=true;
    }
    return result;

}


bool  isManta(dBodyID body)
{
    return isType(body,3);
}

bool  isManta(Vehicle* vehicle)
{
    return isType(vehicle,3);
}

bool  isCarrier(dBodyID body)
{
    return isType(body, 4);
}

bool  isCarrier(Vehicle* vehicle)
{
    return isType(vehicle,4);
}

bool  isWalrus(Vehicle* vehicle)
{
    return isType(vehicle, WALRUS);
}

bool  isAction(dBodyID body)
{
    return isType(body, 5);
}

bool  isAction(Vehicle* vehicle)
{
    return isType(vehicle, 5);
}

// SYNC
bool isRay(dGeomID o)
{
    for(size_t i=entities.first();entities.exists(i);i=entities.next(i))
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
    for(size_t i=entities.first();entities.exists(i);i=entities.next(i))
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
    for(size_t i=entities.first();entities.exists(i);i=entities.next(i))
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
        if (vehicle->getSpeed()>70 and vehicle->getType() == MANTA)
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

void groundcollisions(dBodyID body)
{
    synchronized(entities.m_mutex)
    {
        Vehicle *vehicle = gVehicle(body);
        groundcollisions(vehicle);
    }
}

Manta* findManta(int status)
{
    for(size_t i=entities.first();entities.exists(i);i=entities.next(i))
    {
        Vehicle *v=entities[i];
        if (v->getType() == MANTA && v->getStatus() == status)
        {
            return (Manta*)v;
        }
    }
    return NULL;
}

int findNextNumber(int type)
{
    int numbers[256];
    memset(numbers,0,sizeof(int)*256);

    for(size_t i=entities.first();entities.exists(i);i=entities.next(i))
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

void list()
{
    for(size_t i=entities.first();entities.exists(i);i=entities.next(i))
    {
        printf("[%d]: Body ID (%16p) Position (%d) Type: %d\n", i,(void*)entities[i]->getBodyID(), entities.indexOf(i), entities[i]->getType());
    }
}

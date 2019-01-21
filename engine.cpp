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

void gVehicle(Vehicle* &v1, Vehicle* &v2, dBodyID b1, dBodyID b2, Structure* &s1, Structure*s2, dGeomID g1, dGeomID g2)
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
        if (vehicle->getBodyID()==NULL)
        {
            Structure *s= (Structure*)entities[i];
            if (s->getGeom() == g1)
            {
                s1 = s;
            }
            if (s->getGeom() == g2)
            {
                s2 = s;
            }
        }

    }
}

// SYNC
bool stranded(Vehicle *carrier, Island *island)
{
    if (island && carrier && carrier->getType() == 4 && carrier->getStatus() != Balaenidae::OFFSHORING)
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

        w->setStatus(Walrus::SAILING);
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
    if (island && walrus && walrus->getType() == WALRUS && walrus->getStatus() != Walrus::ROLLING)
    {
        Walrus *w = (Walrus*)walrus;

        w->setStatus(Walrus::ROLLING);
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
    if (manta && island && manta->getType() == 3)
    {
        if (manta->getStatus() != 0)
        {
            char str[256];
            sprintf(str, "Manta has landed on Island %s.", island->getName().c_str());
            messages.insert(messages.begin(), str);
            manta->setStatus(Manta::LANDED);
            manta->setThrottle(0.0f);
            controller.reset();
            manta->doControl(controller);
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
    if (structure)
    {
        //char str[256];
        //sprintf(str, "Island %s under attack!", structure->island->getName().c_str());
        //messages.insert(messages.begin(), str);
    }
    structure->damage(2);
}

// SYNC
bool releasecontrol(Vehicle* vehicle)
{
    if (vehicle && vehicle->getType() == MANTA)
    {
        if (vehicle->getStatus() != 0 && vehicle->getStatus() != 1)
        {
            vehicle->inert = true;
            controller.reset();
            vehicle->setStatus(0);
            vehicle->doControl(controller);
            vehicle->setThrottle(0.0f);
            messages.insert(messages.begin(), std::string("Manta has re landed on Aircraft."));
        }
    }
    return true;
}

void inline releasecontrol(dBodyID body)
{
    synchronized(entities.m_mutex)
    {
        Vehicle* vehicle = gVehicle(body);
        releasecontrol(vehicle);
    }
}


// SYNC
bool inline isType(Vehicle *vehicle, int type)
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


bool inline isType(dBodyID body, int type)
{
    bool result = false;
    synchronized(entities.m_mutex)
    {
        Vehicle *vehicle = gVehicle(body);
        result = isType(vehicle,type);
    }
    return result;
}

bool inline isManta(dBodyID body)
{
    return isType(body,3);
}

bool inline isManta(Vehicle* vehicle)
{
    return isType(vehicle,3);
}

bool inline isCarrier(dBodyID body)
{
    return isType(body, 4);
}

bool inline isCarrier(Vehicle* vehicle)
{
    return isType(vehicle,4);
}

bool inline isWalrus(Vehicle* vehicle)
{
    return isType(vehicle, WALRUS);
}

bool inline isAction(dBodyID body)
{
    return isType(body, 5);
}

bool inline isAction(Vehicle* vehicle)
{
    return isType(vehicle, 5);
}


bool inline isRunway(Structure* s)
{
    if (s && s->getType()==LANDINGABLE)
    {
        return true;
    }
    return false;
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

bool inline isIsland(dGeomID candidate)
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

// SYNC
bool inline groundcollisions(Vehicle *vehicle)
{
    if (vehicle)
    {
        if (vehicle->getSpeed()>100 and vehicle->getType() == MANTA)
        {
            explosion();
            controller.reset();
            vehicle->doControl(controller);
            vehicle->setThrottle(0.0f);
            vehicle->damage(200);
        }
    }
    return true;
}

void inline groundcollisions(dBodyID body)
{
    synchronized(entities.m_mutex)
    {
        Vehicle *vehicle = gVehicle(body);
        groundcollisions(vehicle);
    }
}

#ifndef AI_H
#define AI_H

#include "engine.h"

#define DEFENSE_RANGE   8000
#define IN_RANGE        30000

class QAction
{
private:
    unsigned long timerdelay;
public:
    QAction() {}

    void set(unsigned long timer)
    {
        timerdelay = timer;
    }

    bool delay(unsigned long timer, unsigned long delay)
    {
        return timer > (timerdelay + delay);
    }

    int virtual apply(const int status,int faction,unsigned long &timerevent, unsigned long timer)
    {
        return status;
    }

};

class DefCon : public QAction
{
public:
    DefCon() { }

    int virtual apply(int status,int faction,unsigned long &timerevent, unsigned long timer);
};

class AirDefense : public QAction
{
public:
    AirDefense() { }
    int virtual apply(int status, int faction, unsigned long &timerevent, unsigned long timer);
};

class NavalDefense : public QAction
{
public:
    NavalDefense() { }
    int virtual apply(int status, int faction, unsigned long &timerevent, unsigned long timer);
};

class NavalDefending : public QAction
{
public:
    NavalDefending() { }
    int virtual apply(int status, int faction, unsigned long &timerevent, unsigned long timer);
};
class ApproachEnemyIsland : public QAction
{
public:
    ApproachEnemyIsland() { }
    int virtual apply(int status, int faction, unsigned long &timerevent, unsigned long timer);
};


class ApproachAnyEnemyIsland : public QAction
{
public:
    ApproachAnyEnemyIsland() { }
    int virtual apply(int status, int faction, unsigned long &timerevent, unsigned long timer);
};

class ApproachingEnemyIsland : public QAction
{
public:
    ApproachingEnemyIsland() { }
    int virtual apply(int status, int faction, unsigned long &timerevent, unsigned long timer);
};
class BallisticAttack : public QAction
{
public:
    BallisticAttack() { }
    int virtual apply(int status, int faction, unsigned long &timerevent, unsigned long timer);
};

class AirborneAttack : public QAction
{
public:
    AirborneAttack() { }
    int virtual apply(int status, int faction, unsigned long &timerevent, unsigned long timer);
};
class ApproachFreeIsland : public QAction
{
public:
    ApproachFreeIsland() { }
    int virtual apply(int status, int faction, unsigned long &timerevent, unsigned long timer);
};

class InvadeIsland : public QAction
{
public:
    InvadeIsland() { }
    int virtual apply(int status, int faction, unsigned long &timerevent, unsigned long timer);
};

class AirborneInvadeIsland : public QAction
{
public:
    AirborneInvadeIsland() { }
    int virtual apply(int status, int faction, unsigned long &timerevent, unsigned long timer);
};

class CaptureIsland : public QAction
{
public:
    CaptureIsland() { }
    int virtual apply(int status, int faction, unsigned long &timerevent, unsigned long timer);
};

class ReturnToCarrier : public QAction
{
public:
    ReturnToCarrier() { }
    int virtual apply(int status, int faction, unsigned long &timerevent, unsigned long timer);
};
class DockBack : public QAction
{
public:
    DockBack() { }
    int virtual apply(int status, int faction, unsigned long &timerevent, unsigned long timer);
};


class Player
{
private:

    int faction;
    int state;
    unsigned long timeevent;

    QAction *interruption;

    QAction *qactions[25];

public:
    Player(int faction);
    void playFaction(unsigned long timer);
    int pickQAction();

};

#endif // AI_H

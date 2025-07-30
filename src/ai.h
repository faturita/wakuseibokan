#ifndef AI_H
#define AI_H

#include "engine.h"

#define DEFENSE_RANGE   5000
#define IN_RANGE        30000

class TSequencer {
    const int length = 15;
    unsigned int T[15];
public:
    TSequencer()
    {
        memset(&T,0,length*sizeof(unsigned int));
    }

    void init()
    {
        memset(&T,0,length);
    }

    TSequencer operator+(TSequencer T2)
    {
        for(int i=0;i<length;i++)
        {
            T[i] += T2[i];
        }
        return *this;
    }

    TSequencer operator*(unsigned int val)
    {
        for(int i=0;i<length;i++)
        {
            T[i] *= val;
        }
        return *this;
    }

    unsigned int &operator[](int idx)
    {
        return T[idx];
    }

    void set(int idx, unsigned int val)
    {
        T[idx]=val;
    }

    TSequencer operator=(TSequencer T2)
    {
        for(int i=0;i<length;i++)
        {
            T[i] = T2[i];
        }
        return *this;
    }

    TSequencer sign()
    {
        TSequencer T2;
        for(int i=0;i<length;i++)
        {
            T2.set(i,signzero(T[i]));
        }
        return T2;
    }

    void print()
    {
        printf("[");
        for(int i=0;i<length;i++)
        {
            printf("%lu,", T[i]);
        }
        printf("]\n");
    }
};

enum class State {
    IDLE=0,
    DOCKING=1,
    DOCKED=3,
    APPROACHFREEISLAND=4,
    APPROACHENEMYISLAND=5,
    APPROACHFRIENDLYISLAND=6,
    INVADEISLAND=7,
    RENDEZVOUS=8,
    BALLISTICATTACK=9,
    AIRBORNEATTACK=10,
    AIRDEFENSE=11,
    REFUEL=12
};


// class State
// {
// protected:
//     TSequencer T;
// public:
//     StateType type;
//     State()
//     {
//         type = StateType::IDLE;
//     }

//     State(StateType type) 
//     {
//         State::type = type;
//     }

//     void tick()
//     {
//         T = T + T.sign() * 1;
//     }

// };


class QAction
{   
public:
    virtual void start()
    {
        return ;
    }
    virtual void apply(int faction)
    {
        return ;
    }
};


class Condition 
{
protected:

public:
    bool virtual evaluate(int faction)
    {
        return false;
    }
};

class Transition
{
protected:
    State s;
    State sprime;
    Condition *c;
public:
    Transition(State s, State sprime, Condition *c) 
    { 
        this->s = s;
        this->sprime = sprime;
        this->c = c;
    }
    State virtual transit(int faction, const State current )
    {
        if (current == s && c->evaluate(faction))
        {
            Vehicle *v = findCarrier(faction);

            if (v) std::cout << (int)v->getFaction() << " - Transitioning from " << (int)s << " to " << (int)sprime << std::endl;
            return sprime;
        }
        return current;
    }    
};

class Interruption : public Transition
{
public:
    Interruption(State sprime, Condition *c) :  Transition (State::IDLE, sprime, c)
    { 
        this->sprime = sprime;
        this->c = c;
    }

    // When interruptions return true, the current state is interrupted and the new state is returned.
    State virtual transit(int faction, const State current )
    {
        if (c->evaluate(faction))
        {

            if (current != sprime)
                s = current;  // Save the current state to return to it later.

            return sprime;
        } else {
            if (current == sprime)
            {
                return s; // Return to the previous state.
            } else {
                return current; // Stay in the current state.
            }
        }
    }   
};    


class Player
{
private:

    int faction;
    State state;

public:
    Transition *transitions[25];
    QAction *qactions[25];
    Interruption *interruptions[25];
    Player(int faction);
    void playFaction(unsigned long timer);
    void playStrategy(unsigned long timer);
    State getCurrentState();
};

#endif // AI_H

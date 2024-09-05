#ifndef AI_H
#define AI_H

#include "engine.h"

#define DEFENSE_RANGE   8000
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
            T2.set(i,sgn(T[i]));
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
    DOCKING,
    DOCKED
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
    virtual void apply(int faction)
    {
        return ;
    }
};


class Condition 
{

protected:
    int faction;
    State s;
    State sprime;
public:
    Condition(int faction, State s, State sprime) 
    { 
        this->faction = faction;
        this->s = s;
        this->sprime = sprime;
    }
    State virtual evaluate(const State current )
    {
        return current;
    }
};


class Player
{
private:

    int faction;
    State state;

public:
    Condition *conditions[25];
    QAction *qactions[25];
    Player(int faction);
    void playFaction(unsigned long timer);
    State getCurrentState();
};

#endif // AI_H

#ifndef COMMANDORDER_H
#define COMMANDORDER_H

enum class Command {
    None=0,
    AttackOrder,
    DestinationOrder,
    TaxiOrder,
    TelemetryOrder,
    LaunchOrder,
    CaptureOrder,
    AutoOrder,
    StopOrder,
    SpawnOrder,
    DockOrder,
    FireOrder,
    LandOrder};


enum TargetType {
    Air_To_Air = 1,
    Air_To_Ground = 2
};

struct commandparameters
{
    int spawnid;
    int typeofisland;
    float x;
    float y;
    float z;
    TargetType target_type;
    bool bit;
    int weapon;
};

struct CommandOrder
{
    Command command;
    commandparameters parameters;
};

struct controlregister
{
    // R+,F-
    float thrust=0;

    // ModAngleX
    float roll=0;

    // ModAngleY
    float pitch=0;

    // ModAngleZ
    float yaw=0;

    // ModAngleP
    float precesion=0;

    float bank=0;
};

struct ControlStructure {
    int controllingid;
    struct controlregister registers;
    int faction;
    unsigned long sourcetimer;
    CommandOrder order;
};



#endif // COMMANDORDER_H

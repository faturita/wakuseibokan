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
    RecoveryOrder,
    FireOrder,
    LandOrder,
    JoinOrder,
    RefuelOrder,
    Departure,
    RefillOrder,
    UnfillOrder,
    CollectOrder,
    DockOrder};


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
    char buf[20];
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


#pragma pack(push, 1)

struct controlregister2
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

struct ControlStructure2 {
    int32_t controllingid;
    struct controlregister2 registers;
    int32_t faction;
    int32_t command;
    int32_t spawnid;
    int32_t typeofisland;
    float x;
    float y;
    float z;
    int32_t target_type;
    int32_t weapon;
    uint32_t sourcetimer;
};
#pragma pack(pop)


#endif // COMMANDORDER_H

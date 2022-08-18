#ifndef LEDGER_H
#define LEDGER_H

#include "../units/Vehicle.h"

extern FILE *ledger;

struct LogStructure {
    float pos1;
    float pos2;
    float pos3;
    float r1;
    float r2;
    float r3;
    float r4;
    float r5;
    float r6;
    float r7;
    float r8;
    float r9;
    float r10;
    float r11;
    float r12;

};

struct TickRecord {
    unsigned long timerparam;
    size_t id;
    int type;
    int subtype;
    int faction;
    LogStructure location;
};

void record(unsigned long timerparam, size_t id, Vehicle* v);

#endif // LEDGER_H

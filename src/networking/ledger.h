#ifndef LEDGER_H
#define LEDGER_H

#include "../units/Vehicle.h"

extern FILE *ledger;

void record(unsigned long timerparam, size_t id, Vehicle* v);

void notify(unsigned long timerparam, size_t id, Vehicle *v);

void disconnect();
void init_lobby();
void join_lobby();
int receive(TickRecord *record);

#endif // LEDGER_H

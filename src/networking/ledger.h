#ifndef LEDGER_H
#define LEDGER_H

#include "../units/Vehicle.h"

extern FILE *ledger;

void record(unsigned long timerparam, size_t id, Vehicle* v);


#endif // LEDGER_H

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ledger.h"

#include "telemetry.h"


void record(unsigned long timerparam, size_t id, Vehicle* v)
{
    TickRecord tickrecord;

    tickrecord = v->serialize();

    tickrecord.timerparam = timerparam;
    tickrecord.id = id;

    fwrite(&tickrecord, sizeof(TickRecord),1,ledger);
}






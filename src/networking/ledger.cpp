#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ledger.h"

void record(unsigned long timerparam, size_t id, Vehicle* v)
{
    TickRecord record;

    float dBodyRotation[12];

    v->getR(dBodyRotation);

    Vec3f dBodyPosition = v->getPos();

    record.timerparam = timerparam;
    record.id = id;
    record.type = v->getType();
    record.subtype = v->getSubType();


    record.location.pos1 = dBodyPosition[0];
    record.location.pos2 = dBodyPosition[1];
    record.location.pos3 = dBodyPosition[2];
    record.location.r1 = dBodyRotation[0];
    record.location.r2 = dBodyRotation[1];
    record.location.r3 = dBodyRotation[2];
    record.location.r4 = dBodyRotation[3];
    record.location.r5 = dBodyRotation[4];
    record.location.r6 = dBodyRotation[5];
    record.location.r7 = dBodyRotation[6];
    record.location.r8 = dBodyRotation[7];
    record.location.r9 = dBodyRotation[8];
    record.location.r10 = dBodyRotation[9];
    record.location.r11 = dBodyRotation[10];
    record.location.r12 = dBodyRotation[11];

    fwrite(&record, sizeof(TickRecord),1,ledger);
}

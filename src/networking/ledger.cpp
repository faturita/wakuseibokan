#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ledger.h"

void record(unsigned long timerparam, size_t id, Vehicle* v)
{
    TickRecord tickrecord;

    float dBodyRotation[12];

    v->getR(dBodyRotation);

    Vec3f dBodyPosition = v->getPos();

    tickrecord.timerparam = timerparam;
    tickrecord.id = id;
    tickrecord.type = v->getType();
    tickrecord.subtype = v->getSubType();


    tickrecord.location.pos1 = dBodyPosition[0];
    tickrecord.location.pos2 = dBodyPosition[1];
    tickrecord.location.pos3 = dBodyPosition[2];
    tickrecord.location.r1 = dBodyRotation[0];
    tickrecord.location.r2 = dBodyRotation[1];
    tickrecord.location.r3 = dBodyRotation[2];
    tickrecord.location.r4 = dBodyRotation[3];
    tickrecord.location.r5 = dBodyRotation[4];
    tickrecord.location.r6 = dBodyRotation[5];
    tickrecord.location.r7 = dBodyRotation[6];
    tickrecord.location.r8 = dBodyRotation[7];
    tickrecord.location.r9 = dBodyRotation[8];
    tickrecord.location.r10 = dBodyRotation[9];
    tickrecord.location.r11 = dBodyRotation[10];
    tickrecord.location.r12 = dBodyRotation[11];

    fwrite(&tickrecord, sizeof(TickRecord),1,ledger);
}

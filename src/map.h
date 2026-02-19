#ifndef MAP_H
#define MAP_H

#include "math/vec3f.h"

void drawMap();
void zoommapin();
void centermap(int ccx, int ccy);

void zoommapout();

Vec3f setLocationOnMap(int ccx, int ccy);

void centerMapOnWorldPos(Vec3f pos);

#endif // MAP_H

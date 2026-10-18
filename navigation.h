#ifndef NAVIGATION_H
#define NAVIGATION_H

#include "parking.h"

typedef struct {
    float x, y, z;
} Vec3;

#define MAX_PATH_WAYPOINTS 64

extern Vec3 currentPath[MAX_PATH_WAYPOINTS];
extern int currentPathCount;

void initNavigationNetwork(void);
int calculatePathToSlot(int slotIndex, Vec3* pathOut, int* countOut);
void drawPath(int is3D);

#endif

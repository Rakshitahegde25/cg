#ifndef VEHICLE_H
#define VEHICLE_H

#include "navigation.h"

typedef struct {
    float x, y, z;
    float angle;       // Current heading angle in degrees
    float targetAngle;
    int isNavigating;  // 1 if driving, 0 if idle/parked
    int isPaused;      // 1 if navigation is paused, 0 if active
    int currentWaypoint;
    int targetSlotIndex;
    float speed;
    float color[3];
} Vehicle;

extern Vehicle activeVehicle;
extern char statusMessage[256];
extern char statusFeedback[256];

void initVehicle(void);
void startVehicleNavigation(int slotIndex);
void togglePauseVehicleNavigation(void);
void resetVehicleNavigation(void);
void updateVehicle(float deltaTime);
void drawVehicle(int is3D);

void perSecondTimerCB(int val);

#endif

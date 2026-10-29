#ifndef VEHICLE_H
#define VEHICLE_H

#include "navigation.h"

typedef struct {
    float x, y, z;
    float angle;       // Current heading angle in degrees
    float targetAngle;
    int isNavigating;  // 1 if driving, 0 if idle/parked
    int isExiting;     // 1 if exiting to gate, 0 if parking
    int isPaused;      // 1 if navigation is paused, 0 if active
    int isReversing;   // 1 if reversing out of slot, 0 if forward
    int currentWaypoint;
    int targetSlotIndex;
    int exitSlotIndex;  // Slot to release when exit is complete (-1 if none)
    float speed;
    float color[3];
} Vehicle;

extern Vehicle activeVehicle;
extern char statusMessage[256];
extern char statusFeedback[256];

void initVehicle(void);
void startVehicleNavigation(int slotIndex);
void startExitNavigation(int slotIndex);
void togglePauseVehicleNavigation(void);
void resetVehicleNavigation(void);
void updateVehicle(float deltaTime);
void drawVehicle(int is3D);

void perSecondTimerCB(int val);

#endif

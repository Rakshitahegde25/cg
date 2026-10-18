#include "vehicle.h"
#include "graphics.h"
#include "parking.h"
#include <windows.h>
#include <GL/gl.h>
#include <GL/glut.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Vehicle activeVehicle;
char statusMessage[256] = "System Ready. Select an AVAILABLE slot & press 'B' to Book.";
char statusFeedback[256] = "Ready";

void initVehicle(void) {
    activeVehicle.x = -30.0f;
    activeVehicle.y = 0.0f;
    activeVehicle.z = -26.0f;
    activeVehicle.angle = 90.0f; // Facing +Z
    activeVehicle.targetAngle = 90.0f;
    activeVehicle.isNavigating = 0;
    activeVehicle.isPaused = 0;
    activeVehicle.currentWaypoint = 0;
    activeVehicle.targetSlotIndex = -1;
    activeVehicle.speed = 14.0f;
    activeVehicle.color[0] = 0.9f;
    activeVehicle.color[1] = 0.2f;
    activeVehicle.color[2] = 0.2f;
}

void startVehicleNavigation(int slotIndex) {
    if (slotIndex < 0 || slotIndex >= slotCount) {
        sprintf(statusFeedback, "Error: No valid slot selected.");
        return;
    }

    ParkingSlot target = slots[slotIndex];

    if (target.status == STATUS_RESERVED || target.type == TYPE_RESERVED) {
        activeVehicle.isNavigating = 0;
        activeVehicle.isPaused = 0;
        currentPathCount = 0;
        sprintf(statusFeedback, "Navigation unavailable: This slot is reserved.");
        return;
    }

    if (target.status == STATUS_OCCUPIED || target.status == STATUS_TIME_EXPIRED) {
        activeVehicle.isNavigating = 0;
        activeVehicle.isPaused = 0;
        currentPathCount = 0;
        sprintf(statusFeedback, "Navigation unavailable: This slot is already occupied.");
        return;
    }

    initVehicle();
    activeVehicle.targetSlotIndex = slotIndex;

    if (calculatePathToSlot(slotIndex, currentPath, &currentPathCount)) {
        activeVehicle.isNavigating = 1;
        activeVehicle.isPaused = 0;
        activeVehicle.currentWaypoint = 1;
        activeVehicle.x = currentPath[0].x;
        activeVehicle.z = currentPath[0].z;

        sprintf(statusFeedback, "Route Found (A*) - Navigating to Slot %s...", target.id);
    } else {
        sprintf(statusFeedback, "Error: Could not calculate route to %s", target.id);
    }
}

void togglePauseVehicleNavigation(void) {
    // Toggles pause state for active vehicle movement OR active parking timers
    if (activeVehicle.isNavigating) {
        activeVehicle.isPaused = !activeVehicle.isPaused;
        if (activeVehicle.isPaused) {
            sprintf(statusFeedback, "Navigation PAUSED at Waypoint %d", activeVehicle.currentWaypoint);
        } else {
            sprintf(statusFeedback, "Navigation RESUMED to %s", slots[activeVehicle.targetSlotIndex].id);
        }
    } else if (selectedSlotIndex >= 0 && selectedSlotIndex < slotCount) {
        if (slots[selectedSlotIndex].timerActive) {
            slots[selectedSlotIndex].timerActive = !slots[selectedSlotIndex].timerActive;
            if (slots[selectedSlotIndex].timerActive) {
                sprintf(statusFeedback, "TIMER RESUMED for Slot %s", slots[selectedSlotIndex].id);
            } else {
                sprintf(statusFeedback, "TIMER PAUSED for Slot %s", slots[selectedSlotIndex].id);
            }
        }
    }
}

void resetVehicleNavigation(void) {
    initVehicle();
    currentPathCount = 0;
    sprintf(statusFeedback, "Route & Navigation Reset");
}

void updateVehicle(float deltaTime) {
    if (!activeVehicle.isNavigating || activeVehicle.isPaused) return;

    if (activeVehicle.currentWaypoint >= currentPathCount) {
        // Reached destination slot!
        activeVehicle.isNavigating = 0;
        activeVehicle.isPaused = 0;

        if (activeVehicle.targetSlotIndex >= 0) {
            int idx = activeVehicle.targetSlotIndex;
            slots[idx].status = STATUS_OCCUPIED;
            slots[idx].carColorIndex = rand() % 6;
            
            // Start parking countdown timer if duration was set during booking
            if (slots[idx].parkingDuration > 0) {
                startSlotTimer(idx, slots[idx].parkingDuration, slots[idx].isTestTimer);
            } else {
                startSlotTimer(idx, 7200, 0); // Default 2 hours if not set
            }

            updateParkingStats();
            sprintf(statusFeedback, "Destination Reached! Slot %s is now OCCUPIED. Timer Started.", slots[idx].id);
        }
        return;
    }

    Vec3 targetWP = currentPath[activeVehicle.currentWaypoint];
    float dx = targetWP.x - activeVehicle.x;
    float dz = targetWP.z - activeVehicle.z;
    float dist = sqrtf(dx * dx + dz * dz);

    if (dist < 0.35f) {
        activeVehicle.currentWaypoint++;
        if (activeVehicle.currentWaypoint >= currentPathCount) {
            activeVehicle.isNavigating = 0;
            activeVehicle.isPaused = 0;
            if (activeVehicle.targetSlotIndex >= 0) {
                int idx = activeVehicle.targetSlotIndex;
                slots[idx].status = STATUS_OCCUPIED;
                slots[idx].carColorIndex = rand() % 6;

                if (slots[idx].parkingDuration > 0) {
                    startSlotTimer(idx, slots[idx].parkingDuration, slots[idx].isTestTimer);
                } else {
                    startSlotTimer(idx, 7200, 0);
                }

                updateParkingStats();
                sprintf(statusFeedback, "Destination Reached! Slot %s is now OCCUPIED. Timer Started.", slots[idx].id);
            }
            return;
        }
        targetWP = currentPath[activeVehicle.currentWaypoint];
        dx = targetWP.x - activeVehicle.x;
        dz = targetWP.z - activeVehicle.z;
        dist = sqrtf(dx * dx + dz * dz);
    }

    float desiredAngle = atan2f(dx, dz) * 180.0f / (float)M_PI;
    
    float diff = desiredAngle - activeVehicle.angle;
    while (diff > 180.0f) diff -= 360.0f;
    while (diff < -180.0f) diff += 360.0f;
    activeVehicle.angle += diff * 10.0f * deltaTime;

    float moveStep = activeVehicle.speed * deltaTime;
    if (moveStep > dist) moveStep = dist;

    if (dist > 0.001f) {
        activeVehicle.x += (dx / dist) * moveStep;
        activeVehicle.z += (dz / dist) * moveStep;
    }
}

void drawVehicle(int is3D) {
    if (!activeVehicle.isNavigating) return;

    if (is3D) {
        glPushMatrix();
        glTranslatef(activeVehicle.x, activeVehicle.y, activeVehicle.z);
        glRotatef(activeVehicle.angle, 0.0f, 1.0f, 0.0f);
        drawCar3D(activeVehicle.color[0], activeVehicle.color[1], activeVehicle.color[2]);
        glPopMatrix();
    } else {
        glPushMatrix();
        glTranslatef(activeVehicle.x, 0.3f, activeVehicle.z);
        glRotatef(activeVehicle.angle, 0.0f, 1.0f, 0.0f);
        drawCar2D(activeVehicle.color[0], activeVehicle.color[1], activeVehicle.color[2]);
        glPopMatrix();
    }
}

// -------------------------------------------------------------
// NON-BLOCKING PER-SECOND TIMER ENGINE
// -------------------------------------------------------------
void perSecondTimerCB(int val) {
    for (int i = 0; i < slotCount; i++) {
        // 1. Parking Timer Countdown
        if (slots[i].timerActive && (slots[i].status == STATUS_OCCUPIED || slots[i].status == STATUS_TIME_EXPIRED)) {
            if (slots[i].remainingTime > 0) {
                slots[i].remainingTime--;

                // Warning feedback messages
                if (i == selectedSlotIndex) {
                    if (slots[i].remainingTime == 600) {
                        snprintf(statusFeedback, sizeof(statusFeedback), "WARNING: Parking time ending soon for Slot %.10s! (10 min remaining)", slots[i].id);
                    } else if (slots[i].remainingTime == 300) {
                        snprintf(statusFeedback, sizeof(statusFeedback), "URGENT: Parking time expires soon for Slot %.10s! (5 min remaining)", slots[i].id);
                    }
                }
            }

            if (slots[i].remainingTime <= 0) {
                slots[i].remainingTime = 0;
                slots[i].timerActive = 0;
                slots[i].timeExpired = 1;
                slots[i].status = STATUS_TIME_EXPIRED;
                updateParkingStats();
                if (i == selectedSlotIndex || selectedSlotIndex == -1) {
                    snprintf(statusFeedback, sizeof(statusFeedback), "PARKING TIME EXPIRED!");
                }
            }
        }

        // 2. EV Charging Timer Countdown
        if (slots[i].type == TYPE_EV && (slots[i].status == STATUS_OCCUPIED || slots[i].status == STATUS_TIME_EXPIRED)) {
            if (slots[i].chargingRemaining > 0) {
                slots[i].chargingRemaining--;
            }
        }

        // 3. Reservation Timer Countdown
        if (slots[i].status == STATUS_RESERVED) {
            if (slots[i].reservationRemaining > 0) {
                slots[i].reservationRemaining--;
                if (slots[i].reservationRemaining <= 0) {
                    slots[i].reservationRemaining = 0;
                    slots[i].status = STATUS_AVAILABLE;
                    updateParkingStats();
                    if (i == selectedSlotIndex) {
                        snprintf(statusFeedback, sizeof(statusFeedback), "Reservation Expired! Slot %.10s is now AVAILABLE.", slots[i].id);
                    }
                }
            }
        }
    }

    glutPostRedisplay();
    glutTimerFunc(1000, perSecondTimerCB, 0);
}

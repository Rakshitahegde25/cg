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
    activeVehicle.isExiting = 0;
    activeVehicle.isPaused = 0;
    activeVehicle.isReversing = 0;
    activeVehicle.currentWaypoint = 0;
    activeVehicle.targetSlotIndex = -1;
    activeVehicle.exitSlotIndex = -1;
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

void startExitNavigation(int slotIndex) {
    if (slotIndex < 0 || slotIndex >= slotCount) {
        sprintf(statusFeedback, "Error: No valid slot to exit.");
        return;
    }

    ParkingSlot target = slots[slotIndex];

    if (target.status != STATUS_OCCUPIED && target.status != STATUS_TIME_EXPIRED) {
        sprintf(statusFeedback, "Slot %s is not occupied. Cannot exit.", target.id);
        return;
    }

    // Save slot color BEFORE any changes — do NOT release the slot yet.
    // The slot stays occupied (parked car remains visible) until the animated
    // vehicle pulls out past the aisle waypoint.
    int savedColor = slots[slotIndex].carColorIndex;

    // Setup vehicle at the slot position
    activeVehicle.targetSlotIndex = slotIndex;
    activeVehicle.exitSlotIndex = slotIndex; // Deferred release
    activeVehicle.x = target.x;
    activeVehicle.y = 0.0f;
    activeVehicle.z = target.z;
    activeVehicle.speed = 10.0f; // Slower for realistic exit
    activeVehicle.isPaused = 0;

    // The car is parked facing INTO the slot. The slot.angle field points
    // outward toward the aisle. When parked, the car's nose faces the opposite
    // direction (into the slot). We keep this parked heading and REVERSE out.
    //
    // Parked heading = slot.angle + 180 (nose into slot)
    // The car reverses toward the aisle while keeping its nose pointed in.
    float parkedAngle = target.angle + 180.0f;
    if (parkedAngle >= 360.0f) parkedAngle -= 360.0f;
    activeVehicle.angle = parkedAngle;
    activeVehicle.targetAngle = parkedAngle;
    activeVehicle.isReversing = 1; // Start in reverse

    // Use the car's color from the parked slot
    static float exitColors[6][3] = {
        {0.90f, 0.15f, 0.15f},
        {0.15f, 0.40f, 0.85f},
        {0.30f, 0.35f, 0.40f},
        {0.95f, 0.65f, 0.10f},
        {0.10f, 0.65f, 0.30f},
        {0.92f, 0.92f, 0.95f}
    };
    activeVehicle.color[0] = exitColors[savedColor % 6][0];
    activeVehicle.color[1] = exitColors[savedColor % 6][1];
    activeVehicle.color[2] = exitColors[savedColor % 6][2];

    // Calculate path from slot to exit gate (with pull-out waypoints and A* routing)
    if (calculatePathFromSlotToExit(slotIndex, currentPath, &currentPathCount)) {
        activeVehicle.isNavigating = 1;
        activeVehicle.isExiting = 1;
        activeVehicle.currentWaypoint = 1; // Skip first point (slot position, we're already there)

        sprintf(statusFeedback, "EXIT Route (A*) - Navigating from Slot %s to EXIT gate...", target.id);
    } else {
        // Path calculation failed — release immediately as fallback
        releaseSlot(slotIndex);
        activeVehicle.isNavigating = 0;
        activeVehicle.isExiting = 0;
        activeVehicle.isReversing = 0;
        activeVehicle.exitSlotIndex = -1;
        currentPathCount = 0;
        sprintf(statusFeedback, "Slot %s Released. (Could not calculate exit route)", target.id);
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

// Helper: finalize exit — release deferred slot, reset vehicle to entrance
static void finalizeExit(void) {
    // Release the deferred exit slot if still held
    if (activeVehicle.exitSlotIndex >= 0 && activeVehicle.exitSlotIndex < slotCount) {
        if (slots[activeVehicle.exitSlotIndex].status == STATUS_OCCUPIED ||
            slots[activeVehicle.exitSlotIndex].status == STATUS_TIME_EXPIRED) {
            releaseSlot(activeVehicle.exitSlotIndex);
        }
    }
    activeVehicle.isExiting = 0;
    activeVehicle.isReversing = 0;
    currentPathCount = 0;
    sprintf(statusFeedback, "Vehicle has EXITED through the gate. Slot released.");
    initVehicle();
}

// Helper: finalize parking arrival
static void finalizeParkingArrival(void) {
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
}

void updateVehicle(float deltaTime) {
    if (!activeVehicle.isNavigating || activeVehicle.isPaused) return;

    // Check if we've gone past all waypoints
    if (activeVehicle.currentWaypoint >= currentPathCount) {
        activeVehicle.isNavigating = 0;
        activeVehicle.isPaused = 0;

        if (activeVehicle.isExiting) {
            finalizeExit();
        } else {
            finalizeParkingArrival();
        }
        return;
    }

    Vec3 targetWP = currentPath[activeVehicle.currentWaypoint];
    float dx = targetWP.x - activeVehicle.x;
    float dz = targetWP.z - activeVehicle.z;
    float dist = sqrtf(dx * dx + dz * dz);

    // Reached the current waypoint — advance
    if (dist < 0.35f) {
        // -- Deferred slot release: when the car reaches the aisle waypoint
        //    (waypoint 2 = first aisle node after pull-out), release the slot
        //    so the parked car disappears and the animated car takes over.
        if (activeVehicle.isExiting && activeVehicle.exitSlotIndex >= 0) {
            // Release on waypoint 2 (the aisle node after the pull-out intermediate point)
            if (activeVehicle.currentWaypoint >= 2) {
                if (slots[activeVehicle.exitSlotIndex].status == STATUS_OCCUPIED ||
                    slots[activeVehicle.exitSlotIndex].status == STATUS_TIME_EXPIRED) {
                    releaseSlot(activeVehicle.exitSlotIndex);
                }
                // Don't release again
                activeVehicle.exitSlotIndex = -1;
            }
        }

        // -- Transition from reverse to forward driving at the aisle node
        if (activeVehicle.isExiting && activeVehicle.isReversing) {
            // We reverse through waypoints 1 and 2 (pull-out + aisle approach).
            // Once we reach waypoint 2 (aisle center), switch to forward driving.
            if (activeVehicle.currentWaypoint >= 2) {
                activeVehicle.isReversing = 0;
                activeVehicle.speed = 14.0f; // Speed up for road driving
            }
        }

        activeVehicle.currentWaypoint++;

        if (activeVehicle.currentWaypoint >= currentPathCount) {
            activeVehicle.isNavigating = 0;
            activeVehicle.isPaused = 0;

            if (activeVehicle.isExiting) {
                finalizeExit();
            } else {
                finalizeParkingArrival();
            }
            return;
        }
        targetWP = currentPath[activeVehicle.currentWaypoint];
        dx = targetWP.x - activeVehicle.x;
        dz = targetWP.z - activeVehicle.z;
        dist = sqrtf(dx * dx + dz * dz);
    }

    // --- Angle and movement ---
    if (activeVehicle.isReversing) {
        // REVERSE MODE: The car moves toward the target waypoint, but the
        // car's nose points AWAY from the direction of travel (i.e., the car
        // backs out). We compute the desired travel direction, then set the
        // car's heading to the opposite (travel_dir + 180).
        float travelAngle = atan2f(dx, dz) * 180.0f / (float)M_PI;
        float reverseHeading = travelAngle + 180.0f;
        if (reverseHeading >= 360.0f) reverseHeading -= 360.0f;
        if (reverseHeading < 0.0f) reverseHeading += 360.0f;

        float diff = reverseHeading - activeVehicle.angle;
        while (diff > 180.0f) diff -= 360.0f;
        while (diff < -180.0f) diff += 360.0f;
        activeVehicle.angle += diff * 8.0f * deltaTime;

        float moveStep = activeVehicle.speed * deltaTime;
        if (moveStep > dist) moveStep = dist;

        if (dist > 0.001f) {
            activeVehicle.x += (dx / dist) * moveStep;
            activeVehicle.z += (dz / dist) * moveStep;
        }
    } else {
        // FORWARD MODE: Car's nose points toward the target waypoint
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

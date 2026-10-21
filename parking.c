#include "parking.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

ParkingSlot slots[MAX_SLOTS];
int slotCount = 36;
int selectedSlotIndex = -1;
ParkingStats stats;

void initParkingLot(void) {
    slotCount = 36;
    int index = 0;
    
    float xCoords[9] = { -20.0f, -15.0f, -10.0f, -5.0f, 0.0f, 5.0f, 10.0f, 15.0f, 20.0f };

    #define RESET_SLOT_FIELDS(s) \
        (s).parkingDuration = 0; \
        (s).remainingTime = 0; \
        (s).timerActive = 0; \
        (s).timeExpired = 0; \
        (s).isTestTimer = 0; \
        (s).hasCustomer = 0; \
        memset(&(s).customer, 0, sizeof(Person)); \
        (s).reservationRemaining = 0; \
        (s).chargingTime = 0; \
        (s).chargingRemaining = 0;

    // Row A (A01 .. A09) - A09 is RESERVED
    for (int i = 0; i < 9; i++) {
        sprintf(slots[index].id, "A%02d", i + 1);
        slots[index].x = xCoords[i];
        slots[index].y = 0.0f;
        slots[index].z = -14.0f;
        slots[index].angle = 0.0f;
        slots[index].width = 4.2f;
        slots[index].depth = 7.0f;
        slots[index].type = (i < 2) ? TYPE_DISABLED : TYPE_STANDARD;
        slots[index].status = STATUS_AVAILABLE;
        slots[index].carColorIndex = rand() % 6;
        RESET_SLOT_FIELDS(slots[index]);
        index++;
    }

    // Row B (B01 .. B09) - B09 is RESERVED
    for (int i = 0; i < 9; i++) {
        sprintf(slots[index].id, "B%02d", i + 1);
        slots[index].x = xCoords[i];
        slots[index].y = 0.0f;
        slots[index].z = -4.0f;
        slots[index].angle = 180.0f;
        slots[index].width = 4.2f;
        slots[index].depth = 7.0f;
        slots[index].type = (i < 2) ? TYPE_EV : TYPE_STANDARD;
        slots[index].status = STATUS_AVAILABLE;
        slots[index].carColorIndex = rand() % 6;
        RESET_SLOT_FIELDS(slots[index]);
        index++;
    }

    // Row C (C01 .. C09) - C09 is RESERVED
    for (int i = 0; i < 9; i++) {
        sprintf(slots[index].id, "C%02d", i + 1);
        slots[index].x = xCoords[i];
        slots[index].y = 0.0f;
        slots[index].z = 4.0f;
        slots[index].angle = 0.0f;
        slots[index].width = 4.2f;
        slots[index].depth = 7.0f;
        slots[index].type = (i < 2) ? TYPE_EV : TYPE_STANDARD;
        slots[index].status = STATUS_AVAILABLE;
        slots[index].carColorIndex = rand() % 6;
        RESET_SLOT_FIELDS(slots[index]);
        index++;
    }

    // Row D (D01 .. D09) - D09 is RESERVED
    for (int i = 0; i < 9; i++) {
        sprintf(slots[index].id, "D%02d", i + 1);
        slots[index].x = xCoords[i];
        slots[index].y = 0.0f;
        slots[index].z = 14.0f;
        slots[index].angle = 180.0f;
        slots[index].width = 4.2f;
        slots[index].depth = 7.0f;
        slots[index].type = (i < 2) ? TYPE_DISABLED : TYPE_STANDARD;
        slots[index].status = STATUS_AVAILABLE;
        slots[index].carColorIndex = rand() % 6;
        RESET_SLOT_FIELDS(slots[index]);
        index++;
    }

    selectedSlotIndex = 0; // Start at A01
    while (selectedSlotIndex < slotCount && slots[selectedSlotIndex].status != STATUS_AVAILABLE) {
        selectedSlotIndex++;
    }
    if (selectedSlotIndex >= slotCount) selectedSlotIndex = 0;

    updateParkingStats();
}

void updateParkingStats(void) {
    stats.total = slotCount;
    stats.available = 0;
    stats.occupied = 0;
    stats.reserved = 0;
    stats.timeExpired = 0;
    stats.ev = 0;
    stats.disabled = 0;
    stats.activeBookings = 0;

    for (int i = 0; i < slotCount; i++) {
        if (slots[i].type == TYPE_EV) stats.ev++;
        if (slots[i].type == TYPE_DISABLED) stats.disabled++;

        if (slots[i].status == STATUS_AVAILABLE) {
            stats.available++;
        } else if (slots[i].status == STATUS_OCCUPIED) {
            stats.occupied++;
            if (slots[i].hasCustomer) stats.activeBookings++;
        } else if (slots[i].status == STATUS_RESERVED) {
            stats.reserved++;
        } else if (slots[i].status == STATUS_TIME_EXPIRED) {
            stats.timeExpired++;
            if (slots[i].hasCustomer) stats.activeBookings++;
        }
    }
}

int findAvailableSlot(void) {
    for (int i = 0; i < slotCount; i++) {
        if (slots[i].status == STATUS_AVAILABLE) {
            return i;
        }
    }
    return -1;
}

int addVehicleToSlot(int slotIdx) {
    if (slotIdx >= 0 && slotIdx < slotCount) {
        if (slots[slotIdx].status == STATUS_AVAILABLE) {
            slots[slotIdx].status = STATUS_OCCUPIED;
            slots[slotIdx].carColorIndex = rand() % 6;
            updateParkingStats();
            return 1;
        }
    } else {
        int avail = findAvailableSlot();
        if (avail != -1) {
            slots[avail].status = STATUS_OCCUPIED;
            slots[avail].carColorIndex = rand() % 6;
            updateParkingStats();
            return 1;
        }
    }
    return 0;
}

int removeVehicleFromSlot(int slotIdx) {
    if (slotIdx >= 0 && slotIdx < slotCount) {
        if (slots[slotIdx].status == STATUS_OCCUPIED || slots[slotIdx].status == STATUS_TIME_EXPIRED) {
            releaseSlot(slotIdx);
            return 1;
        }
    } else {
        for (int i = slotCount - 1; i >= 0; i--) {
            if (slots[i].status == STATUS_OCCUPIED || slots[i].status == STATUS_TIME_EXPIRED) {
                releaseSlot(i);
                return 1;
            }
        }
    }
    return 0;
}

void releaseSlot(int slotIdx) {
    if (slotIdx < 0 || slotIdx >= slotCount) return;
    slots[slotIdx].status = STATUS_AVAILABLE;
    slots[slotIdx].parkingDuration = 0;
    slots[slotIdx].remainingTime = 0;
    slots[slotIdx].timerActive = 0;
    slots[slotIdx].timeExpired = 0;
    slots[slotIdx].isTestTimer = 0;
    slots[slotIdx].hasCustomer = 0;
    slots[slotIdx].customer.name[0] = '\0';
    slots[slotIdx].customer.phone[0] = '\0';
    slots[slotIdx].customer.vehicleNumber[0] = '\0';
    slots[slotIdx].customer.vehicleType[0] = '\0';
    slots[slotIdx].customer.eligibleAccessible = 0;
    slots[slotIdx].chargingTime = 0;
    slots[slotIdx].chargingRemaining = 0;

    updateParkingStats();
}

void startSlotTimer(int slotIdx, int durationSec, int isTest) {
    if (slotIdx < 0 || slotIdx >= slotCount) return;
    slots[slotIdx].parkingDuration = durationSec;
    slots[slotIdx].remainingTime = durationSec;
    slots[slotIdx].timerActive = 1;
    slots[slotIdx].timeExpired = 0;
    slots[slotIdx].isTestTimer = isTest;
    if (slots[slotIdx].type == TYPE_EV) {
        slots[slotIdx].chargingTime = durationSec;
        slots[slotIdx].chargingRemaining = durationSec;
    }
}

void formatTimeString(int totalSeconds, char* outBuf) {
    if (totalSeconds < 0) totalSeconds = 0;
    int hrs = totalSeconds / 3600;
    int mins = (totalSeconds % 3600) / 60;
    int secs = totalSeconds % 60;
    if (hrs > 0) {
        sprintf(outBuf, "%02d:%02d:%02d", hrs, mins, secs);
    } else {
        sprintf(outBuf, "%02d:%02d", mins, secs);
    }
}

void resetParkingLot(void) {
    initParkingLot();
}

void selectNextSlot(void) {
    if (slotCount == 0) return;
    int start = (selectedSlotIndex == -1) ? 0 : selectedSlotIndex;
    selectedSlotIndex = (start + 1) % slotCount;
}

void selectPrevSlot(void) {
    if (slotCount == 0) return;
    int start = (selectedSlotIndex == -1) ? 0 : selectedSlotIndex;
    selectedSlotIndex = (start - 1 + slotCount) % slotCount;
}

const char* getSlotTypeName(int type) {
    switch (type) {
        case TYPE_DISABLED: return "Disabled / Accessible";
        case TYPE_EV: return "EV Charging Station";
        case TYPE_RESERVED: return "Reserved VIP";
        default: return "Standard Parking";
    }
}

const char* getSlotStatusName(int status) {
    switch (status) {
        case STATUS_OCCUPIED: return "OCCUPIED";
        case STATUS_RESERVED: return "RESERVED";
        case STATUS_TIME_EXPIRED: return "TIME EXPIRED";
        default: return "AVAILABLE";
    }
}

float getSlotDistance(int slotIdx) {
    if (slotIdx < 0 || slotIdx >= slotCount) return 0.0f;
    float aisleZ = (slots[slotIdx].z < 0.0f) ? -9.0f : 9.0f;
    float distEntranceToAisle = fabsf(-26.0f - (-9.0f));
    if (slots[slotIdx].z > 0.0f) distEntranceToAisle += 18.0f;
    float distAlongAisle = fabsf(slots[slotIdx].x - (-30.0f));
    float distIntoSlot = fabsf(slots[slotIdx].z - aisleZ);
    return distEntranceToAisle + distAlongAisle + distIntoSlot;
}

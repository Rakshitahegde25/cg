#ifndef PARKING_H
#define PARKING_H

#define MAX_SLOTS 36

typedef enum {
    STATUS_AVAILABLE = 0,
    STATUS_OCCUPIED = 1,
    STATUS_RESERVED = 2,
    STATUS_TIME_EXPIRED = 3
} SlotStatus;

typedef enum {
    TYPE_STANDARD = 0,
    TYPE_DISABLED = 1,
    TYPE_EV = 2,
    TYPE_RESERVED = 3
} SlotType;

typedef struct {
    char name[50];
    char phone[20];
    char vehicleNumber[20];
    char vehicleType[20]; // Car, Bike, SUV, EV Car
    int eligibleAccessible; // 1 = YES, 0 = NO
} Person;

typedef struct {
    char id[10];           // e.g., "A01", "A05"

    int status;            // STATUS_AVAILABLE, STATUS_OCCUPIED, STATUS_RESERVED, STATUS_TIME_EXPIRED
    int type;              // TYPE_STANDARD, TYPE_DISABLED, TYPE_EV, TYPE_RESERVED

    float x;
    float y;
    float z;
    float angle;
    float width;
    float depth;
    int carColorIndex;

    int parkingDuration;   // Duration in seconds
    int remainingTime;     // Remaining time in seconds
    int timerActive;       // 1 = active timer, 0 = inactive
    int timeExpired;       // 1 = time expired
    int isTestTimer;       // 1 = 30s/60s test timer

    Person customer;
    int hasCustomer;

    int reservationRemaining; // Reservation countdown in seconds
    int chargingTime;         // EV charging total time
    int chargingRemaining;    // EV charging remaining time
} ParkingSlot;

typedef struct {
    int total;
    int available;
    int occupied;
    int reserved;
    int timeExpired;
    int ev;
    int disabled;
    int activeBookings;
} ParkingStats;

extern ParkingSlot slots[MAX_SLOTS];
extern int slotCount;
extern int selectedSlotIndex;
extern ParkingStats stats;

void initParkingLot(void);
void updateParkingStats(void);
int findAvailableSlot(void);
int addVehicleToSlot(int slotIdx);
int removeVehicleFromSlot(int slotIdx);
void resetParkingLot(void);
void selectNextSlot(void);
void selectPrevSlot(void);

void releaseSlot(int slotIdx);
void startSlotTimer(int slotIdx, int durationSec, int isTest);
void formatTimeString(int totalSeconds, char* outBuf);

const char* getSlotTypeName(int type);
const char* getSlotStatusName(int status);
float getSlotDistance(int slotIdx);

#endif

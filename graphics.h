#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include "parking.h"

extern int lightingEnabled;

typedef enum {
    BOOKING_STATE_NONE = 0,
    BOOKING_STATE_ENTER_NAME,
    BOOKING_STATE_ENTER_PHONE,
    BOOKING_STATE_ENTER_VEHICLE_NUM,
    BOOKING_STATE_SELECT_VEHICLE_TYPE,
    BOOKING_STATE_ACCESSIBLE_ELIGIBILITY,
    BOOKING_STATE_SELECT_DURATION,
    BOOKING_STATE_TEST_TIMER_SELECT,
    BOOKING_STATE_CONFIRM_SUMMARY,
    BOOKING_STATE_CUSTOMER_INFO_VIEW
} BookingState;

extern BookingState currentBookingState;
extern Person tempCustomer;
extern int tempDurationSec;
extern int tempIsTestTimer;

void initGraphics(void);
void setupLighting(void);
void toggleLighting(void);

// Basic 3D shapes
void drawCube(float w, float h, float d);
void drawCylinder(float baseRadius, float topRadius, float height, int slices);
void drawSphere(float radius, int slices, int stacks);

// Environmental elements
void drawTree3D(float x, float z);
void drawStreetLight3D(float x, float z);
void drawEVStation3D(float x, float z);
void drawParkingSign3D(float x, float z, const char* label, float r, float g, float b);
void drawGate3D(float x, float z, const char* title, float r, float g, float b);

// Car drawing
void drawCar3D(float r, float g, float b);
void drawCar2D(float r, float g, float b);

// Rendering scenes
void drawEnvironment3D(void);
void drawParkingLot2D(void);

// Text & Dashboard HUD
void drawText2D(float x, float y, const char* str, void* font);
void drawText3D(float x, float y, float z, const char* str, void* font);
void drawDashboardHUD(int is3D, int width, int height);

// Modal UI Dialogs
void drawBookingModal(int winW, int winH);
void drawCustomerInfoModal(int winW, int winH);

// Mouse picking and HUD interaction
int pickSlotWithMouse(int mouseX, int mouseY, int winW, int winH, int is3D);
int handleHUDButtonClick(int mouseX, int mouseY, int winW, int winH);

#endif

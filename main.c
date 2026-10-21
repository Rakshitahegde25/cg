#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parking.h"
#include "camera.h"
#include "navigation.h"
#include "vehicle.h"
#include "graphics.h"

static int is3DMode = 1; // 1 = 3D view, 0 = 2D view
static int winWidth = 1100;
static int winHeight = 780;

static void updateSlotSelectionFeedback(void) {
    if (selectedSlotIndex < 0 || selectedSlotIndex >= slotCount) return;

    ParkingSlot sel = slots[selectedSlotIndex];
    if (sel.status == STATUS_RESERVED || sel.type == TYPE_RESERVED) {
        currentPathCount = 0;
        sprintf(statusFeedback, "This parking slot is RESERVED. You cannot park here.");
    } else if (sel.status == STATUS_OCCUPIED || sel.status == STATUS_TIME_EXPIRED) {
        currentPathCount = 0;
        sprintf(statusFeedback, "This slot is already occupied.");
    } else {
        if (!activeVehicle.isNavigating) {
            calculatePathToSlot(selectedSlotIndex, currentPath, &currentPathCount);
        }
        sprintf(statusFeedback, "Selected Slot %s (Available, %s). Press 'B' to Book.",
                sel.id, getSlotTypeName(sel.type));
    }
}

static void startBookingFlow(void) {
    if (selectedSlotIndex < 0 || selectedSlotIndex >= slotCount) return;

    ParkingSlot sel = slots[selectedSlotIndex];
    if (sel.status == STATUS_RESERVED || sel.type == TYPE_RESERVED) {
        sprintf(statusFeedback, "Navigation unavailable: This slot is reserved.");
        return;
    }

    if (sel.status == STATUS_OCCUPIED || sel.status == STATUS_TIME_EXPIRED) {
        sprintf(statusFeedback, "Navigation unavailable: This slot is already occupied.");
        return;
    }

    // Start booking wizard flow
    currentBookingState = BOOKING_STATE_ENTER_NAME;
    memset(&tempCustomer, 0, sizeof(Person));
    tempCustomer.name[0] = '\0';
    tempCustomer.phone[0] = '\0';
    tempCustomer.vehicleNumber[0] = '\0';
    tempCustomer.vehicleType[0] = '\0';
    tempCustomer.eligibleAccessible = 0;
    tempDurationSec = 3600; // 1 hour default
    tempIsTestTimer = 0;
}

void displayCB(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    applyCamera(is3DMode, winWidth, winHeight);

    if (is3DMode) {
        drawEnvironment3D();
    } else {
        drawParkingLot2D();
    }

    // Draw route path line leading to destination
    drawPath(is3DMode);

    // Draw active moving vehicle
    drawVehicle(is3DMode);

    // Draw HUD text, information panel & statistics overlay
    drawDashboardHUD(is3DMode, winWidth, winHeight);

    // Draw interactive Booking & Customer Info Modals over the window
    drawBookingModal(winWidth, winHeight);
    drawCustomerInfoModal(winWidth, winHeight);

    glutSwapBuffers();
}

void reshapeCB(int w, int h) {
    if (h == 0) h = 1;
    winWidth = w;
    winHeight = h;
    glViewport(0, 0, winWidth, winHeight);
}

void timerCB(int value) {
    float deltaTime = 0.016f; // ~60 FPS update

    updateVehicle(deltaTime);

    glutPostRedisplay();
    glutTimerFunc(16, timerCB, 0);
}

void mouseCB(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        if (!handleHUDButtonClick(x, y, winWidth, winHeight)) {
            pickSlotWithMouse(x, y, winWidth, winHeight, is3DMode);
        }
        glutPostRedisplay();
    }
}

void keyboardCB(unsigned char key, int x, int y) {
    // ---------------------------------------------------------
    // 1. MODAL DIALOG INPUT HANDLING
    // ---------------------------------------------------------
    if (currentBookingState != BOOKING_STATE_NONE) {
        if (key == 27) { // ESC = Cancel modal
            currentBookingState = BOOKING_STATE_NONE;
            sprintf(statusFeedback, "Booking cancelled.");
            glutPostRedisplay();
            return;
        }

        if (currentBookingState == BOOKING_STATE_CUSTOMER_INFO_VIEW) {
            if (key == 'r' || key == 'R') {
                if (selectedSlotIndex >= 0) {
                    releaseSlot(selectedSlotIndex);
                    sprintf(statusFeedback, "Slot %s Released - Status: AVAILABLE", slots[selectedSlotIndex].id);
                }
                currentBookingState = BOOKING_STATE_NONE;
            } else if (key == 'i' || key == 'I') {
                currentBookingState = BOOKING_STATE_NONE;
            }
            glutPostRedisplay();
            return;
        }

        // Text input step 1: Name
        if (currentBookingState == BOOKING_STATE_ENTER_NAME) {
            if (key == 13 || key == 10) { // ENTER -> next step
                if (strlen(tempCustomer.name) == 0) sprintf(tempCustomer.name, "Customer");
                currentBookingState = BOOKING_STATE_ENTER_PHONE;
            } else if (key == 8 || key == 127) { // BACKSPACE
                int len = strlen(tempCustomer.name);
                if (len > 0) tempCustomer.name[len - 1] = '\0';
            } else if (key >= 32 && key <= 126 && strlen(tempCustomer.name) < 45) {
                int len = strlen(tempCustomer.name);
                tempCustomer.name[len] = key;
                tempCustomer.name[len + 1] = '\0';
            }
            glutPostRedisplay();
            return;
        }

        // Text input step 2: Phone
        if (currentBookingState == BOOKING_STATE_ENTER_PHONE) {
            if (key == 13 || key == 10) {
                if (strlen(tempCustomer.phone) == 0) sprintf(tempCustomer.phone, "N/A");
                currentBookingState = BOOKING_STATE_ENTER_VEHICLE_NUM;
            } else if (key == 8 || key == 127) {
                int len = strlen(tempCustomer.phone);
                if (len > 0) tempCustomer.phone[len - 1] = '\0';
            } else if (key >= 32 && key <= 126) {
                if (key >= '0' && key <= '9' && strlen(tempCustomer.phone) < 18) {
                    int len = strlen(tempCustomer.phone);
                    tempCustomer.phone[len] = key;
                    tempCustomer.phone[len + 1] = '\0';
                } else if (key < '0' || key > '9') {
                    MessageBox(NULL, "Please enter numbers only.", "Invalid Input", MB_OK | MB_ICONWARNING);
                }
            }
            glutPostRedisplay();
            return;
        }

        // Text input step 3: Vehicle Number
        if (currentBookingState == BOOKING_STATE_ENTER_VEHICLE_NUM) {
            if (key == 13 || key == 10) {
                if (strlen(tempCustomer.vehicleNumber) == 0) sprintf(tempCustomer.vehicleNumber, "N/A");
                currentBookingState = BOOKING_STATE_SELECT_VEHICLE_TYPE;
            } else if (key == 8 || key == 127) {
                int len = strlen(tempCustomer.vehicleNumber);
                if (len > 0) tempCustomer.vehicleNumber[len - 1] = '\0';
            } else if (key >= 32 && key <= 126 && strlen(tempCustomer.vehicleNumber) < 18) {
                int len = strlen(tempCustomer.vehicleNumber);
                tempCustomer.vehicleNumber[len] = key;
                tempCustomer.vehicleNumber[len + 1] = '\0';
            }
            glutPostRedisplay();
            return;
        }

        // Selection step 4: Vehicle Type
        if (currentBookingState == BOOKING_STATE_SELECT_VEHICLE_TYPE) {
            if (key == '1') sprintf(tempCustomer.vehicleType, "Car");
            else if (key == '2') sprintf(tempCustomer.vehicleType, "Bike");
            else if (key == '3') sprintf(tempCustomer.vehicleType, "SUV");
            else if (key == '4') sprintf(tempCustomer.vehicleType, "EV Car");
            else return;

            // If slot is Disabled/Accessible, ask for eligibility check
            if (slots[selectedSlotIndex].type == TYPE_DISABLED) {
                currentBookingState = BOOKING_STATE_ACCESSIBLE_ELIGIBILITY;
            } else {
                currentBookingState = BOOKING_STATE_SELECT_DURATION;
            }
            glutPostRedisplay();
            return;
        }

        // Selection step 5: Accessible Parking Eligibility Check
        if (currentBookingState == BOOKING_STATE_ACCESSIBLE_ELIGIBILITY) {
            if (key == '1') {
                tempCustomer.eligibleAccessible = 1;
                currentBookingState = BOOKING_STATE_SELECT_DURATION;
            } else if (key == '2') {
                tempCustomer.eligibleAccessible = 0;
                currentBookingState = BOOKING_STATE_NONE;
                sprintf(statusFeedback, "Normal vehicles cannot book accessible slots!");
            }
            glutPostRedisplay();
            return;
        }

        // Selection step 6: Duration
        if (currentBookingState == BOOKING_STATE_SELECT_DURATION) {
            if (key == '1') { tempDurationSec = 1800; tempIsTestTimer = 0; currentBookingState = BOOKING_STATE_CONFIRM_SUMMARY; }      // 30 min
            else if (key == '2') { tempDurationSec = 3600; tempIsTestTimer = 0; currentBookingState = BOOKING_STATE_CONFIRM_SUMMARY; } // 1 hour
            else if (key == '3') { tempDurationSec = 7200; tempIsTestTimer = 0; currentBookingState = BOOKING_STATE_CONFIRM_SUMMARY; } // 2 hours
            else if (key == '4') { tempDurationSec = 10800; tempIsTestTimer = 0; currentBookingState = BOOKING_STATE_CONFIRM_SUMMARY; }// 3 hours
            else if (key == '5') { tempDurationSec = 18000; tempIsTestTimer = 0; currentBookingState = BOOKING_STATE_CONFIRM_SUMMARY; }// 5 hours
            else if (key == 't' || key == 'T') { currentBookingState = BOOKING_STATE_TEST_TIMER_SELECT; }
            glutPostRedisplay();
            return;
        }

        // Selection step 7: Test Timer
        if (currentBookingState == BOOKING_STATE_TEST_TIMER_SELECT) {
            if (key == '1') { tempDurationSec = 30; tempIsTestTimer = 1; currentBookingState = BOOKING_STATE_CONFIRM_SUMMARY; }       // 30 sec test
            else if (key == '2') { tempDurationSec = 60; tempIsTestTimer = 1; currentBookingState = BOOKING_STATE_CONFIRM_SUMMARY; }  // 60 sec test
            glutPostRedisplay();
            return;
        }

        // Confirmation step 8: Booking Summary
        if (currentBookingState == BOOKING_STATE_CONFIRM_SUMMARY) {
            if (key == 13 || key == 10) { // ENTER = Confirm Booking
                int idx = selectedSlotIndex;
                slots[idx].customer = tempCustomer;
                slots[idx].hasCustomer = 1;
                slots[idx].parkingDuration = tempDurationSec;
                slots[idx].isTestTimer = tempIsTestTimer;

                currentBookingState = BOOKING_STATE_NONE;

                // Launch A* navigation for confirmed booking!
                startVehicleNavigation(idx);
            }
            glutPostRedisplay();
            return;
        }
    }

    // ---------------------------------------------------------
    // 2. NORMAL VIEW KEYBOARD SHORTCUTS
    // ---------------------------------------------------------
    switch (key) {
        case '2':
            is3DMode = 0;
            break;
        case '3':
            is3DMode = 1;
            break;
        case 'b':
        case 'B':
            startBookingFlow();
            break;
        case 'n':
        case 'N':
            if (selectedSlotIndex >= 0) {
                if (slots[selectedSlotIndex].status == STATUS_AVAILABLE) {
                    startBookingFlow();
                } else {
                    startVehicleNavigation(selectedSlotIndex);
                }
            }
            break;
        case 'i':
        case 'I':
            if (selectedSlotIndex >= 0) {
                currentBookingState = BOOKING_STATE_CUSTOMER_INFO_VIEW;
            }
            break;
        case 'r':
        case 'R':
            if (selectedSlotIndex >= 0 && (slots[selectedSlotIndex].status == STATUS_OCCUPIED || slots[selectedSlotIndex].status == STATUS_TIME_EXPIRED)) {
                releaseSlot(selectedSlotIndex);
                sprintf(statusFeedback, "Slot %s Released - Status: AVAILABLE", slots[selectedSlotIndex].id);
            } else {
                resetParkingLot();
                initVehicle();
                currentPathCount = 0;
                resetCamera();
                updateSlotSelectionFeedback();
            }
            break;
        case 'p':
        case 'P':
            togglePauseVehicleNavigation();
            break;
        case 't':
        case 'T':
            if (selectedSlotIndex >= 0 && slots[selectedSlotIndex].status == STATUS_AVAILABLE) {
                startBookingFlow();
                currentBookingState = BOOKING_STATE_TEST_TIMER_SELECT;
            }
            break;
        case 'c':
        case 'C':
            resetVehicleNavigation();
            break;
        case 'a':
        case 'A':
            addVehicleToSlot(-1);
            break;
        case 'd':
        case 'D':
            removeVehicleFromSlot(-1);
            break;
        case 'l':
        case 'L':
            toggleLighting();
            break;
        case 'w':
        case 'W':
            moveCamera(1.5f, 0.0f, 0.0f); // Zoom in
            break;
        case 's':
        case 'S':
            moveCamera(-1.5f, 0.0f, 0.0f); // Zoom out
            break;
        case 'q':
        case 'Q':
            moveCamera(0.0f, 0.0f, 1.0f); // Camera height up
            break;
        case 'e':
        case 'E':
            moveCamera(0.0f, 0.0f, -1.0f); // Camera height down
            break;
        case '\t': // Tab key cycles slot
            selectNextSlot();
            updateSlotSelectionFeedback();
            break;
        case 27: // ESC key exit
            exit(0);
            break;
        default:
            break;
    }
    glutPostRedisplay();
}

void specialCB(int key, int x, int y) {
    if (currentBookingState != BOOKING_STATE_NONE) return;

    switch (key) {
        case GLUT_KEY_LEFT:
            if (is3DMode) rotateCamera(-3.0f, 0.0f);
            else {
                selectPrevSlot();
                updateSlotSelectionFeedback();
            }
            break;
        case GLUT_KEY_RIGHT:
            if (is3DMode) rotateCamera(3.0f, 0.0f);
            else {
                selectNextSlot();
                updateSlotSelectionFeedback();
            }
            break;
        case GLUT_KEY_UP:
            if (is3DMode) rotateCamera(0.0f, 3.0f);
            else {
                selectPrevSlot();
                updateSlotSelectionFeedback();
            }
            break;
        case GLUT_KEY_DOWN:
            if (is3DMode) rotateCamera(0.0f, -3.0f);
            else {
                selectNextSlot();
                updateSlotSelectionFeedback();
            }
            break;
        default:
            break;
    }
    glutPostRedisplay();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(winWidth, winHeight);
    glutInitWindowPosition(80, 60);
    glutCreateWindow("Interactive 2D/3D Smart Parking Visualization and Navigation System");

    initGraphics();
    initCamera();
    initParkingLot();
    initNavigationNetwork();
    initVehicle();

    updateSlotSelectionFeedback();

    glutDisplayFunc(displayCB);
    glutReshapeFunc(reshapeCB);
    glutKeyboardFunc(keyboardCB);
    glutSpecialFunc(specialCB);
    glutMouseFunc(mouseCB);
    glutTimerFunc(16, timerCB, 0);

    // Per-second countdown timer for parking, EV charging and reservations
    glutTimerFunc(1000, perSecondTimerCB, 0);

    printf("====================================================\n");
    printf(" Smart Parking System - Full Customer Booking & Timer System Loaded!\n");
    printf(" Controls: \n");
    printf("   'B' -> Book Selected Slot (Person & Vehicle Details)\n");
    printf("   'I' -> View Customer / Booking Information\n");
    printf("   'R' -> Release Slot (Clears Car & Resets Timer)\n");
    printf("   'T' -> Test Timer (30s / 60s for Lab Demonstration)\n");
    printf("   'P' -> Pause / Resume Timers\n");
    printf("   '2' -> 2D View | '3' -> 3D View | 'N' -> Navigate\n");
    printf("====================================================\n");

    glutMainLoop();
    return 0;
}

#include "graphics.h"
#include "parking.h"
#include "navigation.h"
#include "vehicle.h"
#include "camera.h"
#include <stdio.h>
#include <math.h>
#include <string.h>

int lightingEnabled = 1;

BookingState currentBookingState = BOOKING_STATE_NONE;
Person tempCustomer;
int tempDurationSec = 7200; // Default 2 hours
int tempIsTestTimer = 0;

static float carColors[6][3] = {
    {0.90f, 0.15f, 0.15f}, // Crimson Red
    {0.15f, 0.40f, 0.85f}, // Royal Blue
    {0.30f, 0.35f, 0.40f}, // Slate Gray
    {0.95f, 0.65f, 0.10f}, // Amber Gold
    {0.10f, 0.65f, 0.30f}, // Emerald Green
    {0.92f, 0.92f, 0.95f}  // Pearl White
};

void initGraphics(void) {
    glClearColor(0.12f, 0.15f, 0.18f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);
    setupLighting();
}

void setupLighting(void) {
    if (lightingEnabled) {
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
        glEnable(GL_COLOR_MATERIAL);
        glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

        GLfloat lightPos[] = { 20.0f, 40.0f, 20.0f, 1.0f };
        GLfloat lightAmbient[] = { 0.35f, 0.35f, 0.38f, 1.0f };
        GLfloat lightDiffuse[] = { 0.85f, 0.85f, 0.80f, 1.0f };
        GLfloat lightSpecular[] = { 0.6f, 0.6f, 0.6f, 1.0f };

        glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
        glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
        glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);
    } else {
        glDisable(GL_LIGHTING);
    }
}

void toggleLighting(void) {
    lightingEnabled = !lightingEnabled;
    setupLighting();
}

// -------------------------------------------------------------
// BASIC PRIMITIVES
// -------------------------------------------------------------
void drawCube(float w, float h, float d) {
    float hw = w * 0.5f;
    float hh = h * 0.5f;
    float hd = d * 0.5f;

    glBegin(GL_QUADS);
    glNormal3f(0.0f, 0.0f, 1.0f);
    glVertex3f(-hw, -hh, hd);
    glVertex3f( hw, -hh, hd);
    glVertex3f( hw,  hh, hd);
    glVertex3f(-hw,  hh, hd);

    glNormal3f(0.0f, 0.0f, -1.0f);
    glVertex3f(-hw, -hh, -hd);
    glVertex3f(-hw,  hh, -hd);
    glVertex3f( hw,  hh, -hd);
    glVertex3f( hw, -hh, -hd);

    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(-hw, hh, -hd);
    glVertex3f(-hw, hh,  hd);
    glVertex3f( hw, hh,  hd);
    glVertex3f( hw, hh, -hd);

    glNormal3f(0.0f, -1.0f, 0.0f);
    glVertex3f(-hw, -hh, -hd);
    glVertex3f( hw, -hh, -hd);
    glVertex3f( hw, -hh,  hd);
    glVertex3f(-hw, -hh,  hd);

    glNormal3f(1.0f, 0.0f, 0.0f);
    glVertex3f(hw, -hh, -hd);
    glVertex3f(hw,  hh, -hd);
    glVertex3f(hw,  hh,  hd);
    glVertex3f(hw, -hh,  hd);

    glNormal3f(-1.0f, 0.0f, 0.0f);
    glVertex3f(-hw, -hh, -hd);
    glVertex3f(-hw, -hh,  hd);
    glVertex3f(-hw,  hh,  hd);
    glVertex3f(-hw,  hh, -hd);
    glEnd();
}

void drawCylinder(float baseRadius, float topRadius, float height, int slices) {
    GLUquadricObj* quad = gluNewQuadric();
    gluQuadricDrawStyle(quad, GLU_FILL);
    gluQuadricNormals(quad, GLU_SMOOTH);
    
    glPushMatrix();
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    gluCylinder(quad, baseRadius, topRadius, height, slices, 1);
    gluDeleteQuadric(quad);
    glPopMatrix();
}

void drawSphere(float radius, int slices, int stacks) {
    GLUquadricObj* quad = gluNewQuadric();
    gluQuadricDrawStyle(quad, GLU_FILL);
    gluQuadricNormals(quad, GLU_SMOOTH);
    gluSphere(quad, radius, slices, stacks);
    gluDeleteQuadric(quad);
}

// -------------------------------------------------------------
// TEXT RENDERING
// -------------------------------------------------------------
void drawText2D(float x, float y, const char* str, void* font) {
    glRasterPos2f(x, y);
    for (const char* c = str; *c != '\0'; c++) {
        glutBitmapCharacter(font, *c);
    }
}

void drawText3D(float x, float y, float z, const char* str, void* font) {
    glRasterPos3f(x, y, z);
    for (const char* c = str; *c != '\0'; c++) {
        glutBitmapCharacter(font, *c);
    }
}

// -------------------------------------------------------------
// ENVIRONMENT OBJECTS
// -------------------------------------------------------------
void drawTree3D(float x, float z) {
    glPushMatrix();
    glTranslatef(x, 0.0f, z);

    glColor3f(0.40f, 0.25f, 0.15f);
    drawCylinder(0.4f, 0.3f, 3.5f, 12);

    glColor3f(0.12f, 0.55f, 0.20f);
    glPushMatrix();
    glTranslatef(0.0f, 3.0f, 0.0f);
    drawSphere(1.8f, 12, 12);
    glTranslatef(0.0f, 1.2f, 0.0f);
    glColor3f(0.18f, 0.68f, 0.28f);
    drawSphere(1.3f, 12, 12);
    glPopMatrix();

    glPopMatrix();
}

void drawStreetLight3D(float x, float z) {
    glPushMatrix();
    glTranslatef(x, 0.0f, z);

    glColor3f(0.3f, 0.3f, 0.35f);
    drawCylinder(0.2f, 0.15f, 6.0f, 12);

    glPushMatrix();
    glTranslatef(0.0f, 6.0f, 0.0f);
    drawCube(0.3f, 0.2f, 1.0f);
    
    glTranslatef(0.0f, -0.2f, 0.4f);
    if (!lightingEnabled) glColor3f(1.0f, 1.0f, 0.5f);
    else glColor3f(1.0f, 0.95f, 0.7f);
    drawSphere(0.4f, 10, 10);
    glPopMatrix();

    glPopMatrix();
}

void drawEVStation3D(float x, float z) {
    glPushMatrix();
    glTranslatef(x, 0.0f, z);

    glColor3f(0.2f, 0.25f, 0.3f);
    glPushMatrix();
    glTranslatef(0.0f, 2.0f, 0.0f);
    drawCube(1.2f, 4.0f, 1.0f);

    glColor3f(0.0f, 0.8f, 1.0f);
    glTranslatef(0.0f, 0.8f, 0.51f);
    drawCube(0.8f, 0.8f, 0.05f);

    glColor3f(0.2f, 0.9f, 0.3f);
    glTranslatef(0.0f, -1.2f, 0.0f);
    drawCube(1.0f, 0.2f, 0.05f);
    glPopMatrix();

    glPopMatrix();
}

void drawParkingSign3D(float x, float z, const char* label, float r, float g, float b) {
    glPushMatrix();
    glTranslatef(x, 0.0f, z);

    glColor3f(0.5f, 0.5f, 0.55f);
    drawCylinder(0.12f, 0.12f, 4.5f, 10);

    glPushMatrix();
    glTranslatef(0.0f, 4.0f, 0.0f);
    glColor3f(r, g, b);
    drawCube(2.2f, 1.2f, 0.15f);
    
    glColor3f(1.0f, 1.0f, 1.0f);
    drawText3D(-0.7f, -0.2f, 0.1f, label, GLUT_BITMAP_HELVETICA_18);
    glPopMatrix();

    glPopMatrix();
}

void drawGate3D(float x, float z, const char* title, float r, float g, float b) {
    glPushMatrix();
    glTranslatef(x, 0.0f, z);

    glColor3f(0.25f, 0.28f, 0.32f);
    glPushMatrix();
    glTranslatef(-3.5f, 2.5f, 0.0f);
    drawCube(0.8f, 5.0f, 0.8f);
    glTranslatef(7.0f, 0.0f, 0.0f);
    drawCube(0.8f, 5.0f, 0.8f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.0f, 5.2f, 0.0f);
    glColor3f(r, g, b);
    drawCube(8.0f, 1.0f, 0.9f);
    
    glColor3f(1.0f, 1.0f, 1.0f);
    drawText3D(-2.5f, -0.2f, 0.5f, title, GLUT_BITMAP_HELVETICA_18);
    glPopMatrix();

    glPopMatrix();
}

// -------------------------------------------------------------
// CAR RENDERING (3D & 2D)
// -------------------------------------------------------------
void drawCar3D(float r, float g, float b) {
    glPushMatrix();

    glColor3f(r, g, b);
    glPushMatrix();
    glTranslatef(0.0f, 0.7f, 0.0f);
    drawCube(2.4f, 0.8f, 4.8f);

    glColor3f(r * 0.85f, g * 0.85f, b * 0.85f);
    glTranslatef(0.0f, 0.7f, -0.2f);
    drawCube(2.0f, 0.7f, 2.6f);

    glColor3f(0.15f, 0.2f, 0.3f);
    glTranslatef(0.0f, 0.05f, 0.0f);
    drawCube(2.05f, 0.55f, 2.4f);
    glPopMatrix();

    glColor3f(0.15f, 0.15f, 0.15f);
    float wheelOffsetsX[4] = { -1.25f, 1.25f, -1.25f, 1.25f };
    float wheelOffsetsZ[4] = {  1.4f,  1.4f, -1.4f, -1.4f };

    for (int i = 0; i < 4; i++) {
        glPushMatrix();
        glTranslatef(wheelOffsetsX[i], 0.4f, wheelOffsetsZ[i]);
        glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
        drawCylinder(0.4f, 0.4f, 0.3f, 12);
        glPopMatrix();
    }

    glColor3f(1.0f, 1.0f, 0.8f);
    glPushMatrix();
    glTranslatef(-0.8f, 0.7f, 2.42f);
    drawCube(0.4f, 0.25f, 0.05f);
    glTranslatef(1.6f, 0.0f, 0.0f);
    drawCube(0.4f, 0.25f, 0.05f);
    glPopMatrix();

    glColor3f(1.0f, 0.1f, 0.1f);
    glPushMatrix();
    glTranslatef(-0.8f, 0.7f, -2.42f);
    drawCube(0.4f, 0.25f, 0.05f);
    glTranslatef(1.6f, 0.0f, 0.0f);
    drawCube(0.4f, 0.25f, 0.05f);
    glPopMatrix();

    glPopMatrix();
}

void drawCar2D(float r, float g, float b) {
    glPushMatrix();

    glColor3f(r, g, b);
    glBegin(GL_QUADS);
    glVertex3f(-1.1f, 0.0f,  2.1f);
    glVertex3f( 1.1f, 0.0f,  2.1f);
    glVertex3f( 1.1f, 0.0f, -2.1f);
    glVertex3f(-1.1f, 0.0f, -2.1f);
    glEnd();

    glColor3f(0.15f, 0.2f, 0.3f);
    glBegin(GL_QUADS);
    glVertex3f(-0.9f, 0.05f,  1.1f);
    glVertex3f( 0.9f, 0.05f,  1.1f);
    glVertex3f( 0.9f, 0.05f, -1.1f);
    glVertex3f(-0.9f, 0.05f, -1.1f);
    glEnd();

    glColor3f(1.0f, 0.9f, 0.2f);
    glBegin(GL_TRIANGLES);
    glVertex3f( 0.0f, 0.1f,  2.4f);
    glVertex3f(-0.5f, 0.1f,  1.9f);
    glVertex3f( 0.5f, 0.1f,  1.9f);
    glEnd();

    glPopMatrix();
}

// -------------------------------------------------------------
// 3D ENVIRONMENT RENDER
// -------------------------------------------------------------
void drawEnvironment3D(void) {
    glDisable(GL_LIGHTING);
    glColor3f(0.15f, 0.45f, 0.22f);
    glBegin(GL_QUADS);
    glVertex3f(-60.0f, -0.05f, -60.0f);
    glVertex3f( 60.0f, -0.05f, -60.0f);
    glVertex3f( 60.0f, -0.05f,  60.0f);
    glVertex3f(-60.0f, -0.05f,  60.0f);
    glEnd();

    glColor3f(0.22f, 0.24f, 0.26f);
    glBegin(GL_QUADS);
    glVertex3f(-34.0f, 0.0f, -30.0f);
    glVertex3f( 34.0f, 0.0f, -30.0f);
    glVertex3f( 34.0f, 0.0f,  26.0f);
    glVertex3f(-34.0f, 0.0f,  26.0f);
    glEnd();

    if (lightingEnabled) glEnable(GL_LIGHTING);

    drawGate3D(-30.0f, -26.0f, "ENTRANCE", 0.1f, 0.6f, 0.2f);
    drawGate3D( 30.0f, -26.0f, "EXIT", 0.8f, 0.2f, 0.2f);

    drawEVStation3D(-25.0f, -4.0f);
    drawParkingSign3D(-25.0f, -14.0f, "PARKING A", 0.2f, 0.5f, 0.8f);
    drawParkingSign3D(-25.0f,  14.0f, "EV CHARGE", 0.6f, 0.2f, 0.8f);

    drawStreetLight3D(-32.0f, -18.0f);
    drawStreetLight3D(-32.0f,   0.0f);
    drawStreetLight3D(-32.0f,  18.0f);
    drawStreetLight3D( 32.0f, -18.0f);
    drawStreetLight3D( 32.0f,   0.0f);
    drawStreetLight3D( 32.0f,  18.0f);

    drawTree3D(-38.0f, -25.0f);
    drawTree3D(-38.0f, -10.0f);
    drawTree3D(-38.0f,   5.0f);
    drawTree3D(-38.0f,  20.0f);
    drawTree3D( 38.0f, -25.0f);
    drawTree3D( 38.0f, -10.0f);
    drawTree3D( 38.0f,   5.0f);
    drawTree3D( 38.0f,  20.0f);
    drawTree3D(  0.0f,  30.0f);

    for (int i = 0; i < slotCount; i++) {
        ParkingSlot slot = slots[i];

        glPushMatrix();
        glTranslatef(slot.x, slot.y, slot.z);

        float sr = 0.2f, sg = 0.8f, sb = 0.3f; // Green
        if (slot.type == TYPE_DISABLED) { sr = 0.1f; sg = 0.4f; sb = 0.9f; }      // Blue
        else if (slot.type == TYPE_EV) { sr = 0.7f; sg = 0.1f; sb = 0.9f; }       // Purple
        
        if (slot.status == STATUS_RESERVED || slot.type == TYPE_RESERVED) {
            sr = 0.95f; sg = 0.85f; sb = 0.1f; // Yellow
        } else if (slot.status == STATUS_TIME_EXPIRED || slot.timeExpired) {
            sr = 1.0f; sg = 0.2f; sb = 0.6f; // Magenta
        } else if (slot.status == STATUS_OCCUPIED) {
            sr = 0.85f; sg = 0.15f; sb = 0.15f; // Red
        }

        if (i == selectedSlotIndex) {
            glDisable(GL_LIGHTING);
            if (slot.status == STATUS_TIME_EXPIRED || slot.timeExpired) {
                glColor3f(1.0f, 0.1f, 0.8f);
            } else if (slot.status == STATUS_RESERVED || slot.type == TYPE_RESERVED) {
                glColor3f(1.0f, 0.9f, 0.0f);
            } else if (slot.status == STATUS_AVAILABLE) {
                glColor3f(1.0f, 1.0f, 0.0f);
            } else {
                glColor3f(1.0f, 0.2f, 0.2f);
            }
            glLineWidth(4.0f);
            glBegin(GL_LINE_LOOP);
            glVertex3f(-2.2f, 0.08f, -3.7f);
            glVertex3f( 2.2f, 0.08f, -3.7f);
            glVertex3f( 2.2f, 0.08f,  3.7f);
            glVertex3f(-2.2f, 0.08f,  3.7f);
            glEnd();
            if (lightingEnabled) glEnable(GL_LIGHTING);
        }

        glColor3f(sr * 0.8f, sg * 0.8f, sb * 0.8f);
        glBegin(GL_QUADS);
        glVertex3f(-2.0f, 0.02f, -3.5f);
        glVertex3f( 2.0f, 0.02f, -3.5f);
        glVertex3f( 2.0f, 0.02f,  3.5f);
        glVertex3f(-2.0f, 0.02f,  3.5f);
        glEnd();

        // Draw border outline for non-selected slots
        if (i != selectedSlotIndex) {
            glDisable(GL_LIGHTING);
            glColor3f(0.8f, 0.8f, 0.8f);
            glLineWidth(1.5f);
            glBegin(GL_LINE_LOOP);
            glVertex3f(-2.0f, 0.04f, -3.5f);
            glVertex3f( 2.0f, 0.04f, -3.5f);
            glVertex3f( 2.0f, 0.04f,  3.5f);
            glVertex3f(-2.0f, 0.04f,  3.5f);
            glEnd();
            if (lightingEnabled) glEnable(GL_LIGHTING);
        }

        glDisable(GL_LIGHTING);
        glColor3f(1.0f, 1.0f, 1.0f);
        drawText3D(-0.7f, 0.12f, -2.8f, slot.id, GLUT_BITMAP_HELVETICA_12);
        // Show type label for special slots
        if (slot.type == TYPE_DISABLED) {
            glColor3f(0.4f, 0.7f, 1.0f);
            drawText3D(-0.5f, 0.12f, -1.5f, "DIS", GLUT_BITMAP_HELVETICA_10);
        } else if (slot.type == TYPE_EV) {
            glColor3f(0.8f, 0.3f, 1.0f);
            drawText3D(-0.3f, 0.12f, -1.5f, "EV", GLUT_BITMAP_HELVETICA_10);
        }
        if (lightingEnabled) glEnable(GL_LIGHTING);

        if (slot.status == STATUS_OCCUPIED || slot.status == STATUS_TIME_EXPIRED) {
            glRotatef(slot.angle, 0.0f, 1.0f, 0.0f);
            float* cc = carColors[slot.carColorIndex];
            drawCar3D(cc[0], cc[1], cc[2]);
        }

        glPopMatrix();
    }
}

// -------------------------------------------------------------
// 2D TOP-DOWN MAP RENDER
// -------------------------------------------------------------
void drawParkingLot2D(void) {
    glDisable(GL_LIGHTING);

    glColor3f(0.18f, 0.20f, 0.22f);
    glBegin(GL_QUADS);
    glVertex3f(-34.0f, 0.0f, -30.0f);
    glVertex3f( 34.0f, 0.0f, -30.0f);
    glVertex3f( 34.0f, 0.0f,  26.0f);
    glVertex3f(-34.0f, 0.0f,  26.0f);
    glEnd();

    glColor3f(0.9f, 0.9f, 0.9f);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    for (float x = -28.0f; x <= 28.0f; x += 4.0f) {
        glVertex3f(x, 0.05f, -9.0f);
        glVertex3f(x + 2.0f, 0.05f, -9.0f);
    }
    for (float x = -28.0f; x <= 28.0f; x += 4.0f) {
        glVertex3f(x, 0.05f, 9.0f);
        glVertex3f(x + 2.0f, 0.05f, 9.0f);
    }
    glEnd();

    glColor3f(0.2f, 0.8f, 0.3f);
    drawText3D(-32.0f, 0.1f, -27.0f, "ENTRANCE", GLUT_BITMAP_HELVETICA_12);
    glColor3f(0.9f, 0.3f, 0.3f);
    drawText3D( 26.0f, 0.1f, -27.0f, "EXIT", GLUT_BITMAP_HELVETICA_12);

    for (int i = 0; i < slotCount; i++) {
        ParkingSlot slot = slots[i];

        glPushMatrix();
        glTranslatef(slot.x, 0.0f, slot.z);

        float r = 0.1f, g = 0.7f, b = 0.2f;
        if (slot.type == TYPE_DISABLED) { r = 0.15f; g = 0.45f; b = 0.95f; }
        else if (slot.type == TYPE_EV) { r = 0.75f; g = 0.15f; b = 0.85f; }
        
        if (slot.status == STATUS_RESERVED || slot.type == TYPE_RESERVED) {
            r = 0.95f; g = 0.85f; b = 0.1f;
        } else if (slot.status == STATUS_TIME_EXPIRED || slot.timeExpired) {
            r = 1.0f; g = 0.2f; b = 0.6f;
        } else if (slot.status == STATUS_OCCUPIED) {
            r = 0.85f; g = 0.15f; b = 0.15f;
        }

        glColor3f(r, g, b);
        glBegin(GL_QUADS);
        glVertex3f(-2.0f, 0.02f, -3.5f);
        glVertex3f( 2.0f, 0.02f, -3.5f);
        glVertex3f( 2.0f, 0.02f,  3.5f);
        glVertex3f(-2.0f, 0.02f,  3.5f);
        glEnd();

        if (i == selectedSlotIndex) {
            glColor3f(1.0f, 1.0f, 0.0f);
            glLineWidth(4.0f);
        } else {
            glColor3f(1.0f, 1.0f, 1.0f);
            glLineWidth(1.5f);
        }
        glBegin(GL_LINE_LOOP);
        glVertex3f(-2.0f, 0.03f, -3.5f);
        glVertex3f( 2.0f, 0.03f, -3.5f);
        glVertex3f( 2.0f, 0.03f,  3.5f);
        glVertex3f(-2.0f, 0.03f,  3.5f);
        glEnd();

        glColor3f(1.0f, 1.0f, 1.0f);
        drawText3D(-0.7f, 0.1f, -0.2f, slot.id, GLUT_BITMAP_HELVETICA_12);
        // Show type label for special slots in 2D
        if (slot.type == TYPE_DISABLED) {
            glColor3f(0.4f, 0.7f, 1.0f);
            drawText3D(-0.5f, 0.1f, 1.2f, "DIS", GLUT_BITMAP_HELVETICA_10);
        } else if (slot.type == TYPE_EV) {
            glColor3f(0.8f, 0.3f, 1.0f);
            drawText3D(-0.3f, 0.1f, 1.2f, "EV", GLUT_BITMAP_HELVETICA_10);
        }

        if (slot.status == STATUS_OCCUPIED || slot.status == STATUS_TIME_EXPIRED) {
            glRotatef(slot.angle, 0.0f, 1.0f, 0.0f);
            float* cc = carColors[slot.carColorIndex];
            drawCar2D(cc[0], cc[1], cc[2]);
        }

        glPopMatrix();
    }

    glColor3f(0.1f, 0.6f, 0.2f);
    float treePos[8][2] = {
        {-38.0f, -25.0f}, {-38.0f, -10.0f}, {-38.0f, 5.0f}, {-38.0f, 20.0f},
        { 38.0f, -25.0f}, { 38.0f, -10.0f}, { 38.0f, 5.0f}, { 38.0f, 20.0f}
    };
    for (int t = 0; t < 8; t++) {
        glPushMatrix();
        glTranslatef(treePos[t][0], 0.1f, treePos[t][1]);
        glBegin(GL_POLYGON);
        for (int a = 0; a < 360; a += 30) {
            float rad = a * 3.14159f / 180.0f;
            glVertex3f(cosf(rad) * 1.5f, 0.0f, sinf(rad) * 1.5f);
        }
        glEnd();
        glPopMatrix();
    }
}

// -------------------------------------------------------------
// DASHBOARD HUD OVERLAY & ACTION BUTTONS
// -------------------------------------------------------------
void drawDashboardHUD(int is3D, int width, int height) {
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, width, 0, height);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    char buf[256];

    // 1. Top-Left Statistics Panel
    glColor4f(0.08f, 0.12f, 0.18f, 0.88f);
    glBegin(GL_QUADS);
    glVertex2i(15, height - 15);
    glVertex2i(370, height - 15);
    glVertex2i(370, height - 180);
    glVertex2i(15, height - 180);
    glEnd();

    glColor3f(0.2f, 0.6f, 0.9f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2i(15, height - 15);
    glVertex2i(370, height - 15);
    glVertex2i(370, height - 180);
    glVertex2i(15, height - 180);
    glEnd();

    glColor3f(1.0f, 0.9f, 0.2f);
    drawText2D(28, height - 36, "SMART PARKING DASHBOARD", GLUT_BITMAP_HELVETICA_18);

    sprintf(buf, "TOTAL SLOTS     : %d", stats.total);
    glColor3f(1.0f, 1.0f, 1.0f);
    drawText2D(28, height - 56, buf, GLUT_BITMAP_HELVETICA_12);

    sprintf(buf, "AVAILABLE       : %d (GREEN)", stats.available);
    glColor3f(0.2f, 0.95f, 0.3f);
    drawText2D(28, height - 74, buf, GLUT_BITMAP_HELVETICA_12);

    sprintf(buf, "OCCUPIED        : %d (RED)", stats.occupied);
    glColor3f(0.95f, 0.3f, 0.3f);
    drawText2D(28, height - 92, buf, GLUT_BITMAP_HELVETICA_12);

    sprintf(buf, "RESERVED        : %d (YELLOW)", stats.reserved);
    glColor3f(0.95f, 0.85f, 0.2f);
    drawText2D(28, height - 110, buf, GLUT_BITMAP_HELVETICA_12);

    sprintf(buf, "TIME EXPIRED    : %d (MAGENTA)", stats.timeExpired);
    glColor3f(1.0f, 0.3f, 0.7f);
    drawText2D(28, height - 128, buf, GLUT_BITMAP_HELVETICA_12);

    sprintf(buf, "ACTIVE BOOKINGS : %d", stats.activeBookings);
    glColor3f(0.2f, 0.95f, 1.0f);
    drawText2D(28, height - 164, buf, GLUT_BITMAP_HELVETICA_12);

    // 2. Top-Right SELECTED SLOT INFORMATION & BOOKING PANEL
    int panelW = 410;
    int panelH = 205;
    int px1 = width - panelW - 15;
    int py1 = height - 15 - panelH;
    int px2 = width - 15;
    int py2 = height - 15;

    glColor4f(0.08f, 0.12f, 0.18f, 0.90f);
    glBegin(GL_QUADS);
    glVertex2i(px1, py2);
    glVertex2i(px2, py2);
    glVertex2i(px2, py1);
    glVertex2i(px1, py1);
    glEnd();

    glColor3f(0.2f, 0.6f, 0.9f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2i(px1, py2);
    glVertex2i(px2, py2);
    glVertex2i(px2, py1);
    glVertex2i(px1, py1);
    glEnd();

    glColor3f(1.0f, 0.9f, 0.2f);
    drawText2D(px1 + 15, py2 - 24, "SELECTED SLOT DETAILS", GLUT_BITMAP_HELVETICA_18);

    if (selectedSlotIndex >= 0 && selectedSlotIndex < slotCount) {
        ParkingSlot sel = slots[selectedSlotIndex];
        
        sprintf(buf, "Slot ID        : %s  (%s)", sel.id, getSlotTypeName(sel.type));
        glColor3f(1.0f, 1.0f, 1.0f);
        drawText2D(px1 + 15, py2 - 46, buf, GLUT_BITMAP_HELVETICA_12);

        sprintf(buf, "Status         : %s", getSlotStatusName(sel.status));
        if (sel.status == STATUS_RESERVED || sel.type == TYPE_RESERVED) glColor3f(1.0f, 0.9f, 0.1f);
        else if (sel.status == STATUS_AVAILABLE) glColor3f(0.2f, 0.95f, 0.3f);
        else if (sel.status == STATUS_TIME_EXPIRED) glColor3f(1.0f, 0.3f, 0.7f);
        else glColor3f(0.95f, 0.3f, 0.3f);
        drawText2D(px1 + 15, py2 - 64, buf, GLUT_BITMAP_HELVETICA_12);

        if (sel.status == STATUS_OCCUPIED || sel.status == STATUS_TIME_EXPIRED) {
            char tBuf[32];
            formatTimeString(sel.remainingTime, tBuf);
            if (sel.status == STATUS_TIME_EXPIRED) {
                sprintf(buf, "Timer          : EXPIRED! (00:00)");
                glColor3f(1.0f, 0.2f, 0.2f);
            } else {
                sprintf(buf, "Timer Remaining: %s", tBuf);
                if (sel.remainingTime < 300) glColor3f(1.0f, 0.2f, 0.2f);
                else if (sel.remainingTime < 600) glColor3f(1.0f, 0.85f, 0.1f);
                else glColor3f(0.2f, 0.95f, 1.0f);
            }
            drawText2D(px1 + 15, py2 - 82, buf, GLUT_BITMAP_HELVETICA_12);

            if (sel.hasCustomer && strlen(sel.customer.name) > 0) {
                sprintf(buf, "Customer       : %s (%s)", sel.customer.name, sel.customer.vehicleNumber);
                glColor3f(0.9f, 0.9f, 0.95f);
                drawText2D(px1 + 15, py2 - 100, buf, GLUT_BITMAP_HELVETICA_12);
            }
        } else if (sel.status == STATUS_RESERVED || sel.type == TYPE_RESERVED) {
            char tBuf[32];
            formatTimeString(sel.reservationRemaining, tBuf);
            sprintf(buf, "Reserved Until : %s remaining", tBuf);
            glColor3f(1.0f, 0.85f, 0.1f);
            drawText2D(px1 + 15, py2 - 82, buf, GLUT_BITMAP_HELVETICA_12);
        } else {
            sprintf(buf, "Dist. Entrance : %.1f m", getSlotDistance(selectedSlotIndex));
            glColor3f(0.9f, 0.9f, 1.0f);
            drawText2D(px1 + 15, py2 - 82, buf, GLUT_BITMAP_HELVETICA_12);
        }

        int btnX1 = px1 + 15;
        int btnY1 = py2 - 192;
        int btnX2 = px2 - 15;
        int btnY2 = py2 - 152;

        if (sel.status == STATUS_AVAILABLE && sel.type != TYPE_RESERVED) {
            glColor4f(0.1f, 0.65f, 0.3f, 0.9f);
            glBegin(GL_QUADS);
            glVertex2i(btnX1, btnY2);
            glVertex2i(btnX2, btnY2);
            glVertex2i(btnX2, btnY1);
            glVertex2i(btnX1, btnY1);
            glEnd();

            glColor3f(1.0f, 1.0f, 1.0f);
            glLineWidth(2.0f);
            glBegin(GL_LINE_LOOP);
            glVertex2i(btnX1, btnY2);
            glVertex2i(btnX2, btnY2);
            glVertex2i(btnX2, btnY1);
            glVertex2i(btnX1, btnY1);
            glEnd();

            sprintf(buf, "BOOK SLOT %s & NAVIGATE (Press 'B')", sel.id);
            drawText2D(btnX1 + 35, btnY1 + 12, buf, GLUT_BITMAP_HELVETICA_12);
        } else if (sel.status == STATUS_OCCUPIED || sel.status == STATUS_TIME_EXPIRED) {
            int midX = btnX1 + (btnX2 - btnX1) / 2;

            glColor4f(0.85f, 0.25f, 0.20f, 0.9f);
            glBegin(GL_QUADS);
            glVertex2i(btnX1, btnY2);
            glVertex2i(midX - 5, btnY2);
            glVertex2i(midX - 5, btnY1);
            glVertex2i(btnX1, btnY1);
            glEnd();

            glColor3f(1.0f, 1.0f, 1.0f);
            glLineWidth(1.5f);
            glBegin(GL_LINE_LOOP);
            glVertex2i(btnX1, btnY2);
            glVertex2i(midX - 5, btnY2);
            glVertex2i(midX - 5, btnY1);
            glVertex2i(btnX1, btnY1);
            glEnd();
            drawText2D(btnX1 + 15, btnY1 + 12, "RELEASE ('R')", GLUT_BITMAP_HELVETICA_12);

            glColor4f(0.2f, 0.5f, 0.85f, 0.9f);
            glBegin(GL_QUADS);
            glVertex2i(midX + 5, btnY2);
            glVertex2i(btnX2, btnY2);
            glVertex2i(btnX2, btnY1);
            glVertex2i(midX + 5, btnY1);
            glEnd();

            glColor3f(1.0f, 1.0f, 1.0f);
            glLineWidth(1.5f);
            glBegin(GL_LINE_LOOP);
            glVertex2i(midX + 5, btnY2);
            glVertex2i(btnX2, btnY2);
            glVertex2i(btnX2, btnY1);
            glVertex2i(midX + 5, btnY1);
            glEnd();
            drawText2D(midX + 25, btnY1 + 12, "INFO ('I')", GLUT_BITMAP_HELVETICA_12);
        }
    }

    // 3. Navigation Controls Panel (Bottom Right)
    int nPanelW = 340;
    int nPanelH = 150;
    int nx1 = width - nPanelW - 15;
    int ny1 = 15;
    int nx2 = width - 15;
    int ny2 = 15 + nPanelH;

    glColor4f(0.08f, 0.12f, 0.18f, 0.88f);
    glBegin(GL_QUADS);
    glVertex2i(nx1, ny2);
    glVertex2i(nx2, ny2);
    glVertex2i(nx2, ny1);
    glVertex2i(nx1, ny1);
    glEnd();

    glColor3f(0.2f, 0.6f, 0.9f);
    glLineWidth(1.5f);
    glBegin(GL_LINE_LOOP);
    glVertex2i(nx1, ny2);
    glVertex2i(nx2, ny2);
    glVertex2i(nx2, ny1);
    glVertex2i(nx1, ny1);
    glEnd();

    glColor3f(1.0f, 0.9f, 0.2f);
    drawText2D(nx1 + 15, ny2 - 20, "KEYBOARD SHORTCUTS", GLUT_BITMAP_HELVETICA_12);

    int pbX1 = nx1 + 15;
    int pbY1 = ny2 - 62;
    int pbX2 = nx1 + 160;
    int pbY2 = ny2 - 30;

    glColor4f(0.85f, 0.55f, 0.10f, 0.9f);
    glBegin(GL_QUADS);
    glVertex2i(pbX1, pbY2);
    glVertex2i(pbX2, pbY2);
    glVertex2i(pbX2, pbY1);
    glVertex2i(pbX1, pbY1);
    glEnd();

    glColor3f(1.0f, 1.0f, 1.0f);
    drawText2D(pbX1 + 15, pbY1 + 10, activeVehicle.isPaused ? "RESUME ('P')" : "PAUSE ('P')", GLUT_BITMAP_HELVETICA_12);

    int rbX1 = nx1 + 175;
    int rbY1 = ny2 - 62;
    int rbX2 = nx2 - 15;
    int rbY2 = ny2 - 30;

    glColor4f(0.85f, 0.25f, 0.20f, 0.9f);
    glBegin(GL_QUADS);
    glVertex2i(rbX1, rbY2);
    glVertex2i(rbX2, rbY2);
    glVertex2i(rbX2, rbY1);
    glVertex2i(rbX1, rbY1);
    glEnd();

    glColor3f(1.0f, 1.0f, 1.0f);
    drawText2D(rbX1 + 18, rbY1 + 10, "RELEASE ('R')", GLUT_BITMAP_HELVETICA_12);

    glColor3f(0.9f, 0.9f, 0.95f);
    drawText2D(nx1 + 15, ny2 - 82, "B = Book | I = Info | R = Release | V = Reserve", GLUT_BITMAP_HELVETICA_12);
    drawText2D(nx1 + 15, ny2 - 100, "2=2D, 3=3D, N=Nav, P=Pause/Resume", GLUT_BITMAP_HELVETICA_12);
    drawText2D(nx1 + 15, ny2 - 118, "A=Add, D=Remove, L=Lighting", GLUT_BITMAP_HELVETICA_12);
    drawText2D(nx1 + 15, ny2 - 136, "W/S/Q/E & Arrows = Camera", GLUT_BITMAP_HELVETICA_12);

    // 4. Bottom VISUAL FEEDBACK BANNER
    int fbW = width - 30;
    int fbH = 40;
    int fx1 = 15;
    int fy1 = 15;
    int fx2 = 15 + fbW - nPanelW - 15;
    int fy2 = 15 + fbH;

    glColor4f(0.08f, 0.12f, 0.18f, 0.92f);
    glBegin(GL_QUADS);
    glVertex2i(fx1, fy2);
    glVertex2i(fx2, fy2);
    glVertex2i(fx2, fy1);
    glVertex2i(fx1, fy1);
    glEnd();

    glColor3f(0.2f, 0.6f, 0.9f);
    glLineWidth(1.5f);
    glBegin(GL_LINE_LOOP);
    glVertex2i(fx1, fy2);
    glVertex2i(fx2, fy2);
    glVertex2i(fx2, fy1);
    glVertex2i(fx1, fy1);
    glEnd();

    glColor3f(1.0f, 0.95f, 0.2f);
    drawText2D(fx1 + 15, fy1 + 13, "STATUS: ", GLUT_BITMAP_HELVETICA_12);

    glColor3f(0.2f, 0.95f, 1.0f);
    drawText2D(fx1 + 80, fy1 + 13, statusFeedback, GLUT_BITMAP_HELVETICA_12);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glEnable(GL_DEPTH_TEST);
    if (lightingEnabled && is3D) glEnable(GL_LIGHTING);
}

// -------------------------------------------------------------
// INTERACTIVE BOOKING MODAL DIALOG OVERLAY
// -------------------------------------------------------------
void drawBookingModal(int winW, int winH) {
    return; // Booking modal removed - direct booking without manual input

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, winW, 0, winH);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glColor4f(0.0f, 0.0f, 0.0f, 0.75f);
    glBegin(GL_QUADS);
    glVertex2i(0, winH);
    glVertex2i(winW, winH);
    glVertex2i(winW, 0);
    glVertex2i(0, 0);
    glEnd();

    int boxW = 560;
    int boxH = 360;
    int x1 = (winW - boxW) / 2;
    int y1 = (winH - boxH) / 2;
    int x2 = x1 + boxW;
    int y2 = y1 + boxH;

    glColor4f(0.10f, 0.14f, 0.22f, 0.98f);
    glBegin(GL_QUADS);
    glVertex2i(x1, y2);
    glVertex2i(x2, y2);
    glVertex2i(x2, y1);
    glVertex2i(x1, y1);
    glEnd();

    glColor3f(0.2f, 0.7f, 1.0f);
    glLineWidth(2.5f);
    glBegin(GL_LINE_LOOP);
    glVertex2i(x1, y2);
    glVertex2i(x2, y2);
    glVertex2i(x2, y1);
    glVertex2i(x1, y1);
    glEnd();

    char buf[256];

    if (currentBookingState == BOOKING_STATE_ENTER_NAME ||
        currentBookingState == BOOKING_STATE_ENTER_PHONE ||
        currentBookingState == BOOKING_STATE_ENTER_VEHICLE_NUM) {

        glColor3f(1.0f, 0.9f, 0.2f);
        drawText2D(x1 + 160, y2 - 35, "ENTER PERSON DETAILS", GLUT_BITMAP_HELVETICA_18);

        glColor3f(0.8f, 0.9f, 1.0f);
        sprintf(buf, "Target Slot: %s (%s)", slots[selectedSlotIndex].id, getSlotTypeName(slots[selectedSlotIndex].type));
        drawText2D(x1 + 40, y2 - 68, buf, GLUT_BITMAP_HELVETICA_12);

        // Name Field
        glColor3f(1.0f, 1.0f, 1.0f);
        drawText2D(x1 + 40, y2 - 100, "Name:", GLUT_BITMAP_HELVETICA_12);
        sprintf(buf, "> [%s%s]", tempCustomer.name, (currentBookingState == BOOKING_STATE_ENTER_NAME) ? "_" : "");
        if (currentBookingState == BOOKING_STATE_ENTER_NAME) glColor3f(0.2f, 0.95f, 1.0f);
        else glColor3f(0.7f, 0.7f, 0.7f);
        drawText2D(x1 + 40, y2 - 118, buf, GLUT_BITMAP_HELVETICA_12);

        // Phone Field
        glColor3f(1.0f, 1.0f, 1.0f);
        drawText2D(x1 + 40, y2 - 145, "Phone Number:", GLUT_BITMAP_HELVETICA_12);
        sprintf(buf, "> [%s%s]", tempCustomer.phone, (currentBookingState == BOOKING_STATE_ENTER_PHONE) ? "_" : "");
        if (currentBookingState == BOOKING_STATE_ENTER_PHONE) glColor3f(0.2f, 0.95f, 1.0f);
        else glColor3f(0.7f, 0.7f, 0.7f);
        drawText2D(x1 + 40, y2 - 163, buf, GLUT_BITMAP_HELVETICA_12);

        // Vehicle Num Field
        glColor3f(1.0f, 1.0f, 1.0f);
        drawText2D(x1 + 40, y2 - 190, "Vehicle Number:", GLUT_BITMAP_HELVETICA_12);
        sprintf(buf, "> [%s%s]", tempCustomer.vehicleNumber, (currentBookingState == BOOKING_STATE_ENTER_VEHICLE_NUM) ? "_" : "");
        if (currentBookingState == BOOKING_STATE_ENTER_VEHICLE_NUM) glColor3f(0.2f, 0.95f, 1.0f);
        else glColor3f(0.7f, 0.7f, 0.7f);
        drawText2D(x1 + 40, y2 - 208, buf, GLUT_BITMAP_HELVETICA_12);

        glColor3f(0.2f, 0.95f, 0.3f);
        drawText2D(x1 + 40, y1 + 45, "ENTER = Next Field / Continue", GLUT_BITMAP_HELVETICA_12);
        glColor3f(0.9f, 0.4f, 0.4f);
        drawText2D(x1 + 40, y1 + 22, "ESC = Cancel", GLUT_BITMAP_HELVETICA_12);
    }
    else if (currentBookingState == BOOKING_STATE_SELECT_VEHICLE_TYPE) {
        glColor3f(1.0f, 0.9f, 0.2f);
        drawText2D(x1 + 170, y2 - 35, "SELECT VEHICLE TYPE", GLUT_BITMAP_HELVETICA_18);

        glColor3f(1.0f, 1.0f, 1.0f);
        drawText2D(x1 + 60, y2 - 90,  "1. Car", GLUT_BITMAP_HELVETICA_12);
        drawText2D(x1 + 60, y2 - 120, "2. Bike", GLUT_BITMAP_HELVETICA_12);
        drawText2D(x1 + 60, y2 - 150, "3. SUV", GLUT_BITMAP_HELVETICA_12);
        drawText2D(x1 + 60, y2 - 180, "4. EV Car", GLUT_BITMAP_HELVETICA_12);

        glColor3f(0.2f, 0.95f, 1.0f);
        drawText2D(x1 + 40, y1 + 45, "Press 1, 2, 3, or 4 on keyboard.", GLUT_BITMAP_HELVETICA_12);
        glColor3f(0.9f, 0.4f, 0.4f);
        drawText2D(x1 + 40, y1 + 22, "ESC = Cancel.", GLUT_BITMAP_HELVETICA_12);
    }
    else if (currentBookingState == BOOKING_STATE_ACCESSIBLE_ELIGIBILITY) {
        glColor3f(1.0f, 0.9f, 0.2f);
        drawText2D(x1 + 120, y2 - 35, "ACCESSIBLE PARKING CHECK", GLUT_BITMAP_HELVETICA_18);

        glColor3f(0.95f, 0.85f, 0.2f);
        drawText2D(x1 + 50, y2 - 80, "Eligible for Accessible / Disabled Parking?", GLUT_BITMAP_HELVETICA_12);

        glColor3f(1.0f, 1.0f, 1.0f);
        drawText2D(x1 + 100, y2 - 130, "1. YES (Allowed to book Accessible slot)", GLUT_BITMAP_HELVETICA_12);
        drawText2D(x1 + 100, y2 - 165, "2. NO  (Normal vehicles cannot book Accessible slots)", GLUT_BITMAP_HELVETICA_12);

        glColor3f(0.2f, 0.95f, 1.0f);
        drawText2D(x1 + 40, y1 + 45, "Press 1 for YES, 2 for NO.", GLUT_BITMAP_HELVETICA_12);
        glColor3f(0.9f, 0.4f, 0.4f);
        drawText2D(x1 + 40, y1 + 22, "ESC = Cancel.", GLUT_BITMAP_HELVETICA_12);
    }
    else if (currentBookingState == BOOKING_STATE_SELECT_DURATION) {
        glColor3f(1.0f, 0.9f, 0.2f);
        drawText2D(x1 + 140, y2 - 35, "SELECT PARKING DURATION", GLUT_BITMAP_HELVETICA_18);

        glColor3f(1.0f, 1.0f, 1.0f);
        drawText2D(x1 + 60, y2 - 80,  "1. 30 Minutes", GLUT_BITMAP_HELVETICA_12);
        drawText2D(x1 + 60, y2 - 110, "2. 1 Hour", GLUT_BITMAP_HELVETICA_12);
        drawText2D(x1 + 60, y2 - 140, "3. 2 Hours", GLUT_BITMAP_HELVETICA_12);
        drawText2D(x1 + 60, y2 - 170, "4. 3 Hours", GLUT_BITMAP_HELVETICA_12);
        drawText2D(x1 + 60, y2 - 200, "5. 5 Hours", GLUT_BITMAP_HELVETICA_12);
        
        glColor3f(0.2f, 0.95f, 0.3f);
        drawText2D(x1 + 60, y2 - 235, "T = Test Timer (30s / 60s for Lab Demo)", GLUT_BITMAP_HELVETICA_12);

        glColor3f(0.2f, 0.95f, 1.0f);
        drawText2D(x1 + 40, y1 + 35, "Press 1 to 5 or T on keyboard.", GLUT_BITMAP_HELVETICA_12);
    }
    else if (currentBookingState == BOOKING_STATE_TEST_TIMER_SELECT) {
        glColor3f(1.0f, 0.9f, 0.2f);
        drawText2D(x1 + 150, y2 - 35, "TEST TIMER (LAB DEMO)", GLUT_BITMAP_HELVETICA_18);

        glColor3f(1.0f, 1.0f, 1.0f);
        drawText2D(x1 + 80, y2 - 100, "1. 30 Seconds Test Timer", GLUT_BITMAP_HELVETICA_12);
        drawText2D(x1 + 80, y2 - 140, "2. 60 Seconds Test Timer", GLUT_BITMAP_HELVETICA_12);

        glColor3f(0.2f, 0.95f, 1.0f);
        drawText2D(x1 + 40, y1 + 45, "Press 1 or 2 on keyboard.", GLUT_BITMAP_HELVETICA_12);
    }
    else if (currentBookingState == BOOKING_STATE_CONFIRM_SUMMARY) {
        glColor3f(1.0f, 0.9f, 0.2f);
        drawText2D(x1 + 160, y2 - 35, "BOOKING CONFIRMATION", GLUT_BITMAP_HELVETICA_18);

        glColor3f(0.9f, 0.9f, 1.0f);
        drawText2D(x1 + 40, y2 - 70, "PERSON DETAILS", GLUT_BITMAP_HELVETICA_12);
        
        glColor3f(1.0f, 1.0f, 1.0f);
        sprintf(buf, "Name            : %s", tempCustomer.name);
        drawText2D(x1 + 40, y2 - 92, buf, GLUT_BITMAP_HELVETICA_12);

        sprintf(buf, "Phone           : %s", tempCustomer.phone);
        drawText2D(x1 + 40, y2 - 110, buf, GLUT_BITMAP_HELVETICA_12);

        sprintf(buf, "Vehicle Number  : %s", tempCustomer.vehicleNumber);
        drawText2D(x1 + 40, y2 - 128, buf, GLUT_BITMAP_HELVETICA_12);

        sprintf(buf, "Vehicle Type    : %s", tempCustomer.vehicleType);
        drawText2D(x1 + 40, y2 - 146, buf, GLUT_BITMAP_HELVETICA_12);

        glColor3f(0.9f, 0.9f, 1.0f);
        drawText2D(x1 + 40, y2 - 175, "PARKING DETAILS", GLUT_BITMAP_HELVETICA_12);

        glColor3f(1.0f, 1.0f, 1.0f);
        sprintf(buf, "Slot            : %s (%s)", slots[selectedSlotIndex].id, getSlotTypeName(slots[selectedSlotIndex].type));
        drawText2D(x1 + 40, y2 - 195, buf, GLUT_BITMAP_HELVETICA_12);

        if (tempIsTestTimer) {
            sprintf(buf, "Duration        : %d Seconds (TEST TIMER)", tempDurationSec);
        } else {
            char tBuf[32];
            formatTimeString(tempDurationSec, tBuf);
            sprintf(buf, "Duration        : %s", tBuf);
        }
        drawText2D(x1 + 40, y2 - 215, buf, GLUT_BITMAP_HELVETICA_12);

        glColor3f(0.2f, 0.95f, 0.3f);
        drawText2D(x1 + 40, y1 + 45, "Press ENTER to Confirm Booking & Start Navigation!", GLUT_BITMAP_HELVETICA_12);
        glColor3f(0.9f, 0.4f, 0.4f);
        drawText2D(x1 + 40, y1 + 22, "Press ESC to Cancel.", GLUT_BITMAP_HELVETICA_12);
    }

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glEnable(GL_DEPTH_TEST);
    if (lightingEnabled) glEnable(GL_LIGHTING);
}

// -------------------------------------------------------------
// CUSTOMER / BOOKING INFORMATION DIALOG OVERLAY
// -------------------------------------------------------------
void drawCustomerInfoModal(int winW, int winH) {
    if (currentBookingState != BOOKING_STATE_CUSTOMER_INFO_VIEW) return;

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, winW, 0, winH);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glColor4f(0.0f, 0.0f, 0.0f, 0.75f);
    glBegin(GL_QUADS);
    glVertex2i(0, winH);
    glVertex2i(winW, winH);
    glVertex2i(winW, 0);
    glVertex2i(0, 0);
    glEnd();

    int boxW = 520;
    int boxH = 350;
    int x1 = (winW - boxW) / 2;
    int y1 = (winH - boxH) / 2;
    int x2 = x1 + boxW;
    int y2 = y1 + boxH;

    glColor4f(0.10f, 0.14f, 0.22f, 0.98f);
    glBegin(GL_QUADS);
    glVertex2i(x1, y2);
    glVertex2i(x2, y2);
    glVertex2i(x2, y1);
    glVertex2i(x1, y1);
    glEnd();

    glColor3f(0.2f, 0.7f, 1.0f);
    glLineWidth(2.5f);
    glBegin(GL_LINE_LOOP);
    glVertex2i(x1, y2);
    glVertex2i(x2, y2);
    glVertex2i(x2, y1);
    glVertex2i(x1, y1);
    glEnd();

    char buf[256];

    glColor3f(1.0f, 0.9f, 0.2f);
    drawText2D(x1 + 130, y2 - 35, "CUSTOMER / BOOKING INFO", GLUT_BITMAP_HELVETICA_18);

    if (selectedSlotIndex >= 0 && selectedSlotIndex < slotCount) {
        ParkingSlot sel = slots[selectedSlotIndex];

        glColor3f(1.0f, 1.0f, 1.0f);
        sprintf(buf, "Name           : %s", (sel.hasCustomer && strlen(sel.customer.name) > 0) ? sel.customer.name : "N/A");
        drawText2D(x1 + 40, y2 - 80, buf, GLUT_BITMAP_HELVETICA_12);

        sprintf(buf, "Phone          : %s", (sel.hasCustomer && strlen(sel.customer.phone) > 0) ? sel.customer.phone : "N/A");
        drawText2D(x1 + 40, y2 - 105, buf, GLUT_BITMAP_HELVETICA_12);

        sprintf(buf, "Vehicle Number : %s", (sel.hasCustomer && strlen(sel.customer.vehicleNumber) > 0) ? sel.customer.vehicleNumber : "N/A");
        drawText2D(x1 + 40, y2 - 130, buf, GLUT_BITMAP_HELVETICA_12);

        sprintf(buf, "Vehicle Type   : %s", (sel.hasCustomer && strlen(sel.customer.vehicleType) > 0) ? sel.customer.vehicleType : "Car");
        drawText2D(x1 + 40, y2 - 155, buf, GLUT_BITMAP_HELVETICA_12);

        sprintf(buf, "Slot           : %s (%s)", sel.id, getSlotTypeName(sel.type));
        drawText2D(x1 + 40, y2 - 185, buf, GLUT_BITMAP_HELVETICA_12);

        char tBuf[32];
        formatTimeString(sel.parkingDuration, tBuf);
        sprintf(buf, "Duration       : %s", tBuf);
        drawText2D(x1 + 40, y2 - 210, buf, GLUT_BITMAP_HELVETICA_12);

        formatTimeString(sel.remainingTime, tBuf);
        sprintf(buf, "Remaining      : %s", tBuf);
        if (sel.status == STATUS_TIME_EXPIRED) glColor3f(1.0f, 0.2f, 0.2f);
        else glColor3f(0.2f, 0.95f, 1.0f);
        drawText2D(x1 + 40, y2 - 235, buf, GLUT_BITMAP_HELVETICA_12);

        sprintf(buf, "Status         : %s", getSlotStatusName(sel.status));
        if (sel.status == STATUS_TIME_EXPIRED) glColor3f(1.0f, 0.2f, 0.2f);
        else glColor3f(0.2f, 0.95f, 0.3f);
        drawText2D(x1 + 40, y2 - 260, buf, GLUT_BITMAP_HELVETICA_12);
    }

    glColor3f(0.2f, 0.95f, 1.0f);
    drawText2D(x1 + 40, y1 + 40, "Press 'R' to Release Slot.", GLUT_BITMAP_HELVETICA_12);
    glColor3f(0.9f, 0.4f, 0.4f);
    drawText2D(x1 + 40, y1 + 20, "Press ESC or 'I' to close window.", GLUT_BITMAP_HELVETICA_12);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glEnable(GL_DEPTH_TEST);
    if (lightingEnabled) glEnable(GL_LIGHTING);
}

// -------------------------------------------------------------
// MOUSE PICKING OF PARKING SLOTS IN 2D & 3D VIEWS
// -------------------------------------------------------------
int pickSlotWithMouse(int mouseX, int mouseY, int winW, int winH, int is3D) {
    if (currentBookingState != BOOKING_STATE_NONE) return 0;

    applyCamera(is3D, winW, winH);

    GLdouble modelMatrix[16], projMatrix[16];
    GLint viewport[4];
    
    glGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix);
    glGetDoublev(GL_PROJECTION_MATRIX, projMatrix);
    glGetIntegerv(GL_VIEWPORT, viewport);
    
    GLdouble winY = (GLdouble)viewport[3] - (GLdouble)mouseY;
    
    GLdouble nearX, nearY, nearZ;
    GLdouble farX, farY, farZ;
    
    if (gluUnProject(mouseX, winY, 0.0, modelMatrix, projMatrix, viewport, &nearX, &nearY, &nearZ) != GL_TRUE ||
        gluUnProject(mouseX, winY, 1.0, modelMatrix, projMatrix, viewport, &farX, &farY, &farZ) != GL_TRUE) {
        return 0;
    }

    double dy = farY - nearY;
    if (fabs(dy) < 1e-6) return 0;
    
    double t = -nearY / dy;
    float gx = (float)(nearX + t * (farX - nearX));
    float gz = (float)(nearZ + t * (farZ - nearZ));

    for (int i = 0; i < slotCount; i++) {
        float halfW = slots[i].width * 0.6f;
        float halfD = slots[i].depth * 0.6f;

        if (fabsf(gx - slots[i].x) <= halfW && fabsf(gz - slots[i].z) <= halfD) {
            selectedSlotIndex = i;

            if (slots[i].status == STATUS_RESERVED || slots[i].type == TYPE_RESERVED) {
                currentPathCount = 0;
                sprintf(statusFeedback, "This parking slot is RESERVED. You cannot park here.");
            } else if (slots[i].status == STATUS_OCCUPIED || slots[i].status == STATUS_TIME_EXPIRED) {
                currentPathCount = 0;
                sprintf(statusFeedback, "This slot is already occupied.");
            } else {
                if (!activeVehicle.isNavigating) {
                    calculatePathToSlot(selectedSlotIndex, currentPath, &currentPathCount);
                }
                sprintf(statusFeedback, "Selected Slot %s (Available, %s). Press 'B' to Book.",
                        slots[i].id, getSlotTypeName(slots[i].type));
            }
            return 1;
        }
    }

    return 0;
}

// -------------------------------------------------------------
// HUD ON-SCREEN BUTTON CLICK INTERACTION
// -------------------------------------------------------------
int handleHUDButtonClick(int mouseX, int mouseY, int winW, int winH) {
    if (currentBookingState != BOOKING_STATE_NONE) return 0;

    int invY = winH - mouseY;

    int panelW = 410;
    int px1 = winW - panelW - 15;
    int px2 = winW - 15;
    int py2 = winH - 15;

    // 1. Check [ BOOK SLOT ] / [ RELEASE SLOT ] / [ INFO ] Buttons
    int btnX1 = px1 + 15;
    int btnY1 = py2 - 192;
    int btnX2 = px2 - 15;
    int btnY2 = py2 - 152;

    if (mouseX >= btnX1 && mouseX <= btnX2 && invY >= btnY1 && invY <= btnY2) {
        if (selectedSlotIndex >= 0) {
            ParkingSlot sel = slots[selectedSlotIndex];
            if (sel.status == STATUS_AVAILABLE && sel.type != TYPE_RESERVED) {
                // Direct booking - no manual input required
                memset(&tempCustomer, 0, sizeof(Person));
                sprintf(tempCustomer.name, "Customer");
                sprintf(tempCustomer.phone, "N/A");
                sprintf(tempCustomer.vehicleNumber, "N/A");
                if (sel.type == TYPE_EV) {
                    sprintf(tempCustomer.vehicleType, "EV Car");
                } else {
                    sprintf(tempCustomer.vehicleType, "Car");
                }
                tempCustomer.eligibleAccessible = (sel.type == TYPE_DISABLED) ? 1 : 0;

                slots[selectedSlotIndex].customer = tempCustomer;
                slots[selectedSlotIndex].hasCustomer = 1;
                slots[selectedSlotIndex].parkingDuration = 7200;
                slots[selectedSlotIndex].isTestTimer = 0;

                startVehicleNavigation(selectedSlotIndex);
            } else if (sel.status == STATUS_OCCUPIED || sel.status == STATUS_TIME_EXPIRED) {
                int midX = btnX1 + (btnX2 - btnX1) / 2;
                if (mouseX <= midX) {
                    startExitNavigation(selectedSlotIndex);
                } else {
                    currentBookingState = BOOKING_STATE_CUSTOMER_INFO_VIEW;
                }
            }
        }
        return 1;
    }

    int nPanelW = 340;
    int nx1 = winW - nPanelW - 15;
    int ny2 = 15 + 150;

    // 2. Check [ PAUSE / RESUME ] Button
    int pbX1 = nx1 + 15;
    int pbY1 = ny2 - 62;
    int pbX2 = nx1 + 160;
    int pbY2 = ny2 - 30;

    if (mouseX >= pbX1 && mouseX <= pbX2 && invY >= pbY1 && invY <= pbY2) {
        togglePauseVehicleNavigation();
        return 1;
    }

    // 3. Check [ RELEASE SLOT ] Button
    int rbX1 = nx1 + 175;
    int rbY1 = ny2 - 62;
    int rbX2 = winW - 30;
    int rbY2 = ny2 - 30;

    if (mouseX >= rbX1 && mouseX <= rbX2 && invY >= rbY1 && invY <= rbY2) {
        if (selectedSlotIndex >= 0) {
            startExitNavigation(selectedSlotIndex);
        } else {
            resetVehicleNavigation();
        }
        return 1;
    }

    return 0;
}

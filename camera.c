#include "camera.h"
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Camera mainCamera;

void initCamera(void) {
    mainCamera.targetX = 0.0f;
    mainCamera.targetY = 0.0f;
    mainCamera.targetZ = 0.0f;
    mainCamera.upX = 0.0f;
    mainCamera.upY = 1.0f;
    mainCamera.upZ = 0.0f;
    
    mainCamera.yaw = -90.0f;  // Facing along Z axis
    mainCamera.pitch = 40.0f; // 40 degrees elevation
    mainCamera.distance = 55.0f;

    rotateCamera(0.0f, 0.0f); // Compute initial eye pos
}

void rotateCamera(float deltaYaw, float deltaPitch) {
    mainCamera.yaw += deltaYaw;
    mainCamera.pitch += deltaPitch;

    if (mainCamera.pitch > 89.0f) mainCamera.pitch = 89.0f;
    if (mainCamera.pitch < 10.0f) mainCamera.pitch = 10.0f;

    float radYaw = mainCamera.yaw * (float)M_PI / 180.0f;
    float radPitch = mainCamera.pitch * (float)M_PI / 180.0f;

    mainCamera.eyeX = mainCamera.targetX + mainCamera.distance * cosf(radPitch) * cosf(radYaw);
    mainCamera.eyeY = mainCamera.targetY + mainCamera.distance * sinf(radPitch);
    mainCamera.eyeZ = mainCamera.targetZ + mainCamera.distance * cosf(radPitch) * sinf(radYaw);
}

void moveCamera(float forward, float right, float up) {
    mainCamera.distance -= forward * 2.0f;
    if (mainCamera.distance < 15.0f) mainCamera.distance = 15.0f;
    if (mainCamera.distance > 120.0f) mainCamera.distance = 120.0f;

    mainCamera.targetY += up;
    if (mainCamera.targetY < -5.0f) mainCamera.targetY = -5.0f;
    if (mainCamera.targetY > 20.0f) mainCamera.targetY = 20.0f;

    rotateCamera(0.0f, 0.0f);
}

void resetCamera(void) {
    initCamera();
}

void applyCamera(int is3D, int windowWidth, int windowHeight) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    float aspect = (float)windowWidth / (float)(windowHeight ? windowHeight : 1);

    if (is3D) {
        gluPerspective(45.0, aspect, 1.0, 300.0);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        gluLookAt(mainCamera.eyeX, mainCamera.eyeY, mainCamera.eyeZ,
                  mainCamera.targetX, mainCamera.targetY, mainCamera.targetZ,
                  mainCamera.upX, mainCamera.upY, mainCamera.upZ);
    } else {
        // Orthographic Top-Down 2D View
        float orthoSize = 38.0f;
        if (aspect >= 1.0f) {
            glOrtho(-orthoSize * aspect, orthoSize * aspect, -orthoSize, orthoSize, -100.0, 100.0);
        } else {
            glOrtho(-orthoSize, orthoSize, -orthoSize / aspect, orthoSize / aspect, -100.0, 100.0);
        }
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        // Top-down camera looking straight down at Y=0
        gluLookAt(0.0, 50.0, 0.0,
                  0.0, 0.0, 0.0,
                  0.0, 0.0, -1.0); // Up vector points north (-Z)
    }
}

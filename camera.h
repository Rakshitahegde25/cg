#ifndef CAMERA_H
#define CAMERA_H

typedef struct {
    float eyeX, eyeY, eyeZ;
    float targetX, targetY, targetZ;
    float upX, upY, upZ;
    float yaw;     // Angle in degrees
    float pitch;   // Angle in degrees
    float distance;// Orbit radius
} Camera;

extern Camera mainCamera;

void initCamera(void);
void applyCamera(int is3D, int windowWidth, int windowHeight);
void moveCamera(float forward, float right, float up);
void rotateCamera(float deltaYaw, float deltaPitch);
void resetCamera(void);

#endif

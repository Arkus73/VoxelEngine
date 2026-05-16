#ifndef CAMERA_H
#define CAMERA_H

#include <cglm/cglm.h>
#include <GLFW/glfw3.h>

typedef struct {
    vec3 pos, dir, right, up;
    float yaw, pitch;
} Camera;

Camera* createCamera(vec3 pos, float yaw, float pitch);
void destroyCamera(Camera* this);
void getCameraView(Camera* this, mat4 dest);
void processCameraKeyboardInput(GLFWwindow* window, Camera* this, float delta);
void processCameraMouseInput(Camera* this, float xOffset, float yOffset);

#endif
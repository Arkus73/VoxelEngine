#ifndef UTILS_H
#define UTILS_H

#include <GLFW/glfw3.h>
#include <stdbool.h>

GLFWwindow* InitAndCreateWindow(int windowWidth, int windowHeight, const char* title);  // Initialisiert Glad und erstellt ein Fenster
void default_framebuffer_size_callback(GLFWwindow* window, int width, int height);     // Callback-Funktion bei Window Resize
void throwException(char* msg);
float clamp(float value, float min, float max);
float randf(float lowerBound, float upperBound);
bool randomise(float probability);
unsigned int createTexture(const char* dir, int filter);
int modulo(int a, int b);

#endif
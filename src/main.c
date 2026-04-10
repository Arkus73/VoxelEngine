#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>

#include "camera.h"
#include "shader.h"
#include "stb_image.h"
#include "utils.h"
#include "dynamicArray.h"

#include <stdint.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

void processInput(GLFWwindow* window, float delta);

int main() {
    
    INIT_MODULE_DYNAMIC_ARRAY;

    glfwInit();
    GLFWwindow* window = InitAndCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Minecraft Clone");
    glfwSetFramebufferSizeCallback(window, default_framebuffer_size_callback);

    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    unsigned int blockAtlas = createTexture("../assets/blockAtlas.png", GL_NEAREST);

    float lastFrame = glfwGetTime();

    while(!glfwWindowShouldClose(window)) {
        
        glClearColor(0.55f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float currentFrame = glfwGetTime();
        float delta = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window, delta);

        glfwSwapBuffers(window);
        glfwPollEvents();

    }

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window, float delta) {
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, 1);
    }
}
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>

#include "camera.h"
#include "shader.h"
#include "stb_image.h"
#include "utils.h"
#include "dynamicArray.h"

#include "chunk.h"
#include "block.h"

#include <stdint.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

Camera* cam;
float lastX = WINDOW_WIDTH / 2, lastY = WINDOW_HEIGHT / 2;
bool firstMouse = true;

void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void processInput(GLFWwindow* window, float delta);

int main() {
    
    INIT_MODULE_DYNAMIC_ARRAY;

    glfwInit();
    GLFWwindow* window = InitAndCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Minecraft Clone");
    glfwSetFramebufferSizeCallback(window, default_framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);

    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    Shader shader = createShader("../src/shaders/vertex.glsl", "../src/shaders/fragment.glsl");
    use(shader);

    mat4 model, view, proj;
    glm_mat4_identity(model);
    glm_mat4_identity(view);
    glm_perspective(glm_rad(60.0f), (float) WINDOW_WIDTH / (float) WINDOW_HEIGHT, 0.1f, 100.0f, proj);

    setMatrix(shader, "proj", proj);
    setMatrix(shader, "model", model);

    unsigned int blockAtlas = createTexture("../assets/blockAtlas.png", GL_NEAREST);
    setInt(shader, "blockAtlas", 0);

    cam = createCamera((vec3) {2.0f, 0.0f, 25.0f}, -90.0f, 0.0f);

    Block* blocks = malloc(CHUNK_WIDTH * CHUNK_HEIGHT * CHUNK_DEPTH * sizeof(Block));
    for(int x = 0; x < CHUNK_WIDTH; x++) {
        for(int y = 0; y < CHUNK_HEIGHT; y++) {
            for(int z = 0; z < CHUNK_DEPTH; z++) {
                blocks[x * CHUNK_HEIGHT * CHUNK_DEPTH + y * CHUNK_DEPTH + z] = dirt;
            }
        }
    }

    Chunk* chunk = createChunk(blocks, 0, 0);
    free(blocks);

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    float lastFrame = glfwGetTime();

    while(!glfwWindowShouldClose(window)) {
        
        glClearColor(0.55f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float currentFrame = glfwGetTime();
        float delta = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window, delta);
        getCameraView(cam, view);

        use(shader);
        setMatrix(shader, "view", view);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, blockAtlas);

        renderChunk(chunk, shader);

        glfwSwapBuffers(window);
        glfwPollEvents();

    }

    destroyChunk(chunk);
    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window, float delta) {
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, 1);
    }
    processCameraKeyboardInput(window, cam, delta);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {

    if(firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xOffset = xpos - lastX;
    float yOffset = lastY - ypos;
    processCameraMouseInput(cam, xOffset, yOffset);

    lastX = xpos;
    lastY = ypos;

}
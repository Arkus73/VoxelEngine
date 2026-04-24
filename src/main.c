// technische OpenGL includes
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>

// utils includes
#include "shader.h"
#include "stb_image.h"
#include "utils.h"
#include "dynamicArray.h"

// world und render includes
#include "chunkRenderer.h"
#include "block.h"
#include "worldGenerator.h"

// player includes
#include "camera.h"

// C includes

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

Camera* cam;
float lastX = WINDOW_WIDTH / 2, lastY = WINDOW_HEIGHT / 2;
bool firstMouse = true;

void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void processInput(GLFWwindow* window, float delta);

int main() {

    // Das Setup findet statt
    glfwInit();
    GLFWwindow* window = InitAndCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Minecraft Clone");
    glfwSetFramebufferSizeCallback(window, default_framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);

    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    // Die Shader werden geladen
    Shader shader = createShader("../src/shaders/vertex.glsl", "../src/shaders/fragment.glsl");
    use(shader);

    // Die nötigen Matrizen werden initialisiert und dem Vertex-Shader übergeben
    mat4 view, proj;
    glm_mat4_identity(view);
    glm_perspective(glm_rad(60.0f), (float) WINDOW_WIDTH / (float) WINDOW_HEIGHT, 0.1f, (RENDER_DISTANCE + 1) * CHUNK_WIDTH * sqrt(2), proj);

    setMatrix(shader, "proj", proj);

    // Der Block-Atlas wird geladen und ihm wird die Texture-Unit 0 zugeteilt
    unsigned int blockAtlas = createTexture("../assets/blockAtlas.png", GL_NEAREST);
    setInt(shader, "blockAtlas", 0);

    // Die Kamera wird initialisiert
    cam = createCamera((vec3) {0.0f, 10.0f, 0.0f}, -90.0f, 0.0f);
    
    // Alles zur Welt und Rendering benötigtes wird initialisiert
    initBlocks();
    generateWorld();
    initChunkRenderer();
    remeshLoadedChunks();

    glEnable(GL_CULL_FACE); // Face-Culling wird aktiviert, wodurch Faces, die vom Spieler wegzeigen nicht gerendert werden, was zu deutlich besserer Perfomance führt

    glEnable(GL_DEPTH_TEST);    // Depth-Test wird angeschaltet, um zu gewährleisten, dass die 3D-Drawing-Order eingehalten wird

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);  // Wireframe-Modus falls nötig

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);    // Cursor wird im Fenster gecatched

    float lastFrame = glfwGetTime();

    while(!glfwWindowShouldClose(window)) {
        
        glClearColor(0.55f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float currentFrame = glfwGetTime(); // delta wird berechnet
        float delta = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window, delta);
        getCameraView(cam, view);

        use(shader);
        setMatrix(shader, "view", view);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, blockAtlas);

        renderChunks(shader);

        glfwSwapBuffers(window);
        glfwPollEvents();

    }

    // Alles wird aufgeräumt
    destroyChunks();
    destroyBlocks();
    destroyChunks();
    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window, float delta) {
    // Wird Escape gedrückt, wird das Fenster geschlossen
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, 1);
    }

    vec3 lastPlayerPos;
    glm_vec3_copy(cam->pos, lastPlayerPos);
    processCameraKeyboardInput(window, cam, delta); // Spieler-Input wird gehandelt
    // Hat sich der Chunk des Spielers geändert, werden die geladenen Chunks geupdated
    if(lastPlayerPos[0] != cam->pos[0] || lastPlayerPos[2] != cam->pos[2]) {
        dynamicallyLoadAndUnloadChunks(lastPlayerPos, cam->pos);
    }
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
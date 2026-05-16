// technische OpenGL includes
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>

// utils includes
#include "shader.h"
#include "stb_image.h"
#include "utils.h"

// world und render includes
#include "chunkRenderer.h"
#include "block.h"
#include "worldGenerator.h"

// player includes
#include "camera.h"

// C includes
#include <time.h>
#include <string.h>

// Refactoring:


// Progress:
// TODO Unendliche Welt durch chunkGenerationWorkBatch
// TODO Spieler-Controller und Physik
// TODO Speichern von Chunks bei Veränderung (->dirty-Flag)

// Sonstiges:
// TODO Ein bisschen mit Postprocessing und Shadern herumspielen

Shader shader;

Camera* cam;
float lastX, lastY;
bool firstMouse = true;
mat4 proj;

ChunkRenderer* chunkRenderer;
WorldGenerator* worldGenerator;

int RENDER_DISTANCE = 0;
float FOV = 0.0f;

void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window, float delta);
void __destructor deinit(); // Funktion, die alles im Falle der Beendiung des Programms aufräumt

int main(int argc, char** argv) {

    // Die Anfangsargumente werden geparst
    bool vsync = false;
    bool wireframe = false;
    int seed = 0;
    float frequency = 0.0f;
    int gradientCount = 0;
    int windowWidth = 0;
    int windowHeight = 0;
    int currentToken = 1;
    while(currentToken < argc) {

        if(strcmp(argv[currentToken], "--seed") == 0) {
            currentToken++;
            if(currentToken >= argc) {
                throwException("Seed couldn't be parsed");
            }
            char* endptr;
            seed = (int) strtol(argv[currentToken], &endptr, 10);
            if(endptr == argv[currentToken]) {
                throwException("Seed couldn't be parsed");
            }
        }

        else if(strcmp(argv[currentToken], "--frequency") == 0) {
            currentToken++;
            if(currentToken >= argc) {
                throwException("Frequency couldn't be parsed");
            }
            char* endptr;
            frequency = (float) strtof(argv[currentToken], &endptr);
            if(endptr == argv[currentToken]) {
                throwException("Frequency couldn't be parsed");
            }
        }

        else if(strcmp(argv[currentToken], "--gradientCount") == 0) {
            currentToken++;
            if(currentToken >= argc) {
                throwException("GradientCount couldn't be parsed");
            }
            char* endptr;
            gradientCount = (int) strtol(argv[currentToken], &endptr, 10);
            if(endptr == argv[currentToken]) {
                throwException("GradientCount couldn't be parsed");
            }
        }

        else if(strcmp(argv[currentToken], "--wireframe") == 0) {
            wireframe = true;
        }

        else if(strcmp(argv[currentToken], "--vsync") == 0) {
            vsync = true;
        }

        else if(strcmp(argv[currentToken], "--windowWidth") == 0) {
            currentToken++;
            if(currentToken >= argc) {
                throwException("WindowWidth couldn't be parsed");
            }
            char* endptr;
            windowWidth = (int) strtol(argv[currentToken], &endptr, 10);
            if(endptr == argv[currentToken]) {
                throwException("WindowWidth couldn't be parsed");
            }
        }

        else if(strcmp(argv[currentToken], "--windowHeight") == 0) {
            currentToken++;
            if(currentToken >= argc) {
                throwException("WindowHeight couldn't be parsed");
            }
            char* endptr;
            windowHeight = (int) strtol(argv[currentToken], &endptr, 10);
            if(endptr == argv[currentToken]) {
                throwException("WindowHeight couldn't be parsed");
            }
        }

        else if(strcmp(argv[currentToken], "--FOV") == 0) {
            currentToken++;
            if(currentToken >= argc) {
                throwException("FOV couldn't be parsed");
            }
            char* endptr;
            FOV = (float) strtof(argv[currentToken], &endptr);
            if(endptr == argv[currentToken]) {
                throwException("FOV couldn't be parsed");
            }
        }

        else if(strcmp(argv[currentToken], "--renderDistance") == 0) {
            currentToken++;
            if(currentToken >= argc) {
                throwException("RenderDistance couldn't be parsed");
            }
            char* endptr;
            RENDER_DISTANCE = (int) strtol(argv[currentToken], &endptr, 10);
            if(endptr == argv[currentToken]) {
                throwException("RenderDistance couldn't be parsed");
            }
        }

        else{
            throwException("Flag not recognised");
        }

        currentToken++;
    }

    // Wurde ein Wert nicht in den Argumenten gesetzt oder wurde ein nicht zugelassener Wert gesetzt, wird er gedefaultet
    seed = (seed == 0) ? (int) time(NULL) : seed;
    frequency = (frequency == 0.0f || frequency < 0.0f) ? 0.8f : frequency;
    gradientCount = (gradientCount == 0 || gradientCount < 0) ? 8 : gradientCount;
    windowWidth = (windowWidth == 0 || windowWidth < 0) ? 800 : windowWidth;
    windowHeight = (windowHeight == 0 || windowWidth < 0) ? 600 : windowHeight;
    FOV = (FOV == 0.0f || FOV < 0.0f) ? 60.0f : FOV;
    RENDER_DISTANCE = (RENDER_DISTANCE == 0 || RENDER_DISTANCE < 0) ? 10 : RENDER_DISTANCE;

    lastX = (float) windowWidth / 2.0f;
    lastY = (float) windowHeight / 2.0f;

    // Das Setup findet statt
    glfwInit();
    GLFWwindow* window = InitAndCreateWindow(windowWidth, windowHeight, "Minecraft Clone");
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    // vsync wird aktiviert
    if(vsync) {
        glfwSwapInterval(1); 
    }

    glViewport(0, 0, windowWidth, windowHeight);

    // Die Shader werden geladen
    shader = createShader("../src/shaders/vertex.glsl", "../src/shaders/fragment.glsl");
    use(shader);

    // Die nötigen Matrizen werden initialisiert und dem Vertex-Shader übergeben
    mat4 view;
    glm_mat4_identity(view);
    glm_perspective(glm_rad(FOV), (float) windowWidth / (float) windowHeight, 0.1f, 1000.0f, proj);

    setMatrix(shader, "proj", proj);

    // Der Block-Atlas wird geladen und ihm wird die Texture-Unit 0 zugeteilt
    unsigned int blockAtlas = createTexture("../assets/blockAtlas.png", GL_NEAREST);
    setInt(shader, "blockAtlas", 0);

    // Die Kamera wird initialisiert
    cam = createCamera((vec3) {0.0f, 20.0f, 0.0f}, -90.0f, 0.0f);
    
    // Alles zur Welt und Rendering benötigtes wird initialisiert
    initBlocks();
    worldGenerator = initWorldGenerator(gradientCount, seed, frequency);
    chunkRenderer = initChunkRenderer();
    generateLoadedChunks(chunkRenderer, worldGenerator);
    remeshLoadedChunks(chunkRenderer);

    glEnable(GL_CULL_FACE); // Face-Culling wird aktiviert, wodurch Faces, die vom Spieler wegzeigen nicht gerendert werden, was zu deutlich besserer Perfomance führt

    glEnable(GL_DEPTH_TEST);    // Depth-Test wird angeschaltet, um zu gewährleisten, dass die 3D-Drawing-Order eingehalten wird

    if(wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);    // Cursor wird im Fenster gecatched

    float lastFrame = glfwGetTime();

    long int frames = 0;

    while(!glfwWindowShouldClose(window)) {

        frames++;
        
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

        renderChunks(chunkRenderer, shader, view, proj);

        glfwSwapBuffers(window);
        glfwPollEvents();

    }

    printf("Average FPS: %.2f", frames / glfwGetTime());
    return EXIT_SUCCESS;
}

void processInput(GLFWwindow* window, float delta) {
    // Wird Escape gedrückt, wird das Fenster geschlossen
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, 1);
    }

    vec3 lastPlayerPos;
    glm_vec3_copy(cam->pos, lastPlayerPos);
    processCameraKeyboardInput(window, cam, delta); // Spieler-Input wird gehandelt
    dynamicallyLoadAndUnloadChunks(chunkRenderer, worldGenerator, lastPlayerPos, cam->pos);    // Die geladenen Chunks werden geupdatet
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

void __destructor deinit() {
    // Alles wird aufgeräumt
    system("if exist \"..\\chunks\\*.bin\" del ..\\chunks\\*.bin");
    deinitWorldGenerator(worldGenerator);
    deinitChunkRenderer(chunkRenderer);
    destroyBlocks();
    destroyCamera(cam);
    glfwTerminate();
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    glm_perspective(glm_rad(FOV), (float) width / (float) height, 0.1f, 1000.0f, proj);
    setMatrix(shader, "proj", proj);
}
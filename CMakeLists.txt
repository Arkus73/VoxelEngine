cmake_minimum_required(VERSION 3.10)
project(Test)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

# OpenGL finden
find_package(OpenGL REQUIRED)

# GLAD einbinden
add_subdirectory(external/glad)

# GLFW einbinden (GLFW bringt eigene CMakeLists.txt mit)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(external/glfw)

# CGLM einbinden
add_subdirectory(external/cglm)

# Executable
add_executable(Test

    src/main.c

    src/world/block.c
    src/world/chunkFileInterfacing.c
    src/world/worldGenerator.c

    src/player/camera.c

    src/render/chunk.c
    src/render/mesh.c
    src/render/chunkRenderer.c

    src/utils/shader.c
    src/utils/stb_image.c
    src/utils/utils.c
    src/utils/dynamicArray.c
    
)

# Includes
target_include_directories(Test
    PUBLIC external/glad/include/
    PUBLIC headers/player
    PUBLIC headers/render
    PUBLIC headers/utils
    PUBLIC headers/world
)

# Linken
target_link_libraries(Test
    glfw
    glad
    cglm
    ${OPENGL_gl_LIBRARY}
)

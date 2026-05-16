# Voxel-Engine

## Description
This is a small Minecraft-inspired Voxel-Engine I programmed in C using OpenGL for rendering, GLFW for the window and input handling as well as CGLM for the math utilities. This is a hobby project, so feedback would be greatly appreciated.

## Usage
### Basic Running
To run this application download the latest zip from the releases, extract all the files, navigate to the executable in a terminal and run it with `.\Test`. (Currently Windows-only. If you are interested in Linux support, feel free to open an issue.)

### Controls
- Mouse for looking around
- w, a, s, d for basic movement
- q and e for moving down/up

### Arguments
When running the application various flags can be provided, such as:
- `--seed <int>` Specifies a seed to use for world generation. This defaults to the current time.
- `--frequency <float>` Specifies a frequency for the noise function. This defaults to 0.8.
- `--wireframe` Enables wireframe rendering mode. Mostly for debugging purposes.
- `--gradientCount <unsigned int>` Specifies a number of gradients to use for the noise function.
- `--windowWidth <unsigned int>` Specifies a window width. This defaults to 800.
- `--windowHeight <unsigned int>` Specifies a window height. This defaults to 600.
- `--FOV <float>` Specifies a FOV (= field of view) in Degree. This defaults to 60.0.
- `--renderDistance <unsigned int>` Specifies a render distance in Chunks. This defaults to 10.
- `--vsync` Enables Vsync.

*Examples*:
```powershell 
.\Test --seed 5986812 --frequency 0.4 # Sets the seed to 5986812 and the frequency to 0.4
.\Test --wireframe --seed 0 --vsync # Enables wireframe, sets the seed to 0 and enables Vsync
.\Test --windowWidth 1200 --windowHeight 800 --frequency  # Would set the window width and height to 1200, 800, but results in an error as no frequency was specified
```

### Building
To build this project yourself, you'll have to use Windows and have MinGW as well as CMake installed. The necessary libraries are already included in the repository ([GLFW](https://github.com/glfw/glfw), GLAD and [CGLM](https://github.com/recp/cglm)). Run The following commands in the VoxelEngine directory in your terminal to build the Engine:
```powershell
mkdir build
cd build
cmake .. -G "MinGW Makefiles"
cmake --build .
```

## Current Features
- A procedurally generated, infinite world
- A first person camera with free movement
- The ability to customise the engine with CLI arguments

## Technical Features
- Chunk-based world system
- Procedural terrain generation using noise functions
- Frustum and face culling
- Infinite terrain streaming

## Plans
- Ability to place and break blocks
- Physics for the player controller (collision detection, gravity)
- Dynamic lighting and shadows
- More fleshed out terrain generation

*Note*: For further technical details please refer to the code with various comments. If someone is interested, I could also write a quick rundown of the pipeline.
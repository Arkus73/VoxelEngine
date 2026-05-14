# Voxel Engine

## Description
This is a small Minecraft-like Voxel-Engine I programmed in C using OpenGL for rendering, GLFW for the window and input handling as well as CGLM for the math utilities. This is a hobby project, so feedback would be greatly appreciated.

## Usage
### Basic Running
To run this application download the latest zip from the releases, extract all the files, navigate to the executable in a terminal and run it with `.\Test`. (Currently Windows-only. If you are interested in Linux support, feel free to open an issue.)

### Controls
- Mouse for looking around
- w, a, s, d for basic movement
- q and e for moving down/up

### Arguments
When running the application various flags can be provided, such as:
- `--seed <value>` Specifies a seed to use for world generation. This defaults to the current time.
- `--frequency <value>` Specifies a frequency for the noise function. This defaults to 0.8.
- `--wireframe` Enables wireframe rendering mode. Mostly for debugging purposes

*Examples*:
```powershell 
.\Test --seed 5986812 --frequency 0.4 # Sets the seed to 5986812 and the frequency to 0.4
.\Test --wireframe --seed 0  # Enables wireframe and sets the seed to 0
.\Test --frequency   # Results in an error as no frequency was specified
```

## Current Features
- A procedurally generated, finite world
- A first person camera with a free movement
- The ability to customise the generation process a bit and get a look behind the scenes with a wireframe-mode

## Plans
- An infinite World
- A proper controller with physics and the ability to break and place blocks
- Lighting and Shadows

*Note*: For technical details please refer to the code with various comments. If someone is interested, I could also write a quick rundown of the pipeline.
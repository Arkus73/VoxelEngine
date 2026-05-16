#ifndef WORLD_GENERATOR_H
#define WORLD_GENERATOR_H

#include <windows.h>
#include <cglm/cglm.h>

typedef struct {
    int seed, gradientCount;
    float frequency;
    vec2* gradients;
} WorldGenerator;

WorldGenerator* initWorldGenerator(int gradientCount, int seed, float frequency);
void deinitWorldGenerator(WorldGenerator* this);

void generateWorld(WorldGenerator* this);
void __stdcall generateChunk(PTP_CALLBACK_INSTANCE instance, void* param, PTP_WORK work);

#endif
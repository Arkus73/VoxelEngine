#include "worldGenerator.h"
#include "chunkFileInterfacing.h"
#include "chunk.h"
#include "block.h"
#include "utils.h"
#include "consts.h"
#include "chunkRenderer.h"
#include "queue.h"
#include "dynamicArray.h"

#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <cglm/cglm.h>
#include <string.h>
#include <windows.h>

#define INDEX3D(gcx, y, gcz) ((gcx) * CHUNK_HEIGHT * CHUNK_DEPTH + (y) * CHUNK_DEPTH + (gcz))

uint32_t hash(int seed, float gcx, float y) {
    uint32_t h = seed;

    h ^= (uint32_t)gcx * 0x27d4eb2d;
    h ^= (uint32_t)y * 0x85ebca6b;

    h ^= h >> 15;
    h *= 0x2c1b3c6d;
    h ^= h >> 12;
    h *= 0x297a2d39;
    h ^= h >> 15;

    return h;
}


float getKernel(vec2 dir, vec2 grad) {
    float t = 0.5f - pow(dir[0], 2) - pow(dir[1], 2);
    return (t < 0) ? 0 : pow(t, 4) * glm_vec2_dot(dir, grad);
}

#define F2 ((sqrt(3.0) - 1.0f) / 2.0f)
#define G2 ((3.0f - sqrt(3.0)) / 6.0f)

float generateSimplexNoise(WorldGenerator* worldGenerator, vec2 point) {

    // Simplex-Zelle des Punkts wird bestimmt
    float s = (point[0] + point[1]) * F2;
    vec2 cell = {floor(point[0] + s), floor(point[1] + s)};

    // Koordinaten der ersten Ecke des Simplex werden bestimmt
    float t = (cell[0] + cell[1]) * G2;
    vec2 corner0 = {cell[0] - t, cell[1] - t};

    // Richtungsvektor von der Ecke zum Punkt wird bestimmt
    vec2 dir0;
    glm_vec2_sub(point, corner0, dir0);

    // Welches Dreiecke der Zelle ist es?
    int i1 = (dir0[0] < dir0[1]) ? 0 : 1;
    int j1 = (dir0[0] < dir0[1]) ? 1 : 0;

    // Andere Richtungsvektoren
    vec2 dir1 = {dir0[0] - i1 + G2, dir0[1] - j1 + G2};
    vec2 dir2 = {dir0[0] - 1.0f + 2.0f * G2, dir0[1] - 1.0f + 2.0f * G2};

    // Gradienten werden aufgrund von Hash ausgewählt
    vec2 grad0, grad1, grad2;
    glm_vec2_copy(worldGenerator->gradients[modulo(hash(worldGenerator->seed, cell[0], cell[1]), worldGenerator->gradientCount)], grad0);
    glm_vec2_copy(worldGenerator->gradients[modulo(hash(worldGenerator->seed, cell[0] + i1, cell[1] + j1), worldGenerator->gradientCount)], grad1);
    glm_vec2_copy(worldGenerator->gradients[modulo(hash(worldGenerator->seed, cell[0] + 1, cell[1] + 1), worldGenerator->gradientCount)], grad2);

    return 100.0f * (getKernel(dir0, grad0) + getKernel(dir1, grad1) + getKernel(dir2, grad2));
}

// Frequency beschreibt, wie schnell die Noise-Werte von Punkt zu Punkt variieren. Je höher, desto chaotischer. Je niedriger, desto flacher.
void __stdcall generateChunk(PTP_CALLBACK_INSTANCE instance, void* param, PTP_WORK work) {

    ChunkGenerationWorkArgs* args = (ChunkGenerationWorkArgs*) param;

    // Platz für die Blöcke des Chunks wird allokiert
    uint8_t* blocks = malloc(CHUNK_WIDTH * CHUNK_HEIGHT * CHUNK_DEPTH * sizeof(uint8_t));
    if(blocks == NULL) {
        throwException("Memory for Blocks couldn't be allocated");
    }

    for(int gx = args->chunk->gcx * CHUNK_WIDTH; gx < (args->chunk->gcx + 1) * CHUNK_WIDTH; gx++) {
        int bx = modulo(gx, CHUNK_WIDTH);
        for(int gz = args->chunk->gcz * CHUNK_DEPTH; gz < (args->chunk->gcz + 1) * CHUNK_DEPTH; gz++) {

            // Das Noise für den globalen Punkt wird erzeugt
            float noise = generateSimplexNoise(args->worldGenerator, (vec2) {(float) gx * args->worldGenerator->frequency / 100.0f, (float) gz * args->worldGenerator->frequency / 100.0f});
            int height = (int) floor((CHUNK_HEIGHT / 4 * noise + 3 * CHUNK_HEIGHT / 4) - 1);
            if(height < 0) {
                height = 0;
            } else if(height >= CHUNK_HEIGHT) {
                height = CHUNK_HEIGHT - 1;
            }

            int bz = modulo(gz, CHUNK_DEPTH);
            // Alles unter der Höhe wird mit Erde aufgefüllt
            for(int by = 0; by < height; by++) { 
                blocks[INDEX3D(bx, by, bz)] = BLOCK_DIRT;
            }
            // Auf die Höhe wird ein Grasblock gesetzt
            blocks[INDEX3D(bx, height, bz)] = BLOCK_GRASS;

            // Alles über der Höhe wird mit Luft aufgefüllt
            for(int by = height + 1; by < CHUNK_HEIGHT; by++) {
                blocks[INDEX3D(bx, by, bz)] = BLOCK_AIR;
            }

        }
    }

    updateChunkBlocks(args->chunk, blocks);
    ChunkGenerationWorkResult* result = createChunkGenerationWorkResult(work);
    EnterCriticalSection(args->resultQueueLock);
    enqueueToQueue(args->resultQueue, result);
    LeaveCriticalSection(args->resultQueueLock);
    free(args);
}

WorldGenerator* initWorldGenerator(int gradientCount, int seed, float frequency) {

    WorldGenerator* this = malloc(sizeof(WorldGenerator));
    if(this == NULL) {
        throwException("Memory for worldGenerator couldn't be allocated");
    }

    this->gradientCount = gradientCount;
    this->seed = seed;
    this->frequency = frequency;
    this->gradients = malloc(sizeof(vec2) * gradientCount);
    if(this->gradients == NULL) {
        throwException("Memory for gradients couldn't be allocated");
    }

    // Die möglichen Gradienten werden generiert
    for(int i = 1; i < this->gradientCount + 1; i++) {
        vec2 gradient = {cos(2.0 * M_PI * this->gradientCount / i), sin(2.0 * M_PI * this->gradientCount / i)};
        memcpy(&this->gradients[i - 1], gradient, sizeof(vec2));
    }

    return this;
}

void deinitWorldGenerator(WorldGenerator* this) {
    free(this->gradients);
    free(this);
}
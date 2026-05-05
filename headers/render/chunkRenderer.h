#ifndef CHUNK_RENDERER_H
#define CHUNK_RENDERER_H

#include "chunk.h"
#include "ringBuffer.h"
#include "camera.h"
#include "dynamicArray.h"
#include "queue.h"

#include <cglm/cglm.h>
#include <windows.h>

typedef struct {
    DynamicArray* vertices,* indices;
    Chunk* chunk;
    PTP_WORK work;
} RemeshingWorkResult;

typedef struct {
    Chunk* chunk;
    Queue* resultQueue;
    CRITICAL_SECTION* resultQueueLock;
} RemeshingWorkArgs;

extern ChunkRingBuffer2D* loadedChunks;

void initChunkRenderer();
RemeshingWorkResult* createRemeshingWorkResult(DynamicArray* vertices, DynamicArray* indices, Chunk* chunk, PTP_WORK work);
void remeshLoadedChunks();
void renderChunks(Shader shader, mat4 view, mat4 proj);
void dynamicallyLoadAndUnloadChunks(vec3 lastPlayerPos, vec3 playerPos);
void deinitChunkRenderer();

#endif
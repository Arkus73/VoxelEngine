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
    ChunkRingBuffer2D* loadedChunks;
    uint8_t* voidChunkData;
    Queue* resultQueue;
    CRITICAL_SECTION resultQueueLock;
    DynamicArray* currentRemeshingWorkBatch;
    DynamicArray* queuedChunkMovements;
} ChunkRenderer;

typedef struct {
    Chunk* chunk;
    Queue* resultQueue;
    CRITICAL_SECTION* resultQueueLock;
    ChunkRingBuffer2D* loadedChunks;
} RemeshingWorkArgs;

ChunkRenderer* initChunkRenderer();
void deinitChunkRenderer(ChunkRenderer* chunkRenderer);
RemeshingWorkResult* createRemeshingWorkResult(DynamicArray* vertices, DynamicArray* indices, Chunk* chunk, PTP_WORK work);
void remeshLoadedChunks(ChunkRenderer* chunkRenderer);
void renderChunks(ChunkRenderer* chunkRenderer, Shader shader, mat4 view, mat4 proj);
void dynamicallyLoadAndUnloadChunks(ChunkRenderer* chunkRenderer, vec3 lastPlayerPos, vec3 playerPos);

#endif
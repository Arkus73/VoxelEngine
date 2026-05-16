#ifndef CHUNK_RENDERER_H
#define CHUNK_RENDERER_H

#include "chunk.h"
#include "ringBuffer.h"
#include "camera.h"
#include "dynamicArray.h"
#include "queue.h"
#include "worldGenerator.h"

#include <cglm/cglm.h>
#include <windows.h>

typedef struct {
    ChunkRingBuffer2D* loadedChunks;

    Queue* remeshingWorkResultQueue;
    CRITICAL_SECTION remeshingWorkResultQueueLock;
    Queue* chunkGenerationWorkResultQueue;
    CRITICAL_SECTION chunkGenerationWorkResultQueueLock;

    DynamicArray* remeshingWorkBatch;
    DynamicArray* chunkGenerationWorkBatch;
    DynamicArray* queuedChunkMovements;
} ChunkRenderer;


typedef struct {
    Chunk* chunk;
    Queue* resultQueue;
    CRITICAL_SECTION* resultQueueLock;
    ChunkRingBuffer2D* loadedChunks;
} RemeshingWorkArgs;

typedef struct {
    DynamicArray* vertices,* indices;
    Chunk* chunk;
    PTP_WORK work;
} RemeshingWorkResult;


typedef struct {
    WorldGenerator* worldGenerator;
    Chunk* chunk;
    Queue* resultQueue;
    CRITICAL_SECTION* resultQueueLock;
} ChunkGenerationWorkArgs;

typedef struct {
    PTP_WORK work;
} ChunkGenerationWorkResult;

ChunkRenderer* initChunkRenderer();
void deinitChunkRenderer(ChunkRenderer* chunkRenderer);

RemeshingWorkResult* createRemeshingWorkResult(DynamicArray* vertices, DynamicArray* indices, Chunk* chunk, PTP_WORK work);
ChunkGenerationWorkResult* createChunkGenerationWorkResult(PTP_WORK work);

void remeshLoadedChunks(ChunkRenderer* chunkRenderer);
void generateLoadedChunks(ChunkRenderer* chunkRenderer, WorldGenerator* worldGenerator);
void renderChunks(ChunkRenderer* chunkRenderer, Shader shader, mat4 view, mat4 proj);
void dynamicallyLoadAndUnloadChunks(ChunkRenderer* chunkRenderer, WorldGenerator* worldGenerator, vec3 lastPlayerPos, vec3 playerPos);

#endif
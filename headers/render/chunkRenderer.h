#ifndef CHUNK_RENDERER_H
#define CHUNK_RENDERER_H

#include "chunk.h"
#include "ringBuffer.h"

extern ChunkRingBuffer2D* loadedChunks;

void initChunkRenderer();
void remeshLoadedChunks();
void renderChunks(Shader shader);
void dynamicallyLoadAndUnloadChunks(vec3 lastPlayerPos, vec3 playerPos);
void destroyChunks();

#endif
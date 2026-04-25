#ifndef CHUNK_RENDERER_H
#define CHUNK_RENDERER_H

#include "chunk.h"
#include "ringBuffer.h"
#include "camera.h"

#include <cglm/cglm.h>

extern ChunkRingBuffer2D* loadedChunks;

void initChunkRenderer();
void remeshLoadedChunks();
void renderChunks(Shader shader, mat4 view, mat4 proj);
void dynamicallyLoadAndUnloadChunks(vec3 lastPlayerPos, vec3 playerPos);
void destroyChunks();

#endif
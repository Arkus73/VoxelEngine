#ifndef CHUNK_RENDERER_H
#define CHUNK_RENDERER_H

#include "chunk.h"

#define RENDER_DISTANCE 3

extern Chunk** loadedChunks;

void initChunkRenderer();
void initChunkMeshes();
void renderChunks(Shader shader);
void destroyChunks();

#endif
#ifndef CHUNK_H
#define CHUNK_H

#include "mesh.h"
#include "block.h"
#include "shader.h"

#define CHUNK_WIDTH 16
#define CHUNK_HEIGHT 256
#define CHUNK_DEPTH 16

typedef enum {
    FRONT,
    BACK,
    TOP,
    BOTTOM,
    RIGHT,
    LEFT
} Face;

typedef struct {
    Mesh* mesh; // Das Mesh des Chunks
    Block* blocks;  // Array mit allen Blöcken des Chunks
    float x, z;  // Koordinaten des Chunks
} Chunk;

Chunk* createChunk(Block* blocks, float x, float z);
void destroyChunk(Chunk* this);
void updateChunkMesh(Chunk* this);  // Updated das Chunk-Mesh aufgrund des block-Arrays
void renderChunk(Chunk* this, Shader shader);

#endif
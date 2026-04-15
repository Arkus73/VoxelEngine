#ifndef CHUNK_H
#define CHUNK_H

#include "mesh.h"
#include "shader.h"

#include <stdint.h>

#define CHUNK_WIDTH 8
#define CHUNK_HEIGHT 16
#define CHUNK_DEPTH 8

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
    uint8_t* blocks;  // Array mit allen Blöcken des Chunks, gespeichert als Byte-große Block-IDs
    int x, z;  // Koordinaten des Chunks
} Chunk;

Chunk* createChunk(uint8_t* blocks, int x, int z);
void destroyChunk(Chunk* this);
void updateChunkMesh(Chunk* this);  // Updated das Chunk-Mesh aufgrund des block-Arrays
void renderChunk(Chunk* this, Shader shader);

#endif
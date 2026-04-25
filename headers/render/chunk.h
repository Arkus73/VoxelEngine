#ifndef CHUNK_H
#define CHUNK_H

#include "mesh.h"
#include "shader.h"

#include <stdint.h>

typedef enum {
    FRONT = 0,
    BACK = 1,
    TOP = 2,
    BOTTOM = 3,
    RIGHT = 4,
    LEFT = 5
} Face;

typedef struct {
    Mesh* mesh; // Das Mesh des Chunks
    uint8_t* blocks;  // Array mit allen Blöcken des Chunks, gespeichert als Byte-große Block-IDs
    int x, z;  // Koordinaten des Chunks in gc
} Chunk;

Chunk* createChunk(uint8_t* blocks, int gcx, int gcz);
void updateChunk(Chunk* this, uint8_t* blocks, int gcx, int gcz);
void destroyChunk(Chunk* this);
void remeshChunk(Chunk* this);  // Updated das Chunk-Mesh aufgrund des block-Arrays
void renderChunk(Chunk* this, Shader shader);

#endif
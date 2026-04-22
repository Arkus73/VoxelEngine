#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include "chunk.h"
#include "consts.h"

typedef struct {
    Chunk** buffer;    // Der tatsächliche Array mit den Chunks
    int rowLen;    // Die Länge des Arrays
    int startX, startZ; // Die Indices, die 0, 0 entsprechen
    int offsetX, offsetZ;   // Der Ursprung der lokalen Chunks. 0, 0 in gc für lc
} ChunkRingBuffer2D;

ChunkRingBuffer2D* createChunkRingBuffer2D();
void destroyChunkRingBuffer2D(ChunkRingBuffer2D* this);
Chunk* getFromChunkRingBuffer2D(ChunkRingBuffer2D* this, int lcx, int lcz);
void writeToChunkRingBuffer2D(ChunkRingBuffer2D* this, int lcx, int lcz, Chunk* chunk);
void incrementStartX(ChunkRingBuffer2D* this);
void incrementStartZ(ChunkRingBuffer2D* this);
void decrementStartX(ChunkRingBuffer2D* this);
void decrementStartZ(ChunkRingBuffer2D* this);

#endif
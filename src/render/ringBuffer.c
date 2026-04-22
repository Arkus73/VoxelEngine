#include "ringBuffer.h"
#include "utils.h"
#include "stdlib.h"

ChunkRingBuffer2D* createChunkRingBuffer2D() {

    ChunkRingBuffer2D* this = malloc(sizeof(ChunkRingBuffer2D));
    if(this == NULL) {
        throwException("Memory for ChunkRingBuffer2D couldn't be allocated");
    }

    this->buffer = malloc(pow(2 * RENDER_DISTANCE + 1, 2) * sizeof(Chunk*));
    if(this == NULL) {
        throwException("Memory for ChunkRingBuffer2D couldn't be allocated");
    }

    this->startX = this->startZ = 0;
    this->rowLen = 2 * RENDER_DISTANCE + 1;
    this->offsetX = this->offsetZ = -RENDER_DISTANCE;

    return this;
}

void destroyChunkRingBuffer2D(ChunkRingBuffer2D* this) {
    free(this);
}

Chunk* getFromChunkRingBuffer2D(ChunkRingBuffer2D* this, int lcx, int lcz) {
    if(lcx < 0 || lcx >= this->rowLen || lcz < 0 || lcz >= this->rowLen) {
        return NULL;
    }
    return this->buffer[((this->startX + lcx) % this->rowLen) * this->rowLen + (this->startZ + lcz) % this->rowLen];
}

void writeToChunkRingBuffer2D(ChunkRingBuffer2D* this, int lcx, int lcz, Chunk* chunk) {
    this->buffer[((this->startX + lcx) % this->rowLen) * this->rowLen + (this->startZ + lcz) % this->rowLen] = chunk;
}

void incrementStartX(ChunkRingBuffer2D* this) {
    this->startX = (this->startX + 1) % this->rowLen;
}

void incrementStartZ(ChunkRingBuffer2D* this) {
    this->startZ = (this->startZ + 1) % this->rowLen;
}

void decrementStartX(ChunkRingBuffer2D* this) {
    this->startX = (this->startX - 1 + this->rowLen) % this->rowLen;
}

void decrementStartZ(ChunkRingBuffer2D* this) {
    this->startZ = (this->startZ - 1 + this->rowLen) % this->rowLen;
}
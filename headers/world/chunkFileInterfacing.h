#ifndef CHUNK_FILE_INTERFACING_H
#define CHUNK_FILE_INTERFACING_H

#include "chunk.h"

#include <stdint.h>

void saveChunk(uint8_t* blocks, int x, int z);
uint8_t* loadChunkData(int x, int z);

#endif
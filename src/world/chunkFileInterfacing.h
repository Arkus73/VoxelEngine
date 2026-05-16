#ifndef CHUNK_FILE_INTERFACING_H
#define CHUNK_FILE_INTERFACING_H

#include "chunk.h"

#include <stdint.h>

void saveChunk(uint8_t* blocks, int gcx, int gcz);
uint8_t* loadChunkData(int gcx, int gcz);

#endif
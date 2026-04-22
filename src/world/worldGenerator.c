#include "worldGenerator.h"
#include "chunkFileInterfacing.h"
#include "chunk.h"
#include "block.h"
#include "utils.h"
#include "consts.h"

#include <stdint.h>
#include <stdlib.h>

void generateWorld() {

    uint8_t* blocks = malloc(CHUNK_WIDTH * CHUNK_HEIGHT * CHUNK_DEPTH * sizeof(uint8_t));
    if(blocks == NULL) {
        throwException("Memory for blocks in order of func \"generateWorld()\" couldn't be allocated");
    }

    for(int bx = 0; bx < CHUNK_WIDTH; bx++) {
        for(int by = 0; by < CHUNK_HEIGHT - 1; by++) {
            for(int bz = 0; bz < CHUNK_DEPTH; bz++) {
                blocks[bx * CHUNK_HEIGHT * CHUNK_DEPTH + by * CHUNK_DEPTH + bz] = BLOCK_DIRT;
            }
        }
    }
    for(int bx = 0; bx < CHUNK_WIDTH; bx++) {
        for(int bz = 0; bz < CHUNK_DEPTH; bz++) {
            blocks[bx * CHUNK_HEIGHT * CHUNK_DEPTH + (CHUNK_HEIGHT - 1) * CHUNK_DEPTH + bz] = BLOCK_GRASS;
        }
    }
    
    system("del /Q ..\\chunks\\*.bin"); // system() führt den Batch-Command aus, den man als Argument in den String schreibt
    for(int gcx = -WORLD_WIDTH / 2; gcx < WORLD_WIDTH / 2 + 1; gcx++) {
        for(int gcz = -WORLD_DEPTH / 2; gcz < WORLD_DEPTH / 2 + 1; gcz++) {
            saveChunk(blocks, gcx, gcz);
        }
    }
    free(blocks);
}
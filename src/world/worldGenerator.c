#include "worldGenerator.h"
#include "chunkFileInterfacing.h"
#include "chunk.h"
#include "block.h"
#include "utils.h"

#include <stdint.h>
#include <stdlib.h>

void generateWorld() {

    uint8_t* blocks = malloc(CHUNK_WIDTH * CHUNK_HEIGHT * CHUNK_DEPTH * sizeof(uint8_t));
    if(blocks == NULL) {
        throwException("Memory for blocks in order of func \"generateWorld()\" couldn't be allocated");
    }

    for(int x = 0; x < CHUNK_WIDTH; x++) {
        for(int y = 0; y < CHUNK_HEIGHT - 1; y++) {
            for(int z = 0; z < CHUNK_DEPTH; z++) {
                blocks[x * CHUNK_HEIGHT * CHUNK_DEPTH + y * CHUNK_DEPTH + z] = BLOCK_DIRT;
            }
        }
    }
    for(int x = 0; x < CHUNK_WIDTH; x++) {
        for(int z = 0; z < CHUNK_DEPTH; z++) {
            blocks[x * CHUNK_HEIGHT * CHUNK_DEPTH + (CHUNK_HEIGHT - 1) * CHUNK_DEPTH + z] = BLOCK_GRASS;
        }
    }
    
    for(int x = 0; x < 3; x++) {
        for(int z = 0; z < 3; z++) {
            saveChunk(blocks, x, z);
        }
    }
    free(blocks);
}
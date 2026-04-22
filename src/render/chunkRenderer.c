#include "chunkRenderer.h"
#include "chunkFileInterfacing.h"
#include "ringBuffer.h"
#include "consts.h"
#include "utils.h"
#include "block.h"

#include <stdlib.h>

ChunkRingBuffer2D* loadedChunks;

void initChunkRenderer() {
    loadedChunks = createChunkRingBuffer2D();
    for(int lcx = 0; lcx < loadedChunks->rowLen; lcx++) {
        for(int lcz = 0; lcz < loadedChunks->rowLen; lcz++) {
            writeToChunkRingBuffer2D(loadedChunks, lcx, lcz, createChunk(loadChunkData(lcx + loadedChunks->offsetX, lcz + loadedChunks->offsetZ), lcx + loadedChunks->offsetX, lcz + loadedChunks->offsetZ));
        }
    }
}

void initChunkMeshes() {
    for(int lcx = 0; lcx < loadedChunks->rowLen; lcx++) {
        for(int lcz = 0; lcz < loadedChunks->rowLen; lcz++) {
            updateChunkMesh(getFromChunkRingBuffer2D(loadedChunks, lcx, lcz));
        }
    }
}

void renderChunks(Shader shader) {
    for(int lcx = 0; lcx < loadedChunks->rowLen; lcx++) {
        for(int lcz = 0; lcz < loadedChunks->rowLen; lcz++) {
            renderChunk(getFromChunkRingBuffer2D(loadedChunks, lcx, lcz), shader);
        }
    }
}

void getChunkPos(vec3 pos, vec2 dest) {
    vec2 chunkPos = {floor(pos[0] / CHUNK_WIDTH), floor(pos[2] / CHUNK_DEPTH)};
    glm_vec2_copy(chunkPos, dest);
}

bool isChunkValid(int lcx, int lcz) {
    int gcx = loadedChunks->offsetX + lcx;
    int gcz = loadedChunks->offsetZ + lcz;
    return (gcx <= WORLD_WIDTH / 2 && gcx >= -WORLD_WIDTH / 2) && (gcz <= WORLD_DEPTH / 2 && gcz >= -WORLD_DEPTH / 2);
}

uint8_t* generateVoid() {
    uint8_t* blocks = malloc(CHUNK_WIDTH * CHUNK_HEIGHT * CHUNK_DEPTH * sizeof(uint8_t));
    for(int lcx = 0; lcx < CHUNK_WIDTH; lcx++) {
        for(int y = 0; y < CHUNK_HEIGHT; y++) {
            for(int lcz = 0; lcz < CHUNK_DEPTH; lcz++) {
                blocks[lcx * CHUNK_HEIGHT * CHUNK_DEPTH + y * CHUNK_DEPTH + lcz] = BLOCK_AIR;
            }
        }
    }
    return blocks;
}

void dynamicallyLoadAndUnloadChunks(vec3 lastPlayerPos, vec3 playerPos) {
    vec2 lastChunkPos, chunkPos;
    getChunkPos(lastPlayerPos, lastChunkPos);
    getChunkPos(playerPos, chunkPos);

    if(chunkPos[0] > lastChunkPos[0]) {
        loadedChunks->offsetX++;
        for(int lcz = 0; lcz < loadedChunks->rowLen; lcz++) {
            destroyChunk(getFromChunkRingBuffer2D(loadedChunks, 0, lcz));
            if(!isChunkValid(loadedChunks->rowLen - 1, lcz)) {
                writeToChunkRingBuffer2D(loadedChunks, 0, lcz, createChunk(generateVoid(), loadedChunks->offsetX + loadedChunks->rowLen - 1, lcz + loadedChunks->offsetZ));
            } else {
                writeToChunkRingBuffer2D(loadedChunks, 0, lcz, createChunk(loadChunkData(loadedChunks->offsetX + loadedChunks->rowLen - 1, lcz + loadedChunks->offsetZ), loadedChunks->offsetX + loadedChunks->rowLen - 1, lcz + loadedChunks->offsetZ));
            }
        }
        incrementStartX(loadedChunks);
        for(int lcx = 0; lcx < loadedChunks->rowLen; lcx++) {
            for(int lcz = 0; lcz < loadedChunks->rowLen; lcz++) {
                updateChunkMesh(getFromChunkRingBuffer2D(loadedChunks, lcx, lcz));
            }
        }
    }

    if(chunkPos[0] < lastChunkPos[0]) {
        loadedChunks->offsetX--;
        for(int lcz = 0; lcz < loadedChunks->rowLen; lcz++) {
            destroyChunk(getFromChunkRingBuffer2D(loadedChunks, loadedChunks->rowLen - 1, lcz));
            if(!isChunkValid(0, lcz)) {
                writeToChunkRingBuffer2D(loadedChunks, loadedChunks->rowLen - 1, lcz, createChunk(generateVoid(), loadedChunks->offsetX, lcz + loadedChunks->offsetZ));
            } else{
                writeToChunkRingBuffer2D(loadedChunks, loadedChunks->rowLen - 1, lcz, createChunk(loadChunkData(loadedChunks->offsetX, lcz + loadedChunks->offsetZ), loadedChunks->offsetX, lcz + loadedChunks->offsetZ));
            }
        }
        decrementStartX(loadedChunks);
        for(int lcx = 0; lcx < loadedChunks->rowLen; lcx++) {
            for(int lcz = 0; lcz < loadedChunks->rowLen; lcz++) {
                updateChunkMesh(getFromChunkRingBuffer2D(loadedChunks, lcx, lcz));
            }
        }
    }

    if(chunkPos[1] > lastChunkPos[1]) {
        loadedChunks->offsetZ++;
        for(int lcx = 0; lcx < loadedChunks->rowLen; lcx++) {
            destroyChunk(getFromChunkRingBuffer2D(loadedChunks, lcx, 0));
            if(!isChunkValid(lcx, loadedChunks->rowLen - 1)) {
                writeToChunkRingBuffer2D(loadedChunks, lcx, 0, createChunk(generateVoid(), lcx + loadedChunks->offsetX, loadedChunks->offsetZ + loadedChunks->rowLen - 1));
            } else {
                writeToChunkRingBuffer2D(loadedChunks, lcx, 0, createChunk(loadChunkData(lcx + loadedChunks->offsetX, loadedChunks->offsetZ + loadedChunks->rowLen - 1), lcx + loadedChunks->offsetX, loadedChunks->offsetZ + loadedChunks->rowLen - 1));
            }
        }
        incrementStartZ(loadedChunks);
        for(int lcx = 0; lcx < loadedChunks->rowLen; lcx++) {
            for(int lcz = 0; lcz < loadedChunks->rowLen; lcz++) {
                updateChunkMesh(getFromChunkRingBuffer2D(loadedChunks, lcx, lcz));
            }
        }
    }

    if(chunkPos[1] < lastChunkPos[1]) {
        loadedChunks->offsetZ--;
        for(int lcx = 0; lcx < loadedChunks->rowLen; lcx++) {
            destroyChunk(getFromChunkRingBuffer2D(loadedChunks, lcx, loadedChunks->rowLen - 1));
            if(!isChunkValid(lcx, 0)) {
                writeToChunkRingBuffer2D(loadedChunks, lcx, loadedChunks->rowLen - 1, createChunk(generateVoid(), lcx + loadedChunks->offsetX, loadedChunks->offsetZ));
            } else {
                writeToChunkRingBuffer2D(loadedChunks, lcx, loadedChunks->rowLen - 1, createChunk(loadChunkData(lcx + loadedChunks->offsetX, loadedChunks->offsetZ), lcx + loadedChunks->offsetX, loadedChunks->offsetZ));
            }
        }
        decrementStartZ(loadedChunks);
        for(int lcx = 0; lcx < loadedChunks->rowLen; lcx++) {
            for(int lcz = 0; lcz < loadedChunks->rowLen; lcz++) {
                updateChunkMesh(getFromChunkRingBuffer2D(loadedChunks, lcx, lcz));
            }
        }
    }
}

void destroyChunks() {
    for(int lcx = 0; lcx < loadedChunks->rowLen; lcx++) {
        for(int lcz = 0; lcz < loadedChunks->rowLen; lcz++) {
            destroyChunk(getFromChunkRingBuffer2D(loadedChunks, lcx, lcz));
        }
    }
    destroyChunkRingBuffer2D(loadedChunks);
}
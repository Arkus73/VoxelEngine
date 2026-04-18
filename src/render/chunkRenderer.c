#include "chunkRenderer.h"
#include "chunkFileInterfacing.h"
#include "ringBuffer.h"
#include "consts.h"
#include "utils.h"

#include <stdlib.h>

ChunkRingBuffer2D* loadedChunks;

void initChunkRenderer() {
    loadedChunks = createChunkRingBuffer2D();
    for(int x = 0; x < loadedChunks->rowLen; x++) {
        for(int z = 0; z < loadedChunks->rowLen; z++) {
            writeToChunkRingBuffer2D(loadedChunks, x, z, createChunk(loadChunkData(x + loadedChunks->offsetX, z + loadedChunks->offsetZ), x + loadedChunks->offsetX, z + loadedChunks->offsetZ));
        }
    }
}

void initChunkMeshes() {
    for(int x = 0; x < loadedChunks->rowLen; x++) {
        for(int z = 0; z < loadedChunks->rowLen; z++) {
            updateChunkMesh(getFromChunkRingBuffer2D(loadedChunks, x, z));
        }
    }
}

void renderChunks(Shader shader) {
    for(int x = 0; x < loadedChunks->rowLen; x++) {
        for(int z = 0; z < loadedChunks->rowLen; z++) {
            renderChunk(getFromChunkRingBuffer2D(loadedChunks, x, z), shader);
        }
    }
}

void getChunkPos(vec3 pos, vec2 dest) {
    vec2 chunkPos = {floor(pos[0] / CHUNK_WIDTH), floor(pos[2] / CHUNK_DEPTH)};
    glm_vec2_copy(chunkPos, dest);
}

void dynamicallyLoadAndUnloadChunks(vec3 lastPlayerPos, vec3 playerPos) {
    vec2 lastChunkPos, chunkPos;
    getChunkPos(lastPlayerPos, lastChunkPos);
    getChunkPos(playerPos, chunkPos);

    if(chunkPos[0] > lastChunkPos[0]) {
        loadedChunks->offsetX++;
        for(int z = 0; z < loadedChunks->rowLen; z++) {
            destroyChunk(getFromChunkRingBuffer2D(loadedChunks, 0, z));
            writeToChunkRingBuffer2D(loadedChunks, 0, z, createChunk(loadChunkData(loadedChunks->offsetX + loadedChunks->rowLen - 1, z + loadedChunks->offsetZ), loadedChunks->offsetX + loadedChunks->rowLen - 1, z + loadedChunks->offsetZ));
        }
        for(int z = 0; z < loadedChunks->rowLen; z++) {
            updateChunkMesh(getFromChunkRingBuffer2D(loadedChunks, 0, z));
        }
        incrementStartX(loadedChunks);
    }

    if(chunkPos[0] < lastChunkPos[0]) {
        loadedChunks->offsetX--;
        for(int z = 0; z < loadedChunks->rowLen; z++) {
            destroyChunk(getFromChunkRingBuffer2D(loadedChunks, loadedChunks->rowLen - 1, z));
            writeToChunkRingBuffer2D(loadedChunks, loadedChunks->rowLen - 1, z, createChunk(loadChunkData(loadedChunks->offsetX, z + loadedChunks->offsetZ), loadedChunks->offsetX, z + loadedChunks->offsetZ));
        }
        for(int z = 0; z < loadedChunks->rowLen; z++) {
            updateChunkMesh(getFromChunkRingBuffer2D(loadedChunks, loadedChunks->rowLen - 1, z));
        }
        decrementStartX(loadedChunks);
    }

    if(chunkPos[1] > lastChunkPos[1]) {
        loadedChunks->offsetZ++;
        for(int x = 0; x < loadedChunks->rowLen; x++) {
            destroyChunk(getFromChunkRingBuffer2D(loadedChunks, x, 0));
            writeToChunkRingBuffer2D(loadedChunks, x, 0, createChunk(loadChunkData(x + loadedChunks->offsetX, loadedChunks->offsetZ + loadedChunks->rowLen - 1), x + loadedChunks->offsetX, loadedChunks->offsetZ + loadedChunks->rowLen - 1));
        }
        for(int x = 0; x < loadedChunks->rowLen; x++) {
            updateChunkMesh(getFromChunkRingBuffer2D(loadedChunks, x, 0));
        }
        incrementStartZ(loadedChunks);
    }

    if(chunkPos[1] < lastChunkPos[1]) {
        loadedChunks->offsetZ--;
        for(int x = 0; x < loadedChunks->rowLen; x++) {
            destroyChunk(getFromChunkRingBuffer2D(loadedChunks, x, loadedChunks->rowLen - 1));
            writeToChunkRingBuffer2D(loadedChunks, x, loadedChunks->rowLen - 1, createChunk(loadChunkData(x + loadedChunks->offsetX, loadedChunks->offsetZ), x + loadedChunks->offsetX, loadedChunks->offsetZ));
        }
        for(int x = 0; x < loadedChunks->rowLen; x++) {
            updateChunkMesh(getFromChunkRingBuffer2D(loadedChunks, x, loadedChunks->rowLen - 1));
        }
        decrementStartZ(loadedChunks);
    }
}

void destroyChunks() {
    for(int x = 0; x < loadedChunks->rowLen; x++) {
        for(int z = 0; z < loadedChunks->rowLen; z++) {
            destroyChunk(getFromChunkRingBuffer2D(loadedChunks, x, z));
        }
    }
    destroyChunkRingBuffer2D(loadedChunks);
}
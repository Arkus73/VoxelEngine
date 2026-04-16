#include "chunkRenderer.h"
#include "chunkFileInterfacing.h"

#include <stdlib.h>

Chunk** loadedChunks;

void initChunkRenderer() {
    loadedChunks = malloc(RENDER_DISTANCE * RENDER_DISTANCE * sizeof(Chunk*));
    for(int x = 0; x < RENDER_DISTANCE; x++) {
        for(int z = 0; z < RENDER_DISTANCE; z++) {
            loadedChunks[x * RENDER_DISTANCE + z] = createChunk(loadChunkData(x, z), x, z);
        }
    }
}

void initChunkMeshes() {
    for(int x = 0; x < RENDER_DISTANCE; x++) {
        for(int z = 0; z < RENDER_DISTANCE; z++) {
            updateChunkMesh(loadedChunks[x * RENDER_DISTANCE + z]);
        }
    }
}

void renderChunks(Shader shader) {
    for(int x = 0; x < RENDER_DISTANCE; x++) {
        for(int z = 0; z < RENDER_DISTANCE; z++) {
            renderChunk(loadedChunks[x * RENDER_DISTANCE + z], shader);
        }
    }
}

void destroyChunks() {
    for(int x = 0; x < 3; x++) {
        for(int z = 0; z < 3; z++) {
            destroyChunk(loadedChunks[x * RENDER_DISTANCE + z]);
        }
    }
    free(loadedChunks);
}
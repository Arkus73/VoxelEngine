#include "chunkRenderer.h"
#include "chunkFileInterfacing.h"
#include "ringBuffer.h"
#include "consts.h"
#include "utils.h"
#include "block.h"

#include <stdlib.h>

ChunkRingBuffer2D* loadedChunks;
uint8_t* voidChunkData;

void initChunkRenderer() {
    loadedChunks = createChunkRingBuffer2D();   // Die geladenen Chunks werden initialisiert
    for(int lcx = 0; lcx < loadedChunks->rowLen; lcx++) {
        for(int lcz = 0; lcz < loadedChunks->rowLen; lcz++) {
            writeToChunkRingBuffer2D(loadedChunks, lcx, lcz, createChunk(loadChunkData(lcx + loadedChunks->offsetX, lcz + loadedChunks->offsetZ), lcx + loadedChunks->offsetX, lcz + loadedChunks->offsetZ));
        }
    }
    voidChunkData = malloc(CHUNK_WIDTH * CHUNK_HEIGHT * CHUNK_DEPTH * sizeof(uint8_t)); // Die VoidChunk-Daten (-> Chunk, der nur aus Luftblöcken besteht) werden generiert
    for(int lcx = 0; lcx < CHUNK_WIDTH; lcx++) {
        for(int y = 0; y < CHUNK_HEIGHT; y++) {
            for(int lcz = 0; lcz < CHUNK_DEPTH; lcz++) {
                voidChunkData[lcx * CHUNK_HEIGHT * CHUNK_DEPTH + y * CHUNK_DEPTH + lcz] = BLOCK_AIR;
            }
        }
    }
}

void remeshLoadedChunks() {
    for(int lcx = 0; lcx < loadedChunks->rowLen; lcx++) {
        for(int lcz = 0; lcz < loadedChunks->rowLen; lcz++) {
            remeshChunk(getFromChunkRingBuffer2D(loadedChunks, lcx, lcz));
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

void getChunkPos(vec3 pos, vec2 dest) { // Wandelt die g-Koordinaten von cam->pos in gc-Koordinaten um
    vec2 chunkPos = {floor(pos[0] / CHUNK_WIDTH), floor(pos[2] / CHUNK_DEPTH)};
    glm_vec2_copy(chunkPos, dest);
}

bool isChunkValid(int lcx, int lcz) {   // Prüft, ob sich die lc-Koordinaten innerhalb der Welt befinden
    int gcx = loadedChunks->offsetX + lcx;
    int gcz = loadedChunks->offsetZ + lcz;
    return (gcx <= WORLD_WIDTH / 2 && gcx >= -WORLD_WIDTH / 2) && (gcz <= WORLD_DEPTH / 2 && gcz >= -WORLD_DEPTH / 2);
}

void dynamicallyLoadAndUnloadChunks(vec3 lastPlayerPos, vec3 playerPos) {

    // Die g-Spielerkoordinaten werden in gc-Koordinaten umgewandelt
    vec2 lastChunkPos, chunkPos;
    getChunkPos(lastPlayerPos, lastChunkPos);
    getChunkPos(playerPos, chunkPos);

    if(chunkPos[0] > lastChunkPos[0]) { // Hat sich die gcx-Koordinate des Spielers ins positive verändert?
        loadedChunks->offsetX++;    // Der Ringbuffer wird verschoben
        for(int lcz = 0; lcz < loadedChunks->rowLen; lcz++) { // Es wird über die zu verändernde Spalte iteriert
            if(!isChunkValid(loadedChunks->rowLen - 1, lcz)) {  // Kann der zum Laden benötigte Chunk geladen werden?
                updateChunk(getFromChunkRingBuffer2D(loadedChunks, 0, lcz), voidChunkData, loadedChunks->offsetX + loadedChunks->rowLen - 1, lcz + loadedChunks->offsetZ);  // Nein: Der Chunk wird zu einem VoidChunk
            } else {
                updateChunk(getFromChunkRingBuffer2D(loadedChunks, 0, lcz), loadChunkData(loadedChunks->offsetX + loadedChunks->rowLen - 1, lcz + loadedChunks->offsetZ), loadedChunks->offsetX + loadedChunks->rowLen - 1, lcz + loadedChunks->offsetZ);   // Ja: Die Blockdaten des benötigten Chunks werden aus dem ChunkFile geladen und in den Chunk mit seinen neuen gc-Koordinaten eingesetzt
            }
        }
        incrementStartX(loadedChunks);  // Der Pointer, der zum ersten x-Element zeigt wird inkrementiert (-> Ringbuffer wird verschoben)
        remeshLoadedChunks(); // Die geladenen Chunks werden geremesht, damit die Änderungen in die Meshes übernommen werden
    }

    if(chunkPos[0] < lastChunkPos[0]) {
        loadedChunks->offsetX--;
        for(int lcz = 0; lcz < loadedChunks->rowLen; lcz++) {
            if(!isChunkValid(0, lcz)) {
                updateChunk(getFromChunkRingBuffer2D(loadedChunks, loadedChunks->rowLen - 1, lcz), voidChunkData, loadedChunks->offsetX, lcz + loadedChunks->offsetZ); 
            } else{
                updateChunk(getFromChunkRingBuffer2D(loadedChunks, loadedChunks->rowLen - 1, lcz), loadChunkData(loadedChunks->offsetX, lcz + loadedChunks->offsetZ), loadedChunks->offsetX, lcz + loadedChunks->offsetZ); 
            }
        }
        decrementStartX(loadedChunks);
        remeshLoadedChunks();
    }

    if(chunkPos[1] > lastChunkPos[1]) {
        loadedChunks->offsetZ++;
        for(int lcx = 0; lcx < loadedChunks->rowLen; lcx++) {
            if(!isChunkValid(lcx, loadedChunks->rowLen - 1)) {
                updateChunk(getFromChunkRingBuffer2D(loadedChunks, lcx, 0), voidChunkData, lcx + loadedChunks->offsetX, loadedChunks->offsetZ + loadedChunks->rowLen - 1);
            } else {
                updateChunk(getFromChunkRingBuffer2D(loadedChunks, lcx, 0), loadChunkData(lcx + loadedChunks->offsetX, loadedChunks->offsetZ + loadedChunks->rowLen - 1), lcx + loadedChunks->offsetX, loadedChunks->offsetZ + loadedChunks->rowLen - 1);
            }
        }
        incrementStartZ(loadedChunks);
        remeshLoadedChunks();
    }

    if(chunkPos[1] < lastChunkPos[1]) {
        loadedChunks->offsetZ--;
        for(int lcx = 0; lcx < loadedChunks->rowLen; lcx++) {
            if(!isChunkValid(lcx, 0)) {
                updateChunk(getFromChunkRingBuffer2D(loadedChunks, lcx, loadedChunks->rowLen - 1), voidChunkData, lcx + loadedChunks->offsetX, loadedChunks->offsetZ);
            } else {
                updateChunk(getFromChunkRingBuffer2D(loadedChunks, lcx, loadedChunks->rowLen - 1), loadChunkData(lcx + loadedChunks->offsetX, loadedChunks->offsetZ), lcx + loadedChunks->offsetX, loadedChunks->offsetZ);
            }
        }
        decrementStartZ(loadedChunks);
        remeshLoadedChunks();
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
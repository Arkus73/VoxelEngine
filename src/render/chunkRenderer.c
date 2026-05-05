#include "chunkRenderer.h"
#include "chunkFileInterfacing.h"
#include "ringBuffer.h"
#include "consts.h"
#include "utils.h"
#include "block.h"
#include "camera.h"
#include "queue.h"

#include <cglm/cglm.h>
#include <stdlib.h>
#include <windows.h>

ChunkRingBuffer2D* loadedChunks;
uint8_t* voidChunkData;
Queue* resultQueue;
CRITICAL_SECTION resultQueueLock;
DynamicArray* currentRemeshingWorkBatch;
DynamicArray* queuedChunkMovements;

void initChunkRenderer() {
    loadedChunks = createChunkRingBuffer2D();   // Die geladenen Chunks werden initialisiert
    for(int lcx = 0; lcx < loadedChunks->rowLen; lcx++) {
        for(int lcz = 0; lcz < loadedChunks->rowLen; lcz++) {
            writeToChunkRingBuffer2D(loadedChunks, lcx, lcz, createChunk(loadChunkData(lcx + loadedChunks->offsetX, lcz + loadedChunks->offsetZ), lcx + loadedChunks->offsetX, lcz + loadedChunks->offsetZ));
        }
    }
    voidChunkData = malloc(CHUNK_WIDTH * CHUNK_HEIGHT * CHUNK_DEPTH * sizeof(uint8_t)); // Die VoidChunk-Daten (-> Chunk, der nur aus Luftblöcken besteht) werden generiert
    for(int bx = 0; bx < CHUNK_WIDTH; bx++) {
        for(int by = 0; by < CHUNK_HEIGHT; by++) {
            for(int bz = 0; bz < CHUNK_DEPTH; bz++) {
                voidChunkData[bx * CHUNK_HEIGHT * CHUNK_DEPTH + by * CHUNK_DEPTH + bz] = BLOCK_AIR;
            }
        }
    }
    // Alles zur Synchronisation benötigte wird initialisiert
    resultQueue = createQueue();
    queuedChunkMovements = createDynamicArray(sizeof(vec2), 1);
    InitializeCriticalSection(&resultQueueLock);
}

void deinitChunkRenderer() {
    for(int lcx = 0; lcx < loadedChunks->rowLen; lcx++) {
        for(int lcz = 0; lcz < loadedChunks->rowLen; lcz++) {
            destroyChunk(getFromChunkRingBuffer2D(loadedChunks, lcx, lcz));
        }
    }
    destroyChunkRingBuffer2D(loadedChunks);
}

RemeshingWorkResult* createRemeshingWorkResult(DynamicArray* vertices, DynamicArray* indices, Chunk* chunk, PTP_WORK work) {
    RemeshingWorkResult* this = malloc(sizeof(RemeshingWorkResult));
    if(this == NULL) {
        throwException("Memory for RemeshingWorkResult couldn't be allocated");
    }
    this->indices = indices;
    this->vertices = vertices;
    this->chunk = chunk;
    this->work = work;
    return this;
}

void destroyRemeshingWorkResult(RemeshingWorkResult* this) {
    destroyDynamicArray(this->indices);
    destroyDynamicArray(this->vertices);
    WaitForThreadpoolWorkCallbacks(this->work, false);
    CloseThreadpoolWork(this->work);
    free(this);
}

void remeshLoadedChunks() {
    currentRemeshingWorkBatch = createDynamicArray(sizeof(PTP_WORK), pow(loadedChunks->rowLen, 2));
    for(int lcx = 0; lcx < loadedChunks->rowLen; lcx++) {
        for(int lcz = 0; lcz < loadedChunks->rowLen; lcz++) {

            // Die Argumente für den remeshChunk-Thread werden vorbereitet
            RemeshingWorkArgs* args = malloc(sizeof(RemeshingWorkArgs));
            args->chunk = getFromChunkRingBuffer2D(loadedChunks, lcx, lcz);
            args->resultQueue = resultQueue;
            args->resultQueueLock = &resultQueueLock;

            // Die Work-Instanz wird erstellt, submittet und in currentRemeshingWorkBatch registriert
            PTP_WORK remeshingWork = CreateThreadpoolWork(remeshChunk, args, NULL);
            addToDynamicArray(currentRemeshingWorkBatch, &remeshingWork);
            SubmitThreadpoolWork(remeshingWork);

        }
    }
}

// ------------------------------------------------------------------------------------------------------
//                                          FRUSTUM CULLING
// ------------------------------------------------------------------------------------------------------

typedef struct {

    vec3 normal;
    float distance;

} Plane;

typedef struct {

    Plane planes[6];
    
} Frustum;

void normalizePlanes(Frustum* this) {
    for(int i = 0; i < 6; i++) {
        float magnitude = glm_vec3_norm(this->planes[i].normal);
        glm_vec3_divs(this->planes[i].normal, magnitude, this->planes[i].normal);
        this->planes[i].distance /= magnitude;
    }
}

bool isChunkInFrustum(Frustum this, float gx, float gy, float gz, float width, float height, float depth) {

    for(int i = 0; i < 6; i++) {

        /* Für jeden Punkt wird gecheckt: Befindet sich der Punkt vor der Frustumseite (also auf der Seite der Frustumseite, die ins innere des Frustums zeigt)? 
        Falls ja: Dasselbe wird für die nächste Frustumseite gecheckt, Falls nein: Es wird für den nächsten Eckpunkt gecheckt */
        if(this.planes[i].normal[0] * gx + this.planes[i].normal[1] * gy + this.planes[i].normal[2] * gz + this.planes[i].distance > 0) {
            continue;
        }
        if(this.planes[i].normal[0] * (gx + width) + this.planes[i].normal[1] * gy + this.planes[i].normal[2] * gz + this.planes[i].distance > 0) {
            continue;
        }
        if(this.planes[i].normal[0] * gx + this.planes[i].normal[1] * (gy + height) + this.planes[i].normal[2] * gz + this.planes[i].distance > 0) {
            continue;
        }
        if(this.planes[i].normal[0] * gx + this.planes[i].normal[1] * gy + this.planes[i].normal[2] * (gz + depth) + this.planes[i].distance > 0) {
            continue;
        }
        if(this.planes[i].normal[0] * (gx + width) + this.planes[i].normal[1] * (gy + height) + this.planes[i].normal[2] * gz + this.planes[i].distance > 0) {
            continue;
        }
        if(this.planes[i].normal[0] * (gx + width) + this.planes[i].normal[1] * gy + this.planes[i].normal[2] * (gz + depth) + this.planes[i].distance > 0) {
            continue;
        }
        if(this.planes[i].normal[0] * gx + this.planes[i].normal[1] * (gy + height) + this.planes[i].normal[2] * (gz + depth) + this.planes[i].distance > 0) {
            continue;
        }
        if(this.planes[i].normal[0] * (gx + width) + this.planes[i].normal[1] * (gy + height) + this.planes[i].normal[2] * (gz + depth) + this.planes[i].distance > 0) {
            continue;
        }

        // Befindet sich bei einer Frustumseite kein Eckpunkt des Chunks davor, ist der Chunk außerhalb des Frustums
        return false;
    }

    // Gibt es für jede Frustumseite mindestens einen Eckpunkt, der sich davor befindet, ist der Chunk innerhalb des Frustums
    return true;
}

void renderChunks(Shader shader, mat4 view, mat4 proj) {

    // Sind Threads aus dem Work-Batch fertig geworden, werden ihre Ergebnisse übernommen und sie werden aus currentRemeshingWorkBatch entfernt, also soz. "abgehakt"
    while(true) {
        // Der Zugriff wird gelockt, um Race-Conditions mit den Worker-Threads zu verhindern
        EnterCriticalSection(&resultQueueLock);
        RemeshingWorkResult* result = popFromQueue(resultQueue);
        LeaveCriticalSection(&resultQueueLock);
        if(result == NULL) {
            break;
        }
        generateMesh(result->chunk->mesh, result->vertices->ptr, result->vertices->len, result->indices->ptr, result->indices->len);
        removeByValueFromDynamicArray(currentRemeshingWorkBatch, &result->work);
        destroyRemeshingWorkResult(result);
    }
    // Sind alle Threads des Work-Batches durchgelaufen wird der Batch destroyed und auf NULL gesetzt, damit dynamicallyLoadAndUnloadChunks weiß, dass ein neuer Batch angefangen werden darf
    if(currentRemeshingWorkBatch != NULL && currentRemeshingWorkBatch->len == 0) {
        destroyDynamicArray(currentRemeshingWorkBatch);
        currentRemeshingWorkBatch = NULL;
    }

    Frustum frustum;
    for(int lcx = 0; lcx < loadedChunks->rowLen; lcx++) {
        for(int lcz = 0; lcz < loadedChunks->rowLen; lcz++) {

            Chunk* chunk = getFromChunkRingBuffer2D(loadedChunks, lcx, lcz);

            // Die World-Matrix wird berechnet
            mat4 clip;
            glm_mat4_mul(proj, view, clip);
            
            // Alle Seiten des Frustums werden aus der World-Matrix extrahiert und normalisiert
            glm_vec3_copy((vec3) {clip[0][3] - clip[0][0], clip[1][3] - clip[1][0], clip[2][3] - clip[2][0]}, frustum.planes[RIGHT].normal);
            frustum.planes[RIGHT].distance = clip[3][3] - clip[3][0];

            glm_vec3_copy((vec3) {clip[0][3] + clip[0][0], clip[1][3] + clip[1][0], clip[2][3] + clip[2][0]}, frustum.planes[LEFT].normal);
            frustum.planes[LEFT].distance = clip[3][3] + clip[3][0];

            glm_vec3_copy((vec3) {clip[0][3] + clip[0][1], clip[1][3] + clip[1][1], clip[2][3] + clip[2][1]}, frustum.planes[BOTTOM].normal);
            frustum.planes[BOTTOM].distance = clip[3][3] + clip[3][1];

            glm_vec3_copy((vec3) {clip[0][3] - clip[0][1], clip[1][3] - clip[1][1], clip[2][3] - clip[2][1]}, frustum.planes[TOP].normal);
            frustum.planes[TOP].distance = clip[3][3] - clip[3][1];

            glm_vec3_copy((vec3) {clip[0][3] - clip[0][2], clip[1][3] - clip[1][2], clip[2][3] - clip[2][2]}, frustum.planes[BACK].normal);
            frustum.planes[BACK].distance = clip[3][3] - clip[3][2];

            glm_vec3_copy((vec3) {clip[0][3] + clip[0][2], clip[1][3] + clip[1][2], clip[2][3] + clip[2][2]}, frustum.planes[FRONT].normal);
            frustum.planes[FRONT].distance = clip[3][3] + clip[3][2];

            normalizePlanes(&frustum);

            // Befindet sich ein Chunk im Sichtfeld, wird er gerendert. Ansonsten nicht
            if(isChunkInFrustum(frustum, (float) chunk->x * CHUNK_WIDTH, -CHUNK_HEIGHT / 2.0f, (float) chunk->z * CHUNK_DEPTH, (float) CHUNK_WIDTH, (float) CHUNK_HEIGHT, (float) CHUNK_DEPTH)) {
                renderChunk(chunk, shader);
            }
        }
    }
}

void getChunkPos(vec3 pos, vec2 dest) { // Wandelt die g-Koordinaten von cam->pos in gc-Koordinaten um
    vec2 chunkPos = {floor(pos[0] / CHUNK_WIDTH), floor(pos[2] / CHUNK_DEPTH)};
    glm_vec2_copy(chunkPos, dest);
}

void dynamicallyLoadAndUnloadChunks(vec3 lastPlayerPos, vec3 playerPos) {

    // Die g-Spielerkoordinaten werden in gc-Koordinaten umgewandelt
    vec2 lastChunkPos, chunkPos;
    getChunkPos(lastPlayerPos, lastChunkPos);
    getChunkPos(playerPos, chunkPos);

    // Hat sich die gcx-Koordinate des Spielers ins Positive verändert?
    if(chunkPos[0] > lastChunkPos[0]) {
        vec2 lastChunkMovement = {1.0f, 0.0f};
        bool negated = false;
        // Befindet sich in queuedChunkMovements eine Bewegung, mit der lastChunkMovement verrechnet werden kann?
        for(int i = 0; i < queuedChunkMovements->len; i++) {
            float* chunkMovement = TO_VALUE(vec2) getFromDynamicArray(queuedChunkMovements, i);
            if(chunkMovement[0] == -1.0f) {
                // Falls ja: das geschieht
                removeByIndexFromDynamicArray(queuedChunkMovements, i);
                negated = true;
                break;
            }
        } 
        // Falls nein: lastChunkMovement wird zur Queue hinzugefügt
        if(!negated) {
            addToDynamicArray(queuedChunkMovements, &lastChunkMovement);
        }
    }

    if(chunkPos[0] < lastChunkPos[0]) {
        vec2 lastChunkMovement = {-1.0f, 0.0f};
        bool negated = false;
        for(int i = 0; i < queuedChunkMovements->len; i++) {
            float* chunkMovement = TO_VALUE(vec2) getFromDynamicArray(queuedChunkMovements, i);
            if(chunkMovement[0] == 1.0f) {
                removeByIndexFromDynamicArray(queuedChunkMovements, i);
                negated = true;
                break;
            }
        } 
        if(!negated) {
            addToDynamicArray(queuedChunkMovements, &lastChunkMovement);
        }
    }

    if(chunkPos[1] > lastChunkPos[1]) {
        vec2 lastChunkMovement = {0.0f, 1.0f};
        bool negated = false;
        for(int i = 0; i < queuedChunkMovements->len; i++) {
            float* chunkMovement = TO_VALUE(vec2) getFromDynamicArray(queuedChunkMovements, i);
            if(chunkMovement[1] == -1.0f) {
                removeByIndexFromDynamicArray(queuedChunkMovements, i);
                negated = true;
                break;
            }
        } 
        if(!negated) {
            addToDynamicArray(queuedChunkMovements, &lastChunkMovement);
        }
    }

    if(chunkPos[1] < lastChunkPos[1]) {
        vec2 lastChunkMovement = {0.0f, -1.0f};
        bool negated = false;
        for(int i = 0; i < queuedChunkMovements->len; i++) {
            float* chunkMovement = TO_VALUE(vec2) getFromDynamicArray(queuedChunkMovements, i);
            if(chunkMovement[1] == 1.0f) {
                removeByIndexFromDynamicArray(queuedChunkMovements, i);
                negated = true;
                break;
            }
        } 
        if(!negated) {
            addToDynamicArray(queuedChunkMovements, &lastChunkMovement);
        }
    }

    // Wurde der Batch abgehandelt, wird das nächste Element aus der movement-Queue zu einem neuen Batch verarbeitet, falls es eines gibt
    if(currentRemeshingWorkBatch == NULL && queuedChunkMovements->len != 0) {

        float* nextMovement = getFromDynamicArray(queuedChunkMovements, 0);
        removeByIndexFromDynamicArray(queuedChunkMovements, 0);

        if(nextMovement[0] == 1.0f) { // Hat sich die gcx-Koordinate des Spielers ins positive verändert?
            loadedChunks->offsetX++;    // Der Ringbuffer wird verschoben
            for(int lcz = 0; lcz < loadedChunks->rowLen; lcz++) { // Es wird über die zu verändernde Spalte iteriert
                if(!isLocalChunkValid(loadedChunks->rowLen - 1, lcz)) {  // Kann der zum Laden benötigte Chunk geladen werden?
                    updateChunk(getFromChunkRingBuffer2D(loadedChunks, 0, lcz), voidChunkData, loadedChunks->offsetX + loadedChunks->rowLen - 1, lcz + loadedChunks->offsetZ);  // Nein: Der Chunk wird zu einem VoidChunk
                } else {
                    updateChunk(getFromChunkRingBuffer2D(loadedChunks, 0, lcz), loadChunkData(loadedChunks->offsetX + loadedChunks->rowLen - 1, lcz + loadedChunks->offsetZ), loadedChunks->offsetX + loadedChunks->rowLen - 1, lcz + loadedChunks->offsetZ);   // Ja: Die Blockdaten des benötigten Chunks werden aus dem ChunkFile geladen und in den Chunk mit seinen neuen gc-Koordinaten eingesetzt
                }
            }
            incrementStartX(loadedChunks);  // Der Pointer, der zum ersten x-Element zeigt wird inkrementiert (-> Ringbuffer wird verschoben)
            remeshLoadedChunks(); // Die geladenen Chunks werden geremesht, damit die Änderungen in die Meshes übernommen werden
        }

        if(nextMovement[0] == -1.0f) {
            loadedChunks->offsetX--;
            for(int lcz = 0; lcz < loadedChunks->rowLen; lcz++) {
                if(!isLocalChunkValid(0, lcz)) {
                    updateChunk(getFromChunkRingBuffer2D(loadedChunks, loadedChunks->rowLen - 1, lcz), voidChunkData, loadedChunks->offsetX, lcz + loadedChunks->offsetZ); 
                } else{
                    updateChunk(getFromChunkRingBuffer2D(loadedChunks, loadedChunks->rowLen - 1, lcz), loadChunkData(loadedChunks->offsetX, lcz + loadedChunks->offsetZ), loadedChunks->offsetX, lcz + loadedChunks->offsetZ); 
                }
            }
            decrementStartX(loadedChunks);
            remeshLoadedChunks();
        }

        if(nextMovement[1] == 1.0f) {
            loadedChunks->offsetZ++;
            for(int lcx = 0; lcx < loadedChunks->rowLen; lcx++) {
                if(!isLocalChunkValid(lcx, loadedChunks->rowLen - 1)) {
                    updateChunk(getFromChunkRingBuffer2D(loadedChunks, lcx, 0), voidChunkData, lcx + loadedChunks->offsetX, loadedChunks->offsetZ + loadedChunks->rowLen - 1);
                } else {
                    updateChunk(getFromChunkRingBuffer2D(loadedChunks, lcx, 0), loadChunkData(lcx + loadedChunks->offsetX, loadedChunks->offsetZ + loadedChunks->rowLen - 1), lcx + loadedChunks->offsetX, loadedChunks->offsetZ + loadedChunks->rowLen - 1);
                }
            }
            incrementStartZ(loadedChunks);
            remeshLoadedChunks();
        }

        if(nextMovement[1] == -1.0f) {
            loadedChunks->offsetZ--;
            for(int lcx = 0; lcx < loadedChunks->rowLen; lcx++) {
                if(!isLocalChunkValid(lcx, 0)) {
                    updateChunk(getFromChunkRingBuffer2D(loadedChunks, lcx, loadedChunks->rowLen - 1), voidChunkData, lcx + loadedChunks->offsetX, loadedChunks->offsetZ);
                } else {
                    updateChunk(getFromChunkRingBuffer2D(loadedChunks, lcx, loadedChunks->rowLen - 1), loadChunkData(lcx + loadedChunks->offsetX, loadedChunks->offsetZ), lcx + loadedChunks->offsetX, loadedChunks->offsetZ);
                }
            }
            decrementStartZ(loadedChunks);
            remeshLoadedChunks();
        }
    }
}
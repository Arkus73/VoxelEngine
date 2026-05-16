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
#include <stdint.h>

typedef enum {
    FORWARD_MOVEMENT,
    RIGHT_MOVEMENT,
    BACKWARD_MOVEMENT,
    LEFT_MOVEMENT
} MovementDirection;

typedef enum {
    FRUSTUM_RIGHT,
    FRUSTUM_LEFT,
    FRUSTUM_BOTTOM,
    FRUSTUM_TOP,
    FRUSTUM_BACK,
    FRUSTUM_FRONT
} FrustumPlane;

ChunkRenderer* initChunkRenderer() {
    ChunkRenderer* chunkRenderer = malloc(sizeof(ChunkRenderer));
    if(chunkRenderer == NULL) {
        throwException("Memory for ChunkRenderer couldn't be allocated");
    }

    uint8_t* initData = malloc(CHUNK_WIDTH * CHUNK_HEIGHT * CHUNK_DEPTH * sizeof(uint8_t));
    if(initData == NULL) {
        throwException("Memory for initData couldn't be allocated");
    }
    for(int bx = 0; bx < CHUNK_WIDTH; bx++) {
        for(int by = 0; by < CHUNK_HEIGHT; by++) {
            for(int bz = 0; bz < CHUNK_DEPTH; bz++) {
                initData[bx * CHUNK_HEIGHT * CHUNK_DEPTH + by * CHUNK_DEPTH + bz] = BLOCK_AIR;
            }
        }
    }

    chunkRenderer->loadedChunks = createChunkRingBuffer2D();   // Die geladenen Chunks werden initialisiert
    for(int lcx = 0; lcx < chunkRenderer->loadedChunks->rowLen; lcx++) {
        for(int lcz = 0; lcz < chunkRenderer->loadedChunks->rowLen; lcz++) {
            writeToChunkRingBuffer2D(chunkRenderer->loadedChunks, lcx, lcz, createChunk(initData, lcx + chunkRenderer->loadedChunks->offsetX, lcz + chunkRenderer->loadedChunks->offsetZ));
        }
    }

    free(initData);

    // Alles zur Synchronisation benötigte wird initialisiert
    chunkRenderer->remeshingWorkResultQueue = createQueue();
    chunkRenderer->chunkGenerationWorkResultQueue = createQueue();
    chunkRenderer->remeshingWorkBatch = NULL;
    chunkRenderer->chunkGenerationWorkBatch = NULL;
    chunkRenderer->queuedChunkMovements = createDynamicArray(sizeof(MovementDirection), 1);
    InitializeCriticalSection(&chunkRenderer->remeshingWorkResultQueueLock);
    InitializeCriticalSection(&chunkRenderer->chunkGenerationWorkResultQueueLock);

    return chunkRenderer;
}

RemeshingWorkArgs* createRemeshingWorkArgs(ChunkRenderer* chunkRenderer, int lcx, int lcz) {
    RemeshingWorkArgs* this = malloc(sizeof(RemeshingWorkArgs));
    if(this == NULL) {
        throwException("Memory for RemeshingWorkArgs couldn't be allocated");
    }
    this->chunk = getFromChunkRingBuffer2D(chunkRenderer->loadedChunks, lcx, lcz);
    this->resultQueue = chunkRenderer->remeshingWorkResultQueue;
    this->resultQueueLock = &chunkRenderer->remeshingWorkResultQueueLock;
    this->loadedChunks = chunkRenderer->loadedChunks;
    return this;
}

void createRemeshingWork(ChunkRenderer* chunkRenderer, int lcx, int lcz) {
    // Die Argumente für den remeshChunk-Thread werden vorbereitet
    RemeshingWorkArgs* args = createRemeshingWorkArgs(chunkRenderer, lcx, lcz);

    // Die Work-Instanz wird erstellt und in chunkRenderer->remeshingWorkBatch registriert
    PTP_WORK work = CreateThreadpoolWork(remeshChunk, args, NULL);
    addToDynamicArray(chunkRenderer->remeshingWorkBatch, &work);
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

ChunkGenerationWorkArgs* createChunkGenerationWorkArgs(ChunkRenderer* chunkRenderer, WorldGenerator* worldGenerator, Chunk* chunk) {
    ChunkGenerationWorkArgs* this = malloc(sizeof(ChunkGenerationWorkArgs));
    if(this == NULL) {
        throwException("Memory for ChunkGenerationWorkArgs couldn't be allocated");
    }
    this->chunk = chunk;
    this->worldGenerator = worldGenerator;
    this->resultQueue = chunkRenderer->chunkGenerationWorkResultQueue;
    this->resultQueueLock = &chunkRenderer->chunkGenerationWorkResultQueueLock;
    return this;
}

ChunkGenerationWorkResult* createChunkGenerationWorkResult(PTP_WORK work) {
    ChunkGenerationWorkResult* this = malloc(sizeof(ChunkGenerationWorkResult));
    if(this == NULL) {
        throwException("Memory for ChunkGenerationWorkResult coulnd't be allocated");
    }
    this->work = work;
    return this;
}

void destroyChunkGenerationWorkResult(ChunkGenerationWorkResult* this) {
    WaitForThreadpoolWorkCallbacks(this->work, false);
    CloseThreadpoolWork(this->work);
    free(this);
}

void deinitChunkRenderer(ChunkRenderer* chunkRenderer) {

    if(chunkRenderer->remeshingWorkBatch != NULL) {
        for(int i = 0; i < chunkRenderer->remeshingWorkBatch->len; i++) {
            PTP_WORK work = TO_VALUE(PTP_WORK) getFromDynamicArray(chunkRenderer->remeshingWorkBatch, i);
            WaitForThreadpoolWorkCallbacks(work, false);
            CloseThreadpoolWork(work);
        }
        destroyDynamicArray(chunkRenderer->remeshingWorkBatch);
    }
    if(chunkRenderer->chunkGenerationWorkBatch != NULL) {
        for(int i = 0; i < chunkRenderer->chunkGenerationWorkBatch->len; i++) {
            PTP_WORK work = TO_VALUE(PTP_WORK) getFromDynamicArray(chunkRenderer->chunkGenerationWorkBatch, i);
            WaitForThreadpoolWorkCallbacks(work, false);
            CloseThreadpoolWork(work);
        }
        destroyDynamicArray(chunkRenderer->chunkGenerationWorkBatch);
    }

    for(int lcx = 0; lcx < chunkRenderer->loadedChunks->rowLen; lcx++) {
        for(int lcz = 0; lcz < chunkRenderer->loadedChunks->rowLen; lcz++) {
            destroyChunk(getFromChunkRingBuffer2D(chunkRenderer->loadedChunks, lcx, lcz));
        }
    }
    destroyChunkRingBuffer2D(chunkRenderer->loadedChunks);

    while(chunkRenderer->remeshingWorkResultQueue->front != NULL) {
        destroyRemeshingWorkResult(popFromQueue(chunkRenderer->remeshingWorkResultQueue));
    }
    destroyQueue(chunkRenderer->remeshingWorkResultQueue, false);

    while(chunkRenderer->chunkGenerationWorkResultQueue->front != NULL) {
        destroyChunkGenerationWorkResult(popFromQueue(chunkRenderer->chunkGenerationWorkResultQueue));
    }
    destroyQueue(chunkRenderer->chunkGenerationWorkResultQueue, false);

    free(chunkRenderer);
}

void remeshLoadedChunks(ChunkRenderer* chunkRenderer) {
    chunkRenderer->remeshingWorkBatch = createDynamicArray(sizeof(PTP_WORK), chunkRenderer->loadedChunks->rowLen * chunkRenderer->loadedChunks->rowLen);
    for(int lcx = 0; lcx < chunkRenderer->loadedChunks->rowLen; lcx++) {
        for(int lcz = 0; lcz < chunkRenderer->loadedChunks->rowLen; lcz++) {

            createRemeshingWork(chunkRenderer, lcx, lcz);

        }
    }
}

void generateLoadedChunks(ChunkRenderer* chunkRenderer, WorldGenerator* worldGenerator) {
    chunkRenderer->chunkGenerationWorkBatch = createDynamicArray(sizeof(PTP_WORK), chunkRenderer->loadedChunks->rowLen * chunkRenderer->loadedChunks->rowLen);
    for(int lcx = 0; lcx < chunkRenderer->loadedChunks->rowLen; lcx++) {
        for(int lcz = 0; lcz < chunkRenderer->loadedChunks->rowLen; lcz++) {

            ChunkGenerationWorkArgs* args = createChunkGenerationWorkArgs(chunkRenderer, worldGenerator, getFromChunkRingBuffer2D(chunkRenderer->loadedChunks, lcx, lcz));

            PTP_WORK chunkGenerationWork = CreateThreadpoolWork(generateChunk, args, NULL);
            addToDynamicArray(chunkRenderer->chunkGenerationWorkBatch, &chunkGenerationWork);
            SubmitThreadpoolWork(chunkGenerationWork);
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

void renderChunks(ChunkRenderer* chunkRenderer, Shader shader, mat4 view, mat4 proj) {

    // Sind Threads aus dem chunkRemeshingWorkBatch fertig, werden sie "abgehakt"
    while(true) {
        EnterCriticalSection(&chunkRenderer->chunkGenerationWorkResultQueueLock);
        ChunkGenerationWorkResult* result = popFromQueue(chunkRenderer->chunkGenerationWorkResultQueue);
        LeaveCriticalSection(&chunkRenderer->chunkGenerationWorkResultQueueLock);
        if(result == NULL) {
            break;
        }
        removeByValueFromDynamicArray(chunkRenderer->chunkGenerationWorkBatch, &result->work);
        destroyChunkGenerationWorkResult(result);
    }
    // Sind alle Threads des chunkGenerationWorkBatch durchgelaufen wird der Batch destroyed, auf NULL gesetzt und der remeshingWorkBatch begonnen
    if(chunkRenderer->chunkGenerationWorkBatch != NULL && chunkRenderer->chunkGenerationWorkBatch->len == 0) {
        destroyDynamicArray(chunkRenderer->chunkGenerationWorkBatch);
        chunkRenderer->chunkGenerationWorkBatch = NULL;

        for(int i = 0; i < chunkRenderer->remeshingWorkBatch->len; i++) {
            SubmitThreadpoolWork(TO_VALUE(PTP_WORK) getFromDynamicArray(chunkRenderer->remeshingWorkBatch, i));
        }
    }

    // Sind Threads aus dem remeshingWorkBatch fertig geworden, werden ihre Ergebnisse übernommen und sie werden aus chunkRenderer->remeshingWorkBatch entfernt, also soz. "abgehakt"
    while(true) {
        // Der Zugriff wird gelockt, um Race-Conditions mit den Worker-Threads zu verhindern
        EnterCriticalSection(&chunkRenderer->remeshingWorkResultQueueLock);
        RemeshingWorkResult* result = popFromQueue(chunkRenderer->remeshingWorkResultQueue);
        LeaveCriticalSection(&chunkRenderer->remeshingWorkResultQueueLock);
        if(result == NULL) {
            break;
        }
        generateMesh(result->chunk->mesh, result->vertices->ptr, result->vertices->len, result->indices->ptr, result->indices->len);
        removeByValueFromDynamicArray(chunkRenderer->remeshingWorkBatch, &result->work);
        destroyRemeshingWorkResult(result);
    }
    // Sind alle Threads des remeshingWorkBatches durchgelaufen wird der Batch destroyed und auf NULL gesetzt, damit dynamicallyLoadAndUnloadChunks weiß, dass ein neuer Batch angefangen werden darf
    if(chunkRenderer->remeshingWorkBatch != NULL && chunkRenderer->remeshingWorkBatch->len == 0) {
        destroyDynamicArray(chunkRenderer->remeshingWorkBatch);
        chunkRenderer->remeshingWorkBatch = NULL;
    }

    Frustum frustum;

    // Die World-Matrix wird berechnet
    mat4 clip;
    glm_mat4_mul(proj, view, clip);
            
    // Alle Seiten des Frustums werden aus der World-Matrix extrahiert und normalisiert
    glm_vec3_copy((vec3) {clip[0][3] - clip[0][0], clip[1][3] - clip[1][0], clip[2][3] - clip[2][0]}, frustum.planes[FRUSTUM_RIGHT].normal);
    frustum.planes[FRUSTUM_RIGHT].distance = clip[3][3] - clip[3][0];

    glm_vec3_copy((vec3) {clip[0][3] + clip[0][0], clip[1][3] + clip[1][0], clip[2][3] + clip[2][0]}, frustum.planes[FRUSTUM_LEFT].normal);
    frustum.planes[FRUSTUM_LEFT].distance = clip[3][3] + clip[3][0];

    glm_vec3_copy((vec3) {clip[0][3] + clip[0][1], clip[1][3] + clip[1][1], clip[2][3] + clip[2][1]}, frustum.planes[FRUSTUM_BOTTOM].normal);
    frustum.planes[FRUSTUM_BOTTOM].distance = clip[3][3] + clip[3][1];

    glm_vec3_copy((vec3) {clip[0][3] - clip[0][1], clip[1][3] - clip[1][1], clip[2][3] - clip[2][1]}, frustum.planes[FRUSTUM_TOP].normal);
    frustum.planes[FRUSTUM_TOP].distance = clip[3][3] - clip[3][1];

    glm_vec3_copy((vec3) {clip[0][3] - clip[0][2], clip[1][3] - clip[1][2], clip[2][3] - clip[2][2]}, frustum.planes[FRUSTUM_BACK].normal);
    frustum.planes[FRUSTUM_BACK].distance = clip[3][3] - clip[3][2];

    glm_vec3_copy((vec3) {clip[0][3] + clip[0][2], clip[1][3] + clip[1][2], clip[2][3] + clip[2][2]}, frustum.planes[FRUSTUM_FRONT].normal);
    frustum.planes[FRUSTUM_FRONT].distance = clip[3][3] + clip[3][2];

    normalizePlanes(&frustum);

    for(int lcx = 0; lcx < chunkRenderer->loadedChunks->rowLen; lcx++) {
        for(int lcz = 0; lcz < chunkRenderer->loadedChunks->rowLen; lcz++) {

            Chunk* chunk = getFromChunkRingBuffer2D(chunkRenderer->loadedChunks, lcx, lcz);

            // Befindet sich ein Chunk im Sichtfeld, wird er gerendert. Ansonsten nicht
            if(isChunkInFrustum(frustum, (float) chunk->gcx * CHUNK_WIDTH, -CHUNK_HEIGHT / 2.0f, (float) chunk->gcz * CHUNK_DEPTH, (float) CHUNK_WIDTH, (float) CHUNK_HEIGHT, (float) CHUNK_DEPTH)) {
                renderChunk(chunk, shader);
            }
        }
    }
}

void getChunkPos(vec3 pos, vec2 dest) { // Wandelt die g-Koordinaten von cam->pos in gc-Koordinaten um
    vec2 chunkPos = {floor(pos[0] / CHUNK_WIDTH), floor(pos[2] / CHUNK_DEPTH)};
    glm_vec2_copy(chunkPos, dest);
}

void queueChunkMovement(ChunkRenderer* chunkRenderer, MovementDirection movementDir) {
    bool negated = false;
    MovementDirection negatingDir;  // Die Richtung, mit der movementDir verrechnet werden könnte wird ermittelt
    switch(movementDir) {
        case FORWARD_MOVEMENT:
        negatingDir = BACKWARD_MOVEMENT;
        break;
        case RIGHT_MOVEMENT:
        negatingDir = LEFT_MOVEMENT;
        break;
        case BACKWARD_MOVEMENT:
        negatingDir = FORWARD_MOVEMENT;
        break;
        case LEFT_MOVEMENT:
        negatingDir = RIGHT_MOVEMENT;
        break;
    }

    // Befindet sich in chunkRenderer->queuedChunkMovements eine Bewegung, mit der movementDIr verrechnet werden kann?
    for(int i = 0; i < chunkRenderer->queuedChunkMovements->len; i++) {
        MovementDirection queuedMovementDir = TO_VALUE(MovementDirection) getFromDynamicArray(chunkRenderer->queuedChunkMovements, i);
        if(queuedMovementDir == negatingDir) {
            // Falls ja: das geschieht
            removeByIndexFromDynamicArray(chunkRenderer->queuedChunkMovements, i);
            negated = true;
            break;
        }
    } 
    // Falls nein: lastChunkMovement wird zur Queue hinzugefügt
    if(!negated) {
        addToDynamicArray(chunkRenderer->queuedChunkMovements, &movementDir);
    }
}

void dynamicallyLoadAndUnloadChunks(ChunkRenderer* chunkRenderer, WorldGenerator* worldGenerator, vec3 lastPlayerPos, vec3 playerPos) {

    // Die g-Spielerkoordinaten werden in gc-Koordinaten umgewandelt
    vec2 lastChunkPos, chunkPos;
    getChunkPos(lastPlayerPos, lastChunkPos);
    getChunkPos(playerPos, chunkPos);

    if(chunkPos[0] > lastChunkPos[0]) {
        queueChunkMovement(chunkRenderer, RIGHT_MOVEMENT);
    }

    if(chunkPos[0] < lastChunkPos[0]) {
        queueChunkMovement(chunkRenderer, LEFT_MOVEMENT);
    }

    if(chunkPos[1] > lastChunkPos[1]) {
        queueChunkMovement(chunkRenderer, BACKWARD_MOVEMENT);
    }

    if(chunkPos[1] < lastChunkPos[1]) {
        queueChunkMovement(chunkRenderer, FORWARD_MOVEMENT);
    }

    // Wurde der Batch abgehandelt, wird das nächste Element aus der movement-Queue zu einem neuen Batch verarbeitet, falls es eines gibt
    if(chunkRenderer->remeshingWorkBatch == NULL && chunkRenderer->queuedChunkMovements->len != 0) {

        MovementDirection nextMovement = TO_VALUE(MovementDirection) getFromDynamicArray(chunkRenderer->queuedChunkMovements, 0);
        removeByIndexFromDynamicArray(chunkRenderer->queuedChunkMovements, 0);

        if(nextMovement == RIGHT_MOVEMENT) {
            chunkRenderer->loadedChunks->offsetX++;    // Der Ringbuffer wird verschoben
            incrementStartX(chunkRenderer->loadedChunks);  // Der Pointer, der zum ersten x-Element zeigt wird inkrementiert (-> Die lcx-Positionen der Chunks werden verschoben)
            chunkRenderer->remeshingWorkBatch = createDynamicArray(sizeof(PTP_WORK), 2 * (2 * RENDER_DISTANCE + 1));    // Der neue RemeshingWorkBatch wird initialisiert
            for(int lcz = 0; lcz < chunkRenderer->loadedChunks->rowLen; lcz++) { // Es wird über die zu verändernde Spalte iteriert
                int lcx = chunkRenderer->loadedChunks->rowLen - 1;
                updateChunkPosition(getFromChunkRingBuffer2D(chunkRenderer->loadedChunks, lcx, lcz), chunkRenderer->loadedChunks->offsetX + lcx, chunkRenderer->loadedChunks->offsetZ + lcz); // Die gcx-Position des Chunks wird verschoben
                for(int lcx = chunkRenderer->loadedChunks->rowLen - 2; lcx < chunkRenderer->loadedChunks->rowLen; lcx++) { // Die nötigen Chunks werden geremesht, damit die Änderungen in die Meshes übernommen werden
                    createRemeshingWork(chunkRenderer, lcx, lcz);
                }
            }
            generateLoadedChunks(chunkRenderer, worldGenerator);
        }

        if(nextMovement == LEFT_MOVEMENT) {
            chunkRenderer->loadedChunks->offsetX--;
            decrementStartX(chunkRenderer->loadedChunks);
            chunkRenderer->remeshingWorkBatch = createDynamicArray(sizeof(PTP_WORK), 2 * (2 * RENDER_DISTANCE + 1));
            for(int lcz = 0; lcz < chunkRenderer->loadedChunks->rowLen; lcz++) {
                int lcx = 0;
                updateChunkPosition(getFromChunkRingBuffer2D(chunkRenderer->loadedChunks, lcx, lcz), chunkRenderer->loadedChunks->offsetX + lcx, chunkRenderer->loadedChunks->offsetZ + lcz);
                for(int lcx = 0; lcx < 2; lcx++) {
                    createRemeshingWork(chunkRenderer, lcx, lcz);
                }
            }
            generateLoadedChunks(chunkRenderer, worldGenerator);
        }

        if(nextMovement == BACKWARD_MOVEMENT) {
            chunkRenderer->loadedChunks->offsetZ++;
            incrementStartZ(chunkRenderer->loadedChunks);
            chunkRenderer->remeshingWorkBatch = createDynamicArray(sizeof(PTP_WORK), 2 * (2 * RENDER_DISTANCE + 1));
            for(int lcx = 0; lcx < chunkRenderer->loadedChunks->rowLen; lcx++) {
                int lcz = chunkRenderer->loadedChunks->rowLen - 1;
                updateChunkPosition(getFromChunkRingBuffer2D(chunkRenderer->loadedChunks, lcx, lcz), chunkRenderer->loadedChunks->offsetX + lcx, chunkRenderer->loadedChunks->offsetZ + lcz);
                for(int lcz = chunkRenderer->loadedChunks->rowLen - 2; lcz < chunkRenderer->loadedChunks->rowLen; lcz++) {
                    createRemeshingWork(chunkRenderer, lcx, lcz);
                }
            }
            generateLoadedChunks(chunkRenderer, worldGenerator);
        }

        if(nextMovement == FORWARD_MOVEMENT) {
            chunkRenderer->loadedChunks->offsetZ--;
            decrementStartZ(chunkRenderer->loadedChunks);
            chunkRenderer->remeshingWorkBatch = createDynamicArray(sizeof(PTP_WORK), 2 * (2 * RENDER_DISTANCE + 1));
            for(int lcx = 0; lcx < chunkRenderer->loadedChunks->rowLen; lcx++) {
                int lcz = 0;
                updateChunkPosition(getFromChunkRingBuffer2D(chunkRenderer->loadedChunks, lcx, lcz), chunkRenderer->loadedChunks->offsetX + lcx, chunkRenderer->loadedChunks->offsetZ + lcz);
                for(int lcz = 0; lcz < 2; lcz++) {
                    createRemeshingWork(chunkRenderer, lcx, lcz);
                }
            }
            generateLoadedChunks(chunkRenderer, worldGenerator);
        }
    }
}
#include "mesh.h"
#include "chunk.h"
#include "block.h"
#include "utils.h"
#include "shader.h"
#include "dynamicArray.h"
#include "chunkRenderer.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

#define BLOCK_AT(bx, by, bz) blocks[(bx) * CHUNK_HEIGHT * CHUNK_DEPTH + (by) * CHUNK_DEPTH + (bz)]    // bx, by und bz in Klammern, damit Rechenreihenfolge auch bspw. bei x = x + 1 eingehalten wird

Chunk* createChunk(uint8_t* blocks, int gcx, int gcz) {

    Chunk* this = malloc(sizeof(Chunk));

    if(this == NULL) {
        throwException("Memory couldn't be allocated");
    }
    
    this->x = gcx;
    this->z = gcz;

    this->blocks = malloc(CHUNK_WIDTH * CHUNK_HEIGHT * CHUNK_DEPTH * sizeof(uint8_t));
    if(this->blocks == NULL) {
        throwException("Memory couldn't be allocated for blocks");
    }

    memcpy(this->blocks, blocks, CHUNK_WIDTH * CHUNK_HEIGHT * CHUNK_DEPTH * sizeof(uint8_t));

    this->mesh = initMesh();

    return this;
}

void destroyChunk(Chunk* this) {
    destroyMesh(this->mesh);
    free(this->blocks);
    free(this);
}

void updateChunk(Chunk* this, uint8_t* blocks, int gcx, int gcz) {
    memcpy(this->blocks, blocks, CHUNK_WIDTH * CHUNK_HEIGHT * CHUNK_DEPTH * sizeof(uint8_t));
    this->x = gcx;
    this->z = gcz;
}

uint8_t getBlock(int gx, int gy, int gz);
void addFaceToChunkMesh(Block block, Face face, DynamicArray* vertices, DynamicArray* indices, int bx, int by, int bz);
void addIndicesToChunkMesh(DynamicArray* indices, unsigned int startIndex);
void addVertexToChunkMesh(Face face, DynamicArray* vertices, float bx, float by, float bz, float texCoordX, float texCoordY);

void remeshChunk(Chunk* this) {

    INIT_MODULE_DYNAMIC_ARRAY;

    DynamicArray* vertices = createDynamicArray(sizeof(float), 8e3, true);
    DynamicArray* indices = createDynamicArray(sizeof(unsigned int), 2e3, true);

    // Es wird durch den blocks-Array iteriert und sämtliche Faces der Blöcke, die gesehen werden können werden dem neuen Chunk-Mesh hinzugefügt
    for(int bx = 0; bx < CHUNK_WIDTH; bx++) {
        for(int by = 0; by < CHUNK_HEIGHT; by++) {
            for(int bz = 0; bz < CHUNK_DEPTH; bz++) {

                if(this->BLOCK_AT(bx, by, bz) == BLOCK_AIR) {
                    continue;
                }

                Block currentBlock = TO_VALUE(Block) getFromDynamicArray(blockRegistry, this->BLOCK_AT(bx, by, bz));

                // Die Blockkoordinaten werden in globale Koordinaten umgewandelt
                int gx = this->x * CHUNK_WIDTH + bx;
                int gy = by;
                int gz = this->z * CHUNK_DEPTH + bz;
                
                // Falls Face sichtbar ist (an Luft grenzt), wird es dem Chunk-Mesh hinzugefügt
                if(getBlock(gx, gy, gz + 1) == BLOCK_AIR) {
                    addFaceToChunkMesh(currentBlock, FRONT, vertices, indices, bx, by, bz);
                }
                if(getBlock(gx, gy, gz - 1) == BLOCK_AIR) {
                    addFaceToChunkMesh(currentBlock, BACK, vertices, indices, bx, by, bz);
                }
                if(getBlock(gx, gy + 1, gz) == BLOCK_AIR) {
                    addFaceToChunkMesh(currentBlock, TOP, vertices, indices, bx, by, bz);
                }
                if(getBlock(gx, gy - 1, gz) == BLOCK_AIR) {
                    addFaceToChunkMesh(currentBlock, BOTTOM, vertices, indices, bx, by, bz);
                }
                if(getBlock(gx + 1, gy, gz) == BLOCK_AIR) {
                    addFaceToChunkMesh(currentBlock, RIGHT, vertices, indices, bx, by, bz);
                }
                if(getBlock(gx - 1, gy, gz) == BLOCK_AIR) {
                    addFaceToChunkMesh(currentBlock, LEFT, vertices, indices, bx, by, bz);
                }

            }
        }
    }

    // Zu guter letzt werden die neu generierten Indices und Vertices in das VBO/EBO des Chunk-Meshes gesteckt

    generateMesh(this->mesh, (float*) vertices->ptr, vertices->len, (unsigned int*) indices->ptr, indices->len);

    DEINIT_MODULE_DYNAMIC_ARRAY;
}

int floorDiv(int a, int b) {
    int r = a / b;
    if ((a ^ b) < 0 && a % b) r--;
    return r;
}

uint8_t getBlock(int gx, int gy, int gz) {

    // Die globalen Koordinaten werden erst in gc, dann in lc-Koordinaten umgewandelt
    int lcx = floorDiv(gx, CHUNK_WIDTH) - loadedChunks->offsetX;
    int lcz = floorDiv(gz, CHUNK_DEPTH) - loadedChunks->offsetZ;

    // Aus dem globalen Koordinaten werden nochmal die Blockkoordinaten rekonstruiert
    int bx = (gx % CHUNK_WIDTH + CHUNK_WIDTH) % CHUNK_WIDTH;
    int bz = (gz % CHUNK_DEPTH + CHUNK_DEPTH) % CHUNK_DEPTH;

    // Der Chunk, in dem sich der Block befindet wird geholt
    Chunk* chunk = getFromChunkRingBuffer2D(loadedChunks, lcx, lcz);

    // Sind die Koordinaten out of Bounds (-> Block daneben ist am Map-Rand) wird BLOCK_AIR ausgegeben und das Face, um das es geht, wird dem Mesh hinzugefügt
    if(gy < 0 || gy >= CHUNK_HEIGHT || chunk == NULL) {
        return BLOCK_AIR;
    }

    // Ansonsten wird der gesuchte Block ausgegebenW
    return chunk->BLOCK_AT(bx, gy, bz);
}

void addFaceToChunkMesh(Block block, Face face, DynamicArray* vertices, DynamicArray* indices, int bx, int by, int bz) {

    unsigned int startIndex = vertices->len / 8;

    float texCoordX, texCoordY;

    switch(face) {

        case FRONT:
            // Sämtliche zum Face "FRONT" gehörige Vertices werden dem Mesh hinzugefügt
            texCoordX = block.frontTexture[0];
            texCoordY = block.frontTexture[1];
            addVertexToChunkMesh(face, vertices, bx, by, bz + 1, texCoordX, texCoordY);   
            addVertexToChunkMesh(face, vertices, bx + 1, by, bz + 1, texCoordX + RELATIVE_SUBTEXTURE_WIDTH, texCoordY);
            addVertexToChunkMesh(face, vertices, bx + 1, by + 1, bz + 1, texCoordX + RELATIVE_SUBTEXTURE_WIDTH, texCoordY + RELATIVE_SUBTEXTURE_HEIGHT);
            addVertexToChunkMesh(face, vertices, bx, by + 1, bz + 1, texCoordX, texCoordY + RELATIVE_SUBTEXTURE_HEIGHT);
            break;

        case BACK:
            // Sämtliche zum Face "BACK" gehörige Vertices werden dem Mesh hinzugefügt
            texCoordX = block.backTexture[0];
            texCoordY = block.backTexture[1];
            addVertexToChunkMesh(face, vertices, bx, by + 1, bz, texCoordX, texCoordY + RELATIVE_SUBTEXTURE_HEIGHT);
            addVertexToChunkMesh(face, vertices, bx + 1, by + 1, bz, texCoordX + RELATIVE_SUBTEXTURE_WIDTH, texCoordY + RELATIVE_SUBTEXTURE_HEIGHT);
            addVertexToChunkMesh(face, vertices, bx + 1, by, bz, texCoordX + RELATIVE_SUBTEXTURE_WIDTH, texCoordY);
            addVertexToChunkMesh(face, vertices, bx, by, bz, texCoordX, texCoordY);
            break;

        case TOP:
            // Sämtliche zum Face "TOP" gehörige Vertices werden dem Mesh hinzugefügt
            texCoordX = block.topTexture[0];
            texCoordY = block.topTexture[1];
            addVertexToChunkMesh(face, vertices, bx, by + 1, bz + 1, texCoordX, texCoordY);
            addVertexToChunkMesh(face, vertices, bx + 1, by + 1, bz + 1, texCoordX + RELATIVE_SUBTEXTURE_WIDTH, texCoordY);
            addVertexToChunkMesh(face, vertices, bx + 1, by + 1, bz, texCoordX + RELATIVE_SUBTEXTURE_WIDTH, texCoordY + RELATIVE_SUBTEXTURE_HEIGHT);
            addVertexToChunkMesh(face, vertices, bx, by + 1, bz, texCoordX, texCoordY + RELATIVE_SUBTEXTURE_HEIGHT);
            break;

        case BOTTOM:
            // Sämtliche zum Face "BOTTOM" gehörige Vertices werden dem Mesh hinzugefügt
            texCoordX = block.bottomTexture[0];
            texCoordY = block.bottomTexture[1];
            addVertexToChunkMesh(face, vertices, bx, by, bz, texCoordX, texCoordY + RELATIVE_SUBTEXTURE_HEIGHT);
            addVertexToChunkMesh(face, vertices, bx + 1, by, bz, texCoordX + RELATIVE_SUBTEXTURE_WIDTH, texCoordY + RELATIVE_SUBTEXTURE_HEIGHT);
            addVertexToChunkMesh(face, vertices, bx + 1, by, bz + 1, texCoordX + RELATIVE_SUBTEXTURE_WIDTH, texCoordY);
            addVertexToChunkMesh(face, vertices, bx, by, bz + 1, texCoordX, texCoordY);
            break;

        case RIGHT:
            // Sämtliche zum Face "RIGHT" gehörige Vertices werden dem Mesh hinzugefügt
            texCoordX = block.rightTexture[0];
            texCoordY = block.rightTexture[1];
            addVertexToChunkMesh(face, vertices, bx + 1, by, bz + 1, texCoordX, texCoordY);
            addVertexToChunkMesh(face, vertices, bx + 1, by, bz, texCoordX + RELATIVE_SUBTEXTURE_WIDTH, texCoordY);
            addVertexToChunkMesh(face, vertices, bx + 1, by + 1, bz, texCoordX + RELATIVE_SUBTEXTURE_WIDTH, texCoordY + RELATIVE_SUBTEXTURE_HEIGHT);
            addVertexToChunkMesh(face, vertices, bx + 1, by + 1, bz + 1, texCoordX, texCoordY + RELATIVE_SUBTEXTURE_HEIGHT);
            break;

        case LEFT:
            // Sämtliche zum Face "LEFT" gehörige Vertices werden dem Mesh hinzugefügt
            texCoordX = block.leftTexture[0];
            texCoordY = block.leftTexture[1];
            addVertexToChunkMesh(face, vertices, bx, by + 1, bz + 1, texCoordX, texCoordY + RELATIVE_SUBTEXTURE_HEIGHT);
            addVertexToChunkMesh(face, vertices, bx, by + 1, bz, texCoordX + RELATIVE_SUBTEXTURE_WIDTH, texCoordY + RELATIVE_SUBTEXTURE_HEIGHT);
            addVertexToChunkMesh(face, vertices, bx, by, bz, texCoordX + RELATIVE_SUBTEXTURE_WIDTH, texCoordY);
            addVertexToChunkMesh(face, vertices, bx, by, bz + 1, texCoordX, texCoordY);
            break;
    }

    addIndicesToChunkMesh(indices, startIndex);
}

void addIndicesToChunkMesh(DynamicArray* indices, unsigned int startIndex) {
    unsigned int value = startIndex;
    addToDynamicArray(indices, &value);
    value++;
    addToDynamicArray(indices, &value);
    value++;
    addToDynamicArray(indices, &value);

    value = startIndex;
    addToDynamicArray(indices, &value);
    value += 2;
    addToDynamicArray(indices, &value);
    value++;
    addToDynamicArray(indices, &value);
}

void addVertexToChunkMesh(Face face, DynamicArray* vertices, float bx, float by, float bz, float texCoordX, float texCoordY) {

    // aPos-Attribut des Vertex wird gesetzt
    addToDynamicArray(vertices, &bx);
    addToDynamicArray(vertices, &by);
    addToDynamicArray(vertices, &bz);

    float normalComponent;

    switch(face) {

        case FRONT:
            // aNormal-Attribut des Vertex wird gesetzt
            normalComponent = 0.0f;   // x-Komponente der Normale
            addToDynamicArray(vertices, &normalComponent);
            normalComponent = 0.0f;   // y-Komponente der Normale
            addToDynamicArray(vertices, &normalComponent);
            normalComponent = 1.0f;   // z-Komponente der Normale
            addToDynamicArray(vertices, &normalComponent);
            break;

        case BACK:
            // aNormal-Attribut des Vertex wird gesetzt
            normalComponent = 0.0f;
            addToDynamicArray(vertices, &normalComponent);
            normalComponent = 0.0f;
            addToDynamicArray(vertices, &normalComponent);
            normalComponent = -1.0f;
            addToDynamicArray(vertices, &normalComponent);
            break;

        case TOP:
            // aNormal-Attribut des Vertex wird gesetzt
            normalComponent = 0.0f;
            addToDynamicArray(vertices, &normalComponent);
            normalComponent = 1.0f;
            addToDynamicArray(vertices, &normalComponent);
            normalComponent = 0.0f;
            addToDynamicArray(vertices, &normalComponent);
            break;

        case BOTTOM:
            // aNormal-Attribut des Vertex wird gesetzt
            normalComponent = 0.0f;
            addToDynamicArray(vertices, &normalComponent);
            normalComponent = -1.0f;
            addToDynamicArray(vertices, &normalComponent);
            normalComponent = 0.0f;
            addToDynamicArray(vertices, &normalComponent);
            break;

        case RIGHT:
            // aNormal-Attribut des Vertex wird gesetzt
            normalComponent = 1.0f;
            addToDynamicArray(vertices, &normalComponent);
            normalComponent = 0.0f;
            addToDynamicArray(vertices, &normalComponent);
            normalComponent = 0.0f;
            addToDynamicArray(vertices, &normalComponent);
            break;

        case LEFT:
            // aNormal-Attribut des Vertex wird gesetzt
            normalComponent = -1.0f;
            addToDynamicArray(vertices, &normalComponent);
            normalComponent = 0.0f;
            addToDynamicArray(vertices, &normalComponent);
            normalComponent = 0.0f;
            addToDynamicArray(vertices, &normalComponent);
            break;
    }

    // aTexCoord-Attribut des Vertex wird gesetzt
    addToDynamicArray(vertices, &texCoordX);
    addToDynamicArray(vertices, &texCoordY);
}

void renderChunk(Chunk* this, Shader shader) {
    mat4 model;
    glm_mat4_identity(model);
    glm_translate(model, (vec3) {this->x * CHUNK_WIDTH, -CHUNK_HEIGHT / 2, this->z * CHUNK_DEPTH});
    setMatrix(shader, "model", model);
    renderMesh(this->mesh);
}
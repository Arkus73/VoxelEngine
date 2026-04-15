#include "mesh.h"
#include "chunk.h"
#include "block.h"
#include "utils.h"
#include "shader.h"
#include "dynamicArray.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

#define BLOCK_AT(x, y, z) (this->blocks[(x) * CHUNK_HEIGHT * CHUNK_DEPTH + (y) * CHUNK_DEPTH + (z)])    // x, y und z in Klammern, damit Rechenreihenfolge auch bspw. bei x = x + 1 eingehalten wird

Chunk* createChunk(uint8_t* blocks, int x, int z) {

    Chunk* this = malloc(sizeof(Chunk));

    if(this == NULL) {
        throwException("Memory couldn't be allocated");
    }
    
    this->x = x;
    this->z = z;

    this->blocks = malloc(CHUNK_WIDTH * CHUNK_HEIGHT * CHUNK_DEPTH * sizeof(uint8_t));
    if(this->blocks == NULL) {
        throwException("Memory couldn't be allocated for blocks");
    }

    memcpy(this->blocks, blocks, CHUNK_WIDTH * CHUNK_HEIGHT * CHUNK_DEPTH * sizeof(uint8_t));

    this->mesh = initMesh();
    updateChunkMesh(this);

    return this;
}

void destroyChunk(Chunk* this) {
    destroyMesh(this->mesh);
    free(this->blocks);
    free(this);
}

void addFaceToChunkMesh(Block block, Face face, DynamicArray* vertices, DynamicArray* indices, int x, int y, int z);
void addIndicesToChunkMesh(DynamicArray* indices, unsigned int startIndex);
void addVertexToChunkMesh(Face face, DynamicArray* vertices, float x, float y, float z, float texCoordX, float texCoordY);


void updateChunkMesh(Chunk* this) {

    INIT_MODULE_DYNAMIC_ARRAY;

    DynamicArray* vertices = createDynamicArray(sizeof(float), 8e3, true);
    DynamicArray* indices = createDynamicArray(sizeof(unsigned int), 2e3, true);

    // Es wird durch den blocks-Array iteriert und sämtliche Faces der Blöcke, die gesehen werden können werden dem neuen Chunk-Mesh hinzugefügt
    for(int x = 0; x < CHUNK_WIDTH; x++) {
        for(int y = 0; y < CHUNK_HEIGHT; y++) {
            for(int z = 0; z < CHUNK_DEPTH; z++) {

                if(BLOCK_AT(x, y, z) == BLOCK_AIR) {
                    continue;
                }

                if(BLOCK_AT(x, y, z) < 0 || BLOCK_AT(x, y, z) >= 4) {
                    printf("%d", BLOCK_AT(x, y, z));
                }

                Block currentBlock = TO_VALUE(Block) getFromDynamicArray(blockRegistry, BLOCK_AT(x, y, z));

                // (x, y, z) ist die Position des linken, unteren, hinteren Vertex
                // Falls Face sichtbar ist (nicht an Luft oder einen benachbarten Chunk grenzt), wird es dem Chunk-Mesh hinzugefügt
                if(z == CHUNK_DEPTH - 1 || BLOCK_AT(x, y, z + 1) == BLOCK_AIR) {
                    addFaceToChunkMesh(currentBlock, FRONT, vertices, indices, x, y, z);
                }
                if(z == 0 || BLOCK_AT(x, y, z - 1) == BLOCK_AIR) {
                    addFaceToChunkMesh(currentBlock, BACK, vertices, indices, x, y, z);
                }
                if(y == CHUNK_HEIGHT - 1 || BLOCK_AT(x, y + 1, z) == BLOCK_AIR) {
                    addFaceToChunkMesh(currentBlock, TOP, vertices, indices, x, y, z);
                }
                if(y == 0 || BLOCK_AT(x, y - 1, z) == BLOCK_AIR) {
                    addFaceToChunkMesh(currentBlock, BOTTOM, vertices, indices, x, y, z);
                }
                if(x == CHUNK_WIDTH - 1 || BLOCK_AT(x + 1, y, z) == BLOCK_AIR) {
                    addFaceToChunkMesh(currentBlock, RIGHT, vertices, indices, x, y, z);
                }
                if(x == 0 || BLOCK_AT(x - 1, y, z) == BLOCK_AIR) {
                    addFaceToChunkMesh(currentBlock, LEFT, vertices, indices, x, y, z);
                }


            }
        }
    }

    // Zu guter letzt werden die neu generierten Indices und Vertices in das VBO/EBO des Chunk-Meshes gesteckt

    generateMesh(this->mesh, (float*) vertices->ptr, vertices->len, (unsigned int*) indices->ptr, indices->len);

    DEINIT_MODULE_DYNAMIC_ARRAY;
}

void addFaceToChunkMesh(Block block, Face face, DynamicArray* vertices, DynamicArray* indices, int x, int y, int z) {

    unsigned int startIndex = vertices->len / 8;

    float texCoordX, texCoordY;

    switch(face) {

        case FRONT:
            // Sämtliche zum Face "FRONT" gehörige Vertices werden dem Mesh hinzugefügt
            texCoordX = block.frontTexture[0];
            texCoordY = block.frontTexture[1];
            addVertexToChunkMesh(face, vertices, x, y, z + 1, texCoordX, texCoordY);   
            addVertexToChunkMesh(face, vertices, x + 1, y, z + 1, texCoordX + RELATIVE_SUBTEXTURE_WIDTH, texCoordY);
            addVertexToChunkMesh(face, vertices, x + 1, y + 1, z + 1, texCoordX + RELATIVE_SUBTEXTURE_WIDTH, texCoordY + RELATIVE_SUBTEXTURE_HEIGHT);
            addVertexToChunkMesh(face, vertices, x, y + 1, z + 1, texCoordX, texCoordY + RELATIVE_SUBTEXTURE_HEIGHT);
            break;

        case BACK:
            // Sämtliche zum Face "BACK" gehörige Vertices werden dem Mesh hinzugefügt
            texCoordX = block.backTexture[0];
            texCoordY = block.backTexture[1];
            addVertexToChunkMesh(face, vertices, x, y + 1, z, texCoordX, texCoordY + RELATIVE_SUBTEXTURE_HEIGHT);
            addVertexToChunkMesh(face, vertices, x + 1, y + 1, z, texCoordX + RELATIVE_SUBTEXTURE_WIDTH, texCoordY + RELATIVE_SUBTEXTURE_HEIGHT);
            addVertexToChunkMesh(face, vertices, x + 1, y, z, texCoordX + RELATIVE_SUBTEXTURE_WIDTH, texCoordY);
            addVertexToChunkMesh(face, vertices, x, y, z, texCoordX, texCoordY);
            break;

        case TOP:
            // Sämtliche zum Face "TOP" gehörige Vertices werden dem Mesh hinzugefügt
            texCoordX = block.topTexture[0];
            texCoordY = block.topTexture[1];
            addVertexToChunkMesh(face, vertices, x, y + 1, z + 1, texCoordX, texCoordY);
            addVertexToChunkMesh(face, vertices, x + 1, y + 1, z + 1, texCoordX + RELATIVE_SUBTEXTURE_WIDTH, texCoordY);
            addVertexToChunkMesh(face, vertices, x + 1, y + 1, z, texCoordX + RELATIVE_SUBTEXTURE_WIDTH, texCoordY + RELATIVE_SUBTEXTURE_HEIGHT);
            addVertexToChunkMesh(face, vertices, x, y + 1, z, texCoordX, texCoordY + RELATIVE_SUBTEXTURE_HEIGHT);
            break;

        case BOTTOM:
            // Sämtliche zum Face "BOTTOM" gehörige Vertices werden dem Mesh hinzugefügt
            texCoordX = block.bottomTexture[0];
            texCoordY = block.bottomTexture[1];
            addVertexToChunkMesh(face, vertices, x, y, z, texCoordX, texCoordY + RELATIVE_SUBTEXTURE_HEIGHT);
            addVertexToChunkMesh(face, vertices, x + 1, y, z, texCoordX + RELATIVE_SUBTEXTURE_WIDTH, texCoordY + RELATIVE_SUBTEXTURE_HEIGHT);
            addVertexToChunkMesh(face, vertices, x + 1, y, z + 1, texCoordX + RELATIVE_SUBTEXTURE_WIDTH, texCoordY);
            addVertexToChunkMesh(face, vertices, x, y, z + 1, texCoordX, texCoordY);
            break;

        case RIGHT:
            // Sämtliche zum Face "RIGHT" gehörige Vertices werden dem Mesh hinzugefügt
            texCoordX = block.rightTexture[0];
            texCoordY = block.rightTexture[1];
            addVertexToChunkMesh(face, vertices, x + 1, y, z + 1, texCoordX, texCoordY);
            addVertexToChunkMesh(face, vertices, x + 1, y, z, texCoordX + RELATIVE_SUBTEXTURE_WIDTH, texCoordY);
            addVertexToChunkMesh(face, vertices, x + 1, y + 1, z, texCoordX + RELATIVE_SUBTEXTURE_WIDTH, texCoordY + RELATIVE_SUBTEXTURE_HEIGHT);
            addVertexToChunkMesh(face, vertices, x + 1, y + 1, z + 1, texCoordX, texCoordY + RELATIVE_SUBTEXTURE_HEIGHT);
            break;

        case LEFT:
            // Sämtliche zum Face "LEFT" gehörige Vertices werden dem Mesh hinzugefügt
            texCoordX = block.leftTexture[0];
            texCoordY = block.leftTexture[1];
            addVertexToChunkMesh(face, vertices, x, y + 1, z + 1, texCoordX, texCoordY + RELATIVE_SUBTEXTURE_HEIGHT);
            addVertexToChunkMesh(face, vertices, x, y + 1, z, texCoordX + RELATIVE_SUBTEXTURE_WIDTH, texCoordY + RELATIVE_SUBTEXTURE_HEIGHT);
            addVertexToChunkMesh(face, vertices, x, y, z, texCoordX + RELATIVE_SUBTEXTURE_WIDTH, texCoordY);
            addVertexToChunkMesh(face, vertices, x, y, z + 1, texCoordX, texCoordY);
            break;
    }

    addIndicesToChunkMesh(indices, startIndex);
}

void addIndicesToChunkMesh(DynamicArray* indices, unsigned int startIndex) {
    unsigned int value = startIndex;
    addToDynamicArray(indices, &value);
    value = startIndex + 1;
    addToDynamicArray(indices, &value);
    value = startIndex + 2;
    addToDynamicArray(indices, &value);
    value = startIndex;
    addToDynamicArray(indices, &value);
    value = startIndex + 2;
    addToDynamicArray(indices, &value);
    value = startIndex + 3;
    addToDynamicArray(indices, &value);
}

void addVertexToChunkMesh(Face face, DynamicArray* vertices, float x, float y, float z, float texCoordX, float texCoordY) {

    // aPos-Attribut des Vertex wird gesetzt
    addToDynamicArray(vertices, &x);
    addToDynamicArray(vertices, &y);
    addToDynamicArray(vertices, &z);

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
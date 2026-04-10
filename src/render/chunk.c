#include "mesh.h"
#include "chunk.h"
#include "block.h"
#include "utils.h"
#include "shader.h"
#include "dynamicArray.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define BLOCK_AT(x, y, z) (this->blocks[x * CHUNK_HEIGHT * CHUNK_DEPTH + y * CHUNK_DEPTH + z])

Chunk* createChunk(Block* blocks, float x, float z) {

    Chunk* this = malloc(sizeof(Chunk));

    if(this == NULL) {
        throwException("Memory couldn't be allocated");
    }
    
    this->x = x;
    this->z = z;

    this->blocks = malloc(sizeof(Block) * CHUNK_WIDTH * CHUNK_HEIGHT * CHUNK_DEPTH);
    if(this->blocks == NULL) {
        free(this);
        throwException("Memory couldn't be allocated for blocks");
    }

    memcpy(this->blocks, blocks, sizeof(Block) * CHUNK_WIDTH * CHUNK_HEIGHT * CHUNK_DEPTH);

    this->mesh = initMesh();
    updateChunkMesh(this);

    return this;
}

void destroyChunk(Chunk* this) {
    if(this->mesh != NULL) {
        destroyMesh(this->mesh);
    }
    free(this->blocks);
    free(this);
}

void addFaceToChunkMesh(Block block, Face face, DynamicArray* vertices, DynamicArray* indices, int x, int y, int z);
void addIndicesToChunkMesh(DynamicArray* indices, unsigned int startIndex);
void addVertexToChunkMesh(Face face, DynamicArray* vertices, float x, float y, float z, float texCoordX, float texCoordY);


void updateChunkMesh(Chunk* this) {

    DynamicArray* vertices = createDynamicArray(sizeof(float), 8e3, true);
    DynamicArray* indices = createDynamicArray(sizeof(unsigned int), 2e3, true);

    // Es wird durch den blocks-Array iteriert und sämtliche Faces der Blöcke, die gesehen werden können werden dem neuen Chunk-Mesh hinzugefügt
    for(int x = 0; x < CHUNK_WIDTH; x++) {
        for(int y = 0; y < CHUNK_HEIGHT; y++) {
            for(int z = 0; z < CHUNK_DEPTH; z++) {

                if(BLOCK_AT(x, y, z).id == AIR) {
                    continue;
                }

                // (x, y, z) ist die Position des linken, unteren, hinteren Vertex
                // Falls Face sichtbar ist (nicht an Luft oder einen benachbarten Chunk grenzt), wird es dem Chunk-Mesh hinzugefügt
                if(z == CHUNK_DEPTH - 1 || BLOCK_AT(x, y, z + 1).id == AIR) {
                    addFaceToChunkMesh(BLOCK_AT(x, y, z), FRONT, vertices, indices, x, y, z);
                }
                if(z == 0 || BLOCK_AT(x, y, z - 1).id == AIR) {
                    addFaceToChunkMesh(BLOCK_AT(x, y, z), BACK, vertices, indices, x, y, z);
                }
                if(y == CHUNK_HEIGHT - 1 || BLOCK_AT(x, y + 1, z).id == AIR) {
                    addFaceToChunkMesh(BLOCK_AT(x, y, z), TOP, vertices, indices, x, y, z);
                }
                if(y == 0 || BLOCK_AT(x, y - 1, z).id == AIR) {
                    addFaceToChunkMesh(BLOCK_AT(x, y, z), BOTTOM, vertices, indices, x, y, z);
                }
                if(x == CHUNK_WIDTH - 1 || BLOCK_AT(x + 1, y, z).id == AIR) {
                    addFaceToChunkMesh(BLOCK_AT(x, y, z), RIGHT, vertices, indices, x, y, z);
                }
                if(x == CHUNK_WIDTH - 1 || BLOCK_AT(x + 1, y, z).id == AIR) {
                    addFaceToChunkMesh(BLOCK_AT(x, y, z), LEFT, vertices, indices, x, y, z);
                }

            }
        }
    }

    // Zu guter letzt werden die neu generierten Indices und Vertices in das VBO/EBO des Chunk-Meshes gesteckt

    generateMesh(this->mesh, (float*) vertices->ptr, vertices->len, (unsigned int*) indices->ptr, indices->len);

    destroyDynamicArray(indices);
    destroyDynamicArray(vertices);
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
    glm_translate(model, (vec3) {this->x, -CHUNK_HEIGHT / 2, this->z});
    setMatrix(shader, "model", model);
    renderMesh(this->mesh);
}
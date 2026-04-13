#include <glad/glad.h>

#include "mesh.h"
#include "utils.h"

#include <stdlib.h>

Mesh* initMesh() {

    Mesh* this = malloc(sizeof(Mesh));

    if(this == NULL) {
        throwException("Memory couldn't be allocated");
    }

    // Buffers und VAO werden initialisiert
    glGenVertexArrays(1, &this->VAO);
    glGenBuffers(1, &this->VBO);
    glGenBuffers(1, &this->EBO);

    glBindVertexArray(this->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, this->VBO);

    // aPos-Attribut wird konfiguriert
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*) 0);
    glEnableVertexAttribArray(0);

    // aNormal-Attribut wird konfiguriert
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*) (3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // aTexCoord-Attribut wird konfiguriert
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*) (6 * sizeof(float)));  
    glEnableVertexAttribArray(2);
      
    return this;
}

void destroyMesh(Mesh* this) {
    glDeleteVertexArrays(1, &this->VAO);
    glDeleteBuffers(1, &this->VBO);
    glDeleteBuffers(1, &this->EBO);
    free(this);
}

void generateMesh(Mesh* this, float* vertices, int lenVertices, unsigned int* indices, int lenIndices) {

    glBindVertexArray(this->VAO);

    // Vertices werden in den VBO geladen
    glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
    glBufferData(GL_ARRAY_BUFFER, lenVertices * sizeof(float), vertices, GL_DYNAMIC_DRAW);

    // Indices werden in den EBO geladen
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, lenIndices * sizeof(unsigned int), indices, GL_DYNAMIC_DRAW);

    this->lenIndices = lenIndices;
}

void renderMesh(Mesh* this) {
    glBindVertexArray(this->VAO);
    glDrawElements(GL_TRIANGLES, this->lenIndices, GL_UNSIGNED_INT, 0);
}
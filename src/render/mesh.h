#ifndef MESH_H
#define MESH_H

typedef struct {
    // Nötige Renderdaten
    unsigned int VBO, EBO, VAO;
    // Zum Rendern benötigte Länge der Indices
    int lenIndices;
} Mesh;

Mesh* initMesh();
void destroyMesh(Mesh* this);
void generateMesh(Mesh* this, float* vertices, int lenVertices, unsigned int* indices, int lenIndices);
void renderMesh(Mesh* this);

#endif
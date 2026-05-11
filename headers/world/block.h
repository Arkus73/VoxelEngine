#ifndef BLOCK_H
#define BLOCK_H

#include "dynamicArray.h"

typedef float texCoord[2];

typedef struct {

    // texCoords, die angeben welche Subtextur des Texture Atlas für welche Seite genutzt werden soll
    texCoord frontTexture;
    texCoord backTexture;
    texCoord topTexture;
    texCoord bottomTexture;
    texCoord rightTexture;
    texCoord leftTexture;

} Block;

// Atlas-Werte
#define ATLAS_WIDTH 600.0f
#define ATLAS_HEIGHT 600.0f
#define SUBTEXTURE_WIDTH 37.5f
#define SUBTEXTURE_HEIGHT 37.5f
#define EPSILON 0.001f  // Korrigierender Summand
#define RELATIVE_SUBTEXTURE_WIDTH SUBTEXTURE_WIDTH / ATLAS_WIDTH - EPSILON
#define RELATIVE_SUBTEXTURE_HEIGHT SUBTEXTURE_HEIGHT / ATLAS_HEIGHT - EPSILON

// Globale Blöcke
extern DynamicArray* blockRegistry;

#define BLOCK_AIR 0
#define BLOCK_DIRT 1
#define BLOCK_GRASS 2
#define BLOCK_STONE 3

void initBlocks();
void destroyBlocks();

#endif
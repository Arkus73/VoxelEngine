#ifndef BLOCK_H
#define BLOCK_H

typedef float texCoord[2];

// Block-IDs, die zur eindeutigen Identifizierung eines Blocks nötig sind
typedef enum {
    AIR,
    GRASS,
    DIRT
} BlockId;

typedef struct {
    
    BlockId id;

    // texCoords, die angeben welche Subtextur des Texture Atlas für welche Seite genutzt werden soll
    texCoord frontTexture;
    texCoord backTexture;
    texCoord topTexture;
    texCoord bottomTexture;
    texCoord rightTexture;
    texCoord leftTexture;

} Block;

// Globale Blöcke
extern Block air;
extern Block dirt;
extern Block grass;

#endif
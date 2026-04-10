#include "block.h"

// Definition sämtlicher globaler Blöcke
#define createBlock(id, frontTexture, backTexture, topTexture, bottomTexture, rightTexture, leftTexture) {id, frontTexture, backTexture, topTexture, bottomTexture, rightTexture, leftTexture}
#define TC(x, y) (texCoord){x / ATLAS_WIDTH, y / ATLAS_HEIGHT}

Block air = createBlock(AIR, TC(0, 0), TC(0, 0), TC(0, 0), TC(0, 0), TC(0, 0), TC(0, 0));
Block dirt = createBlock(DIRT, TC(13 * SUBTEXTURE_WIDTH, ATLAS_HEIGHT - SUBTEXTURE_HEIGHT), TC(13 * SUBTEXTURE_WIDTH, ATLAS_HEIGHT - SUBTEXTURE_HEIGHT), TC(13 * SUBTEXTURE_WIDTH, ATLAS_HEIGHT - SUBTEXTURE_HEIGHT), TC(13 * SUBTEXTURE_WIDTH, ATLAS_HEIGHT - SUBTEXTURE_HEIGHT), TC(13 * SUBTEXTURE_WIDTH, ATLAS_HEIGHT - SUBTEXTURE_HEIGHT), TC(13 * SUBTEXTURE_WIDTH, ATLAS_HEIGHT - SUBTEXTURE_HEIGHT));
Block grass = createBlock(GRASS, TC(12 * SUBTEXTURE_WIDTH, 0), TC(12 * SUBTEXTURE_WIDTH, 0), TC(7 * SUBTEXTURE_WIDTH, 2 * SUBTEXTURE_HEIGHT), TC(13 * SUBTEXTURE_WIDTH, 0), TC(12 * SUBTEXTURE_WIDTH, 0), TC(12 * SUBTEXTURE_WIDTH, 0));
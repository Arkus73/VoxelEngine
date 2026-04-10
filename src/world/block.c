#include "block.h"

// Definition sämtlicher globaler Blöcke
#define createBlock(id, frontTexture, backTexture, topTexture, bottomTexture, rightTexture, leftTexture) {id, frontTexture, backTexture, topTexture, bottomTexture, rightTexture, leftTexture}
#define SUBTEXTURE_WIDTH 37.5f
#define SUBTEXTURE_HEIGHT 37.5f
#define TC(x, y) (texCoord){ x, y }

Block air = createBlock(AIR, TC(0, 0), TC(0, 0), TC(0, 0), TC(0, 0), TC(0, 0), TC(0, 0));
Block dirt = createBlock(DIRT, TC(13 * SUBTEXTURE_WIDTH, 0), TC(13 * SUBTEXTURE_WIDTH, 0), TC(13 * SUBTEXTURE_WIDTH, 0), TC(13 * SUBTEXTURE_WIDTH, 0), TC(13 * SUBTEXTURE_WIDTH, 0), TC(13 * SUBTEXTURE_WIDTH, 0));
Block grass = createBlock(GRASS, TC(12 * SUBTEXTURE_WIDTH, 0), TC(12 * SUBTEXTURE_WIDTH, 0), TC(7 * SUBTEXTURE_WIDTH, 2 * SUBTEXTURE_HEIGHT), TC(13 * SUBTEXTURE_WIDTH, 0), TC(12 * SUBTEXTURE_WIDTH, 0), TC(12 * SUBTEXTURE_WIDTH, 0));
#include "block.h"
#include "dynamicArray.h"

// Definition sämtlicher globaler Blöcke
#define createBlock(id, frontTexture, backTexture, topTexture, bottomTexture, rightTexture, leftTexture) {frontTexture, backTexture, topTexture, bottomTexture, rightTexture, leftTexture}
#define TC(x, y) (texCoord){x / ATLAS_WIDTH, y / ATLAS_HEIGHT}

// 0, 0 ist in der linken, oberen Ecke
Block air = createBlock(AIR, TC(0, 0), TC(0, 0), TC(0, 0), TC(0, 0), TC(0, 0), TC(0, 0));
Block dirt = createBlock(DIRT, TC(13 * SUBTEXTURE_WIDTH, ATLAS_HEIGHT - SUBTEXTURE_HEIGHT), TC(13 * SUBTEXTURE_WIDTH, ATLAS_HEIGHT - SUBTEXTURE_HEIGHT), TC(13 * SUBTEXTURE_WIDTH, ATLAS_HEIGHT - SUBTEXTURE_HEIGHT), TC(13 * SUBTEXTURE_WIDTH, ATLAS_HEIGHT - SUBTEXTURE_HEIGHT), TC(13 * SUBTEXTURE_WIDTH, ATLAS_HEIGHT - SUBTEXTURE_HEIGHT), TC(13 * SUBTEXTURE_WIDTH, ATLAS_HEIGHT - SUBTEXTURE_HEIGHT));
Block grass = createBlock(GRASS, TC(12 * SUBTEXTURE_WIDTH, ATLAS_HEIGHT - SUBTEXTURE_HEIGHT), TC(12 * SUBTEXTURE_WIDTH, ATLAS_HEIGHT - SUBTEXTURE_HEIGHT), TC(7 * SUBTEXTURE_WIDTH, 13 * SUBTEXTURE_HEIGHT), TC(13 * SUBTEXTURE_WIDTH, ATLAS_HEIGHT - SUBTEXTURE_HEIGHT), TC(12 * SUBTEXTURE_WIDTH, ATLAS_HEIGHT - SUBTEXTURE_HEIGHT), TC(12 * SUBTEXTURE_WIDTH, ATLAS_HEIGHT - SUBTEXTURE_HEIGHT));
Block stone = createBlock(STONE, TC(14 * SUBTEXTURE_WIDTH, ATLAS_HEIGHT - SUBTEXTURE_HEIGHT), TC(14 * SUBTEXTURE_WIDTH, ATLAS_HEIGHT - SUBTEXTURE_HEIGHT), TC(14 * SUBTEXTURE_WIDTH, ATLAS_HEIGHT - SUBTEXTURE_HEIGHT), TC(14 * SUBTEXTURE_WIDTH, ATLAS_HEIGHT - SUBTEXTURE_HEIGHT), TC(14 * SUBTEXTURE_WIDTH, ATLAS_HEIGHT - SUBTEXTURE_HEIGHT), TC(14 * SUBTEXTURE_WIDTH, ATLAS_HEIGHT - SUBTEXTURE_HEIGHT));
DynamicArray* blockRegistry;

void initBlocks() {
    blockRegistry = createDynamicArray(sizeof(Block), 10, false);
    addToDynamicArray(blockRegistry, &air);
    addToDynamicArray(blockRegistry, &dirt);
    addToDynamicArray(blockRegistry, &grass);
    addToDynamicArray(blockRegistry, &stone);
}

void destroyBlocks() {
    destroyDynamicArray(blockRegistry);
}
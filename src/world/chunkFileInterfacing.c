#include "chunkFileInterfacing.h"
#include "utils.h"
#include "consts.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

void saveChunk(uint8_t* blocks, int gcx, int gcz) {
    char filename[256];     // Dateiname wird generiert
    snprintf(filename, sizeof(filename), "../chunks/chunk_%d_%d.bin", gcx, gcz);    

    FILE* file = fopen(filename, "wb");     // Datei wird geöffnet
    if (file == NULL) {
        throwException("Chunkfile couldn't be opened for writing");
    }

    size_t size = CHUNK_WIDTH * CHUNK_HEIGHT * CHUNK_DEPTH; 
    if (fwrite(blocks, 1, size, file) != size) {    // Datei wird beschrieben.
        throwException("Could not write full chunk");
    }

    fclose(file);   // Datei wird geschlossen
}

uint8_t* loadChunkData(int gcx, int gcz) {
    char filename[256];     // Dateiname wird generiert
    snprintf(filename, sizeof(filename), "../chunks/chunk_%d_%d.bin", gcx, gcz);

    FILE* file = fopen(filename, "rb");     // Datei wird geöffnet
    if (file == NULL) {
        throwException("Chunkfile couldn't be opened for reading");
    }

    size_t size = CHUNK_WIDTH * CHUNK_HEIGHT * CHUNK_DEPTH;

    uint8_t* buffer = malloc(size); // Buffer für die Ausgabe wird allokiert
    if (buffer == NULL) {
        throwException("Memory allocation failed");
    }

    if (fread(buffer, 1, size, file) != size) { // Chunkfile Inhalt wird in den Buffer gelesen
        throwException("Chunk file corrupted or incomplete");
    }

    fclose(file);   // Datei wird geschlossen und der Buffer ausgegeben
    return buffer;
}


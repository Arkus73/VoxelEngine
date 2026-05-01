#include <stdlib.h>
#include <stdbool.h>

#ifndef DYNAMIC_ARRAY_H
#define DYNAMIC_ARRAY_H

typedef struct {
    void* ptr;
    int len;
    int capacity;
    size_t elementSize;
} DynamicArray;

#define TO_VALUE(type) *(type*)

void destroyDynamicArray(DynamicArray* this);
DynamicArray* createDynamicArray(size_t elementSize, int startCapacity);
void endInitDynamicArray(DynamicArray* this);
void addToDynamicArray(DynamicArray* this, void* item);
void removeByIndexFromDynamicArray(DynamicArray* this, int index);
void removeByValueFromDynamicArray(DynamicArray* this, void* value);
void* getFromDynamicArray(DynamicArray* this, int index);
bool contains(DynamicArray* this, void* value);

#endif
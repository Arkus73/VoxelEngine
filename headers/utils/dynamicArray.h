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

extern DynamicArray* dynamicArrays;

#define TO_VALUE(type) *(type*)
#define INIT_MODULE_DYNAMIC_ARRAY {dynamicArrays = createDynamicArray(sizeof(DynamicArray*), 5, false);}
#define DEINIT_MODULE_DYNAMIC_ARRAY for(int i = 0; i < dynamicArrays->len; i++) {destroyDynamicArray(TO_VALUE(DynamicArray*)getFromDynamicArray(dynamicArrays, i));} destroyDynamicArray(dynamicArrays);

void destroyDynamicArray(DynamicArray* this);
DynamicArray* createDynamicArray(size_t elementSize, int startCapacity, bool registered);
void endInitDynamicArray(DynamicArray* this);
void addToDynamicArray(DynamicArray* this, void* item);
void removeByIndexFromDynamicArray(DynamicArray* this, int index);
void removeByValueFromDynamicArray(DynamicArray* this, void* value);
void* getFromDynamicArray(DynamicArray* this, int index);
bool contains(DynamicArray* this, void* value);

#endif
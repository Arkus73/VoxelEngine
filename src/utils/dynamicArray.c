#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "dynamicArray.h"
#include "utils.h"

#define ADDRESS_AT_INDEX(this, index) ((char*)this->ptr + (index) * this->elementSize)

DynamicArray* dynamicArrays;

void destroyDynamicArray(DynamicArray* this) {
    free(this->ptr);
    free(this);
}

DynamicArray* createDynamicArray(size_t elementSize, int startCapacity, bool registered) {
    DynamicArray* this = malloc(sizeof(DynamicArray));
    if(this == NULL) {
        throwException("Couldn't allocate memory");
    }
    this->capacity = startCapacity;
    this->len = 0;
    this->elementSize = elementSize;
    this->ptr = malloc(elementSize * this->capacity);
    if(this->ptr == NULL) {
        free(this);
        throwException("Couldn't allocate memory");
    }
    if(registered) {
        addToDynamicArray(dynamicArrays, this);
    }
    return this;
}

void addToDynamicArray(DynamicArray* this, void* item) {
    this->len++;
    if(this->len > this->capacity) {
        this->capacity *= 2;
        void* newPtr = realloc(this->ptr, this->capacity * this->elementSize);
        if(newPtr == NULL) {
            throwException("Couldn't allocate memory");
        }
        this->ptr = newPtr;
    }
    memcpy(ADDRESS_AT_INDEX(this, this->len - 1), item, this->elementSize);
} 

void removeByIndexFromDynamicArray(DynamicArray* this, int index) {
    if(index < 0 || index >= this->len) {
        throwException("Index not possible");
    }

    for(int i = index; i < this->len - 1; i++) {
            memcpy(ADDRESS_AT_INDEX(this, i), ADDRESS_AT_INDEX(this, i + 1), this->elementSize);
    }

    this->len--;
    if(this->len <= this->capacity / 2) {
        this->capacity /= 2;
        void* newPtr = realloc(this->ptr, this->capacity * this->elementSize);
        if(newPtr == NULL) {
            throwException("Couldn't allocate memory");
        }
        this->ptr = newPtr;
    }
} 

void removeByValueFromDynamicArray(DynamicArray* this, void* value) {
    int index = -1;
    for(int i = 0; i < this->len; i++) {
        if(memcmp(ADDRESS_AT_INDEX(this, i), value, this->elementSize) == 0) {
            index = i;
            break;
        }
    }
    if(index == -1) {
        throwException("DynamicArray doesn't contain that value");
    }
    removeByIndexFromDynamicArray(this, index);
}

bool contains(DynamicArray* this, void* value) {
    bool doesContain = false;
    for(int i = 0; i < this->len; i++) {
        if(memcmp(ADDRESS_AT_INDEX(this, i), value, this->elementSize) == 0) {
            doesContain = true;
            break;
        }
    }
    return doesContain;
}

void* getFromDynamicArray(DynamicArray* this, int index) {
    if(index < 0 || index >= this->len) {
        return NULL;
    }
    return ADDRESS_AT_INDEX(this, index);
}
#ifndef QUEUE_H
#define QUEUE_H

#include <stdbool.h>

typedef struct QueueEntry{
    void* value;
    struct QueueEntry* next;
} QueueEntry;

typedef struct {
    QueueEntry* front,* rear;
} Queue;

Queue* createQueue();
void destroyQueue(Queue* this, bool freeValues);
void enqueueToQueue(Queue* this, void* value);
void dequeueFromQueue(Queue* this, bool freeValue);
void* popFromQueue(Queue* this);

#endif
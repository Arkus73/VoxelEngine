#include "queue.h"
#include "utils.h"

#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

QueueEntry* createQueueEntry(void* value) {
    QueueEntry* this = malloc(sizeof(QueueEntry));
    if(this == NULL) {
        throwException("Memory for QeueEntry couldn't be allocated");
    }
    this->value = value;
    this->next = NULL;
    return this;
}

void destroyQueueEntry(QueueEntry* this, bool freeValue) {
    if(freeValue) {
        free(this->value);
    }
    free(this);
}

Queue* createQueue() {
    Queue* this = malloc(sizeof(Queue));
    if(this == NULL) {
        throwException("Memory for Qeue couldn't be allocated");
    }
    this->front = this->rear = NULL;
    return this;
}

void destroyQueue(Queue* this, bool freeValues) {
    while(this->front != NULL) {
        dequeueFromQueue(this, freeValues);
    }
    free(this);
}

void enqueueToQueue(Queue* this, void* value) {
    QueueEntry* newEntry = createQueueEntry(value);
    if(this->front == NULL) {
        this->front = this->rear = newEntry;
    } else {
        this->rear->next = newEntry;
        this->rear = newEntry;
    }
}

void dequeueFromQueue(Queue* this, bool freeValue) {
    if(this->front == NULL) {
        return;
    } else if(this->front == this->rear) {
        destroyQueueEntry(this->front, freeValue);
        this->front = this->rear = NULL;
    } else {
        QueueEntry* nextFront = this->front->next;
        destroyQueueEntry(this->front, freeValue);
        this->front = nextFront;
    }
}

void* popFromQueue(Queue* this) {
    if(this->front == NULL) {
        return NULL;
    } else if(this->front == this->rear) {
        void* value = this->front->value;
        destroyQueueEntry(this->front, false);
        this->front = this->rear = NULL;
        return value;
    } else {
        QueueEntry* nextFront = this->front->next;
        void* value = this->front->value;
        destroyQueueEntry(this->front, false);
        this->front = nextFront;
        return value;
    }
}
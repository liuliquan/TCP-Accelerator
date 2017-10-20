//
// Created by root on 10/19/17.
//
#define LandataQueueIncluded

#ifndef headerIncluded
#include "headers.h"
#endif

#ifndef structIncluded
#include "types.h"
#endif

#ifndef constantsIncluded
#include "constants.h"
#endif

void Init_data(DATA_QUEUE* queue, int capacity);


// Queue is full when size becomes equal to the capacity
int isFull_data(DATA_QUEUE* queue);

// Queue is empty when size is 0
int isEmpty_data(DATA_QUEUE* queue);


// Function to add an item to the queue.  It changes rear and size
void enqueue_data(DATA_QUEUE* queue, DATA_RECORD* item);

// Function to remove an item from queue.  It changes front and size
int dequeue_data(DATA_QUEUE* queue);

// Function to get front of queue
DATA_RECORD* front_data(DATA_QUEUE* queue);


// Function to get rear of queue
DATA_RECORD* rear_data(DATA_QUEUE* queue);

//display data in queue
void displayDataOnLogger(DATA_QUEUE* queue, FILE* logfile);


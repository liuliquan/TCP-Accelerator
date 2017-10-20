//
// Created by root on 10/19/17.
//

#include "../headerFiles/writeInLogger.h"
#include "../headerFiles/LanDataQueue.h"

void Init_data(DATA_QUEUE* queue, int capacity)
{
    queue->capacity = capacity;
    queue->front = queue->size = 0;
    queue->rear = -1;
    queue->dataRecordArray = (DATA_RECORD**) malloc(queue->capacity * sizeof(DATA_RECORD*));
}

// Queue is full when size becomes equal to the capacity
int isFull_data(DATA_QUEUE* queue)
{
    return (queue->size == queue->capacity);
}

// Queue is empty when size is 0
int isEmpty_data(DATA_QUEUE* queue)
{
    return (queue->size == 0);
}

// Function to add an item to the queue.  It changes rear and size
void enqueue_data(DATA_QUEUE* queue, DATA_RECORD* item)
{

    if (isFull_data(queue))
        return;

    queue->rear = (queue->rear+1)%(queue->capacity);
    queue->dataRecordArray[queue->rear] = item;

    ++queue->size;
}

// Function to remove an item from queue.  It changes front and size
int dequeue_data(DATA_QUEUE* queue)
{
    if (isEmpty_data(queue))
        return INT_MIN;
    DATA_RECORD* item = queue->dataRecordArray[queue->front];
    queue->front=(queue->front+1)%(queue->capacity);
    queue->size = queue->size - 1;
    return item;
}

// Function to get front of queue
DATA_RECORD* front_data(DATA_QUEUE* queue)
{
    if (isEmpty_data(queue))
        return INT_MIN;
    return queue->dataRecordArray[queue->front];
}

// Function to get rear of queue
DATA_RECORD* rear_data(DATA_QUEUE* queue)
{
    if (isEmpty_data(queue))
        return INT_MIN;
    return queue->dataRecordArray[queue->rear];
}

void displayDataOnLogger(DATA_QUEUE* queue, FILE* logfile)
{

    int i=queue->front;
    if(queue->front<=queue->rear){

        for(;i<=queue->rear;i++)
        {
            fprintf(logfile,"data record : %d\n",i);
            fprintf(logfile,"sessionId : %d\n",queue->dataRecordArray[i]->sessionId);
            PrintData(queue->dataRecordArray[i]->tcpPayload,queue->dataRecordArray[i]->size,logfile);

        }
    }else{

        for(;(i%(queue->capacity))!=(queue->rear);++i)
        {
            fprintf(logfile,"data record : %d\n",i%(queue->capacity));
            fprintf(logfile,"sessionId : %d\n",queue->dataRecordArray[i%(queue->capacity)]->sessionId);
            PrintData(queue->dataRecordArray[i%(queue->capacity)]->tcpPayload,queue->dataRecordArray[i%(queue->capacity)]->size,logfile);
        }
    }


    return;
}

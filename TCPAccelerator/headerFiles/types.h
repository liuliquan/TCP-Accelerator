//
// Created by root on 10/19/17.
//
#ifndef headerIncluded
#include "headers.h"
#endif

#define structIncluded


/*...................define session type.........................*/
typedef struct session {
    u_int32_t sourceIP;
    u_int32_t destIP;
    u_int16_t sourcePort;
    u_int16_t destPort;
    u_int32_t sessionId;
    struct dataQueue* queue;

} SESSION;

/*.......................define record types...........................*/
typedef struct orderRecord {
    u_int32_t sessionId;
    u_int16_t location;
} ORDER_RECORD;

typedef struct dataRecord {
    u_int32_t sessionId;
    u_int16_t size;
    unsigned char *tcpPayload;

} DATA_RECORD;



/*..........................define data types for BSTs..............................*/
//define deletedID node for deleted BST
typedef struct idNode{
    int id;
    struct node* left;
    struct node* right;
} ID_NODE;


/*.........................define queues..................................*/
//define LAN data order queue parameters
typedef struct orderQueue{
    int front, rear, capacity, size;
    struct orderRecord** orderRecordArray;
} ORDER_QUEUE;

//define LAN data queue parameters
typedef struct dataQueue{
    int front, rear, capacity, size;
    struct dataRecord** dataRecordArray;
} DATA_QUEUE;

//dynamically growing array
typedef struct {
    char *array;
    size_t used;
    size_t size;
} Array;

//
// Created by root on 10/19/17.
//
#ifndef headerIncluded
#include "headers.h"
#endif


#define LCHILD(x) 2 * x + 1
#define RCHILD(x) 2 * x + 2
#define PARENT(x) (x-1)/2

typedef struct deletedIdNode{
    u_int16_t sessionId;
}DELETED_ID_NODE;

typedef struct minHeap{
    int size;
    DELETED_ID_NODE* heapArray;
} MIN_HEAP;

void initMinHeap(MIN_HEAP* min_heap){
    min_heap->size=0;

}

void swap(DELETED_ID_NODE* node1,DELETED_ID_NODE* node2)
{
    DELETED_ID_NODE temp;
    temp=*node1;
    *node1=*node2;
    *node2=temp;

}

void heapify(MIN_HEAP* min_heap,int root)
{


    int smallest= (LCHILD(root) < min_heap->size) && (min_heap->heapArray[root].sessionId < min_heap->heapArray[LCHILD(root)].sessionId) ? root : LCHILD(root);
    smallest=(RCHILD(root) < min_heap->size) && (min_heap->heapArray[smallest].sessionId < min_heap->heapArray[RCHILD(root)].sessionId) ? smallest : RCHILD(root);

    //printf("\n%d\n",smallest);

    if(smallest!=root){

        //swap(&(min_heap->heapArray[root]),&(min_heap->heapArray[smallest]));*/
        heapify(min_heap,smallest);
    }
}

void insertId(MIN_HEAP* min_heap,int deletedId)
{
    //re allocation of heap array according to size
    if(min_heap->size)
    {
        min_heap->heapArray=realloc(min_heap->heapArray,(min_heap->size+1)*sizeof(DELETED_ID_NODE));
    }else{
        min_heap->heapArray=malloc(sizeof(DELETED_ID_NODE));
    }

    //create new deleted session Id
    DELETED_ID_NODE newNode;
    newNode.sessionId=deletedId;

    int i=(min_heap->size)++;
    while(i && deletedId < min_heap->heapArray[PARENT(i)].sessionId )
    {
        min_heap->heapArray[i] = min_heap->heapArray[PARENT(i)];
        i=PARENT(i);
    }

    min_heap->heapArray[i]=newNode;


}

void deleteId(MIN_HEAP* min_heap){

    if(min_heap->size){
        min_heap->heapArray[0] = min_heap->heapArray[--(min_heap->size)];
        min_heap->heapArray = realloc(min_heap->heapArray,(min_heap->size)*sizeof(DELETED_ID_NODE));
        //printf("\n");
        //printHeap(min_heap);
        //printf("\n");
        heapify(min_heap,0);
    }else{
        printf("Heap Empty : CAN NOT DELETE ANY ITEM\n");
    }
}

void printHeap(MIN_HEAP* min_heap){

    int i=0;
    for(;i<min_heap->size;++i){
        printf("%d ",min_heap->heapArray[i]);
    }
}


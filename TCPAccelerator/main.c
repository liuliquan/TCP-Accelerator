/*............................include custom header files....................................*/
#ifndef headerIncluded
#include "../DataDeduplication/headerFiles/headers.h"
#endif

#include "../DataDeduplication/headerFiles/writeInLogger.h"
#include "../DataDeduplication/headerFiles/deletedIdBST.h"
#include "../DataDeduplication/headerFiles/deletedIdMinHeap.h"


#ifndef structIncluded
#include "../DataDeduplication/headerFiles/types.h"
#endif

#ifndef constantsIncluded
#include "../DataDeduplication/headerFiles/constants.h"
#endif


#ifndef LandataQueueIncluded
#include "../DataDeduplication/headerFiles/LanDataQueue.h"
#endif

#ifndef LanorderQueueIncluded
#include "../DataDeduplication/headerFiles/LanOrderQueue.h"
#include "headerFiles/deletedIdMinHeap.h"

#endif


/**
 * de-duplication variables
 */
#define MAX_CHUNK_SIZE 65536
#define AVERAGE_CHUNK_SIZE 4096
#define E 2.718281828

/**
 * HashTable size
 */
#define HASHSIZE 1000

/**
 * AE window size
 */
static int windowSize = AVERAGE_CHUNK_SIZE/(E-1);

/**
 * Initialize HashTable
 */
static struct nlist *hashtab[HASHSIZE];

/**
 * nlist structure for HashTable
 */
struct nlist {
    struct nlist *next;
    char *name;
    char *defn;
};

/**
 * Dynamic array
 */
typedef struct {
    char *array;
    size_t used;
    size_t size;
} Array;

/**
 * Function prototypes
 */
int isTcp(unsigned char* buffer, int size);
void sessionHandler(unsigned char* buffer,int size,struct iphdr *iph, struct tcphdr *tcph);
int getMinimumIdFromDeletedSessions();
int getId(unsigned char* buffer, int size);
unsigned char* getTCPPayload(unsigned char* buffer, int size);
int getTCPPayloadSize(unsigned char* buffer, int size);
void stringCopy(unsigned char* dest, unsigned char* src, int size);
void initArray(Array *a, size_t initialSize);
void insertArray(Array *a, char element);
void freeArray(Array *a);
static char *mystrdup(char *s) ;
static unsigned hash(char *s);
static struct nlist *lookup(char *s);
static struct nlist *insertData(char *name, char *defn);
static void undef(char *s);
int chunkData(unsigned char *buffer, int n);
int comp(unsigned char *i,unsigned char *max);

/**
 * Reader and de-duplication Thread
 */
void* readerThread(void);
void* dedupThread(void);

/**
 * Variable definitions
 */
FILE *logfile, *queueLogfile_1, *queueLogfile_2, *orderQueueLogfile, *testLogfile,*testLogfile1;
struct dataRecord dataFromLanBuf[MAX_SESSIONS];
int firstRoundUpdate=-1, minimumId, freeSessionId;
struct deletedIdNode* root;
ORDER_QUEUE* orderQueue;


/*create session buffer*/
struct session sessionBuf[MAX_SESSIONS];

int main()
{
    MIN_HEAP min_heap;
    initMinHeap(&min_heap);
    insertId(&min_heap,10);
    insertId(&min_heap,8);
    insertId(&min_heap,1);
    insertId(&min_heap,25);
    insertId(&min_heap,78);
    insertId(&min_heap,4);
    insertId(&min_heap,2);
    insertId(&min_heap,12);
    deleteId(&min_heap);

    /*set ids for threads*/
    pthread_t readerThread_id, dedupThread_id;

    /*set attributes for threads*/
    pthread_attr_t attr;
    pthread_attr_init(&attr);

    pthread_create (&readerThread_id, &attr,&readerThread, NULL);
    pthread_create (&dedupThread_id, &attr,&dedupThread, NULL);

    /*wait until reader thread exits*/
    pthread_join(readerThread_id, NULL);

    return 0;
}


/**
 * Reader Thread
 */
void* readerThread(void)
{

    /*create order queue*/
    orderQueue=(ORDER_RECORD* )malloc(sizeof(ORDER_RECORD));
    Init_order(orderQueue, MAX_WAITING_PACKETS);

    /*packet read variables*/
    int saddr_size , data_size, k, tcpPayloadSize, sessionId;
    struct sockaddr saddr;
    unsigned char* tcpPayloadBuffer;
    DATA_RECORD* tempDataRecord;
    ORDER_RECORD* tempOrderRecord;
    unsigned char *buffer = (unsigned char *) malloc(MSS);

    logfile=fopen("log.txt","w");
    queueLogfile_1=fopen("queueLogfile_1.txt","w");
    queueLogfile_2=fopen("queueLogfile_2.txt","w");
    orderQueueLogfile=fopen("orderQueueLogfile.txt","w");
    testLogfile=fopen("testLogfile.txt","w");
    testLogfile1=fopen("testLogfile1.txt","w");

    if(logfile==NULL){ printf("Unable to create log.txt file.");}

    int sock_raw = socket( AF_PACKET , SOCK_RAW , htons(ETH_P_ALL)) ;
    if(sock_raw < 0)
    {
        //Print the error with proper message
        perror("Socket Error");
    }

    for(k=0;k<1000;k++)
    {
        saddr_size = sizeof saddr;
        //Receive a packet
        data_size = recvfrom(sock_raw , buffer , 1460 , 0 , &saddr , (socklen_t*)&saddr_size);
        if(data_size <0 )
        {
            printf("Recvfrom error , failed to get packets\n");

        }
        if(isTcp(buffer,data_size))//check whether the packet is TCP
        {
            tcpPayloadBuffer=getTCPPayload(buffer, data_size);
            tcpPayloadSize=getTCPPayloadSize(buffer, data_size);
            sessionId=getId(buffer,data_size);

            if(sessionBuf[sessionId].queue)
            {
                /*..................data record processing.......................*/
                //create a new data Record
                tempDataRecord=(DATA_RECORD*)malloc(sizeof(DATA_RECORD));
                tempDataRecord->sessionId=sessionId;
                tempDataRecord->size=tcpPayloadSize;
                tempDataRecord->tcpPayload=(unsigned char*)malloc(tcpPayloadSize*sizeof(unsigned char));
                stringCopy(tempDataRecord->tcpPayload,tcpPayloadBuffer,tcpPayloadSize);

                //enqueue the data Record
                enqueue_data(sessionBuf[sessionId].queue, tempDataRecord);

                /*..................data order processing.......................*/
                //create a new order record
                tempOrderRecord=(ORDER_RECORD* )malloc(sizeof(ORDER_RECORD));
                tempOrderRecord->sessionId=sessionId;
                tempOrderRecord->location=sessionBuf[sessionId].queue->rear;

                //enqueue the order Record
                enqueue_order(orderQueue, tempOrderRecord);

            }

            fprintf(logfile,"sessionId : %d\n",sessionId);
            print_tcp_packet(buffer, data_size, logfile);
            printf("Packet is TCP\n");
        }

    }

    Array dataArray;
    initArray(&dataArray, 8);

    for(int limit =0;limit<orderQueue->size;limit++){

        u_int32_t sessID = orderQueue->orderRecordArray[orderQueue->front+limit]->sessionId;
        u_int16_t loc = orderQueue->orderRecordArray[orderQueue->front+limit]->location;

        DATA_QUEUE* dQueue =  sessionBuf[sessID].queue;

        u_int16_t len = dQueue->dataRecordArray[loc]->size;
        u_int32_t id = dQueue->dataRecordArray[loc]->sessionId;

        int position = 3;
        char lenArray[4] = {'0','0','0','0'};
        while(len !=0){
            lenArray[position--] = (char)(len%10+48);
            len = len/10;
        }

        position = 3;
        char idArray[4]={'0','0','0','0'};
        while(id !=0){
            idArray[position--] = (char)(id%10+48);
            id = id/10;
        }

        for(int k=0;k<4;k++){
            insertArray(&dataArray,idArray[k]);
        }
        for(int k=0;k<4;k++){
            insertArray(&dataArray,lenArray[k]);
        }

        unsigned char *dataLoad = dQueue->dataRecordArray[loc]->tcpPayload;

        while(*dataLoad !='\0'){
           insertArray(&dataArray,*(char*)(dataLoad++));
        }
    }

    freeArray(&dataArray);

    /*print data in different session data queues*/
    displayDataOnLogger(sessionBuf[0].queue, queueLogfile_1);
    displayDataOnLogger(sessionBuf[1].queue, queueLogfile_2);
    displayOrderOnLogger(orderQueue, orderQueueLogfile);

    /*access and print data using order queue*/
    PrintData(sessionBuf[orderQueue->orderRecordArray[orderQueue->front+5]->sessionId].queue->dataRecordArray[orderQueue->orderRecordArray[orderQueue->front+5]->location]->tcpPayload,
              sessionBuf[orderQueue->orderRecordArray[orderQueue->front+5]->sessionId].queue->dataRecordArray[orderQueue->orderRecordArray[orderQueue->front+5]->location]->size,testLogfile);

    PrintData(sessionBuf[orderQueue->orderRecordArray[orderQueue->front+6]->sessionId].queue->dataRecordArray[orderQueue->orderRecordArray[orderQueue->front+6]->location]->tcpPayload,
              sessionBuf[orderQueue->orderRecordArray[orderQueue->front+6]->sessionId].queue->dataRecordArray[orderQueue->orderRecordArray[orderQueue->front+6]->location]->size,testLogfile);

    printf("Finished");
    pthread_exit(0);

}

void* dedupThread(void)
{
    //printf("Inside the deduplication Thread");
    pthread_exit(0);
}




/*..............................function definitions............................................*/
int isTcp(unsigned char* buffer, int size)
{
    int iphdrlen;
    struct tcphdr *tcph;
    struct iphdr *iph = (struct iphdr*)(buffer + sizeof(struct ethhdr));
    iphdrlen = iph->ihl*4;//set ip header length


    if (iph->protocol == 6) //Check the Protocol and do accordingly...
    {
        tcph=(struct tcphdr*)(buffer + iphdrlen + sizeof(struct ethhdr));//set tcp header
        sessionHandler(buffer,size,iph, tcph);
        return 1;
    }

    return 0;
}

void sessionHandler(unsigned char* buffer,int size,struct iphdr *iph, struct tcphdr *tcph)
{
    int freeId;
    if(tcph->syn==1)
    {
        /*...................create new session..................................*/
        minimumId=getMinimumIdFromDeletedSessions();
        if(minimumId==-1)
        {
            /*there is no deleted sessions upto now*/
            if(firstRoundUpdate+1<MAX_SESSIONS)
            {
                /*session buffer is not full in first round*/
                ++firstRoundUpdate;
                sessionBuf[firstRoundUpdate].destIP=iph->daddr;
                sessionBuf[firstRoundUpdate].sourceIP=iph->saddr;
                sessionBuf[firstRoundUpdate].sourcePort=tcph->source;
                sessionBuf[firstRoundUpdate].destPort=tcph->dest;
                sessionBuf[firstRoundUpdate].sessionId=firstRoundUpdate;

                /*create a data queue for created session*/
                sessionBuf[firstRoundUpdate].queue=(DATA_QUEUE*)malloc(sizeof(DATA_QUEUE));
                Init_data(sessionBuf[firstRoundUpdate].queue, MAX_RECORDS_PER_SESSION);

                printf("session %d is successfully created \n",firstRoundUpdate);
            }else{
                /*session buffer is full*/
                printf("No of sessions that can be handled exceeded\n");
            }

        }else{
            /*there are deleted sessions upto now*/
            sessionBuf[minimumId].destIP=iph->daddr;
            sessionBuf[minimumId].sourceIP=iph->saddr;
            sessionBuf[minimumId].sourcePort=tcph->source;
            sessionBuf[minimumId].destPort=tcph->dest;
            sessionBuf[minimumId].sessionId=minimumId;

            /*create a data queue for created session*/
            sessionBuf[minimumId].queue=(DATA_QUEUE*)malloc(sizeof(DATA_QUEUE));
            Init_data(sessionBuf[minimumId].queue, MAX_RECORDS_PER_SESSION);


            printf("session %d is successfully created \n",minimumId);

            /*...............delete element from deletedBST....................*/
            root=deleteNode(root,minimumId);

        }

    }else if(tcph->fin==1)
    {
        freeId=getId(buffer,size);
        /*.........................delete unused session.............................*/
        printf("session %d is successfully deleted\n",freeId);

        /*.................add element to deletedBST..........................*/
        root=insert(root,freeSessionId);
    }

}

int getMinimumIdFromDeletedSessions()
{
    if(root==NULL)
        return -1;
    else
        return minValueNode(root)->id;
}

int getId(unsigned char* buffer, int size)
{
    int iphdrlen;
    struct tcphdr *tcph;
    struct iphdr *iph = (struct iphdr*)(buffer + sizeof(struct ethhdr));
    iphdrlen = iph->ihl*4;//set ip header length
    tcph=(struct tcphdr*)(buffer + iphdrlen + sizeof(struct ethhdr));//set tcp header

    int sessionId=0;
    for(;sessionId<MAX_SESSIONS;++sessionId)
    {
        if(sessionBuf[sessionId].sourceIP!=iph->saddr)
        {
            continue;
        }else if(sessionBuf[sessionId].destIP!=iph->daddr)
        {
            continue;
        }else if(sessionBuf[sessionId].sourcePort!=tcph->source)
        {
            continue;
        }else if(sessionBuf[sessionId].destPort!=tcph->dest)
        {
            continue;
        }else
        {
            return sessionId;
        }
    }
    return 0;
}

unsigned char* getTCPPayload(unsigned char* buffer, int size)
{

    struct iphdr *iph = (struct iphdr*)(buffer + sizeof(struct ethhdr));
    int iphdrlen = iph->ihl*4;//set ip header length
    struct tcphdr *tcph=(struct tcphdr*)(buffer  + sizeof(struct ethhdr)+ iphdrlen);//set tcp header
    int tcphdrlen = tcph->doff*4;//set TCP header length
    unsigned char* tcpPayload=(unsigned char*)(buffer  + sizeof(struct ethhdr)+ iphdrlen+tcphdrlen);

    return tcpPayload;
}

int getTCPPayloadSize(unsigned char* buffer, int size)
{
    struct iphdr *iph = (struct iphdr*)(buffer + sizeof(struct ethhdr));
    int iphdrlen = iph->ihl*4;//set ip header length
    struct tcphdr *tcph=(struct tcphdr*)(buffer  + sizeof(struct ethhdr)+ iphdrlen);//set tcp header
    int tcphdrlen = tcph->doff*4;//set TCP header length
    int applicationDataLength=size-iphdrlen-tcphdrlen-sizeof(struct ethhdr);
    return applicationDataLength;
}

void stringCopy(unsigned char* dest, unsigned char* src, int size)
{
    int i=0;
    for(;i<size;++i)
    {
        dest[i]=src[i];
    }
}

/*
 * insert elements to array
 */
void initArray(Array *a, size_t initialSize) {
    a->array = (char *)malloc(initialSize);
    a->used = 0;
    a->size = initialSize;
}

/*
 * insert elements to array
 */
void insertArray(Array *a, char element) {
    if (a->used == a->size) {
        a->size *= 2;
        a->array = (char *)realloc(a->array, a->size);
    }
    a->array[a->used++] = element;
}

/*
 * release array
 */
void freeArray(Array *a) {
    free(a->array);
    a->array = NULL;
    a->used = a->size = 0;
}

/*
 * make a duplicate copy of s
 */
static char *mystrdup(char *s) {

    char *p = (char *)malloc(strlen(s) + 1);

    if (p == NULL) {
        printf("Error: ran out of memory");
        exit(EXIT_FAILURE);
    }

    strcpy(p, s);
    return p;
}

/*
 * Calculates a hash value for a given string
 */
static unsigned hash(char *s) {

    unsigned hashval;

    for (hashval = 0; *s != '\0'; s++)
        hashval = *s + 31 * hashval;

    return hashval % HASHSIZE;
}

/*
 * Look for s in the hash table
 */
static struct nlist *lookup(char *s) {

    struct nlist *np;

    for (np = hashtab[hash(s)]; np != NULL; np = np->next)
        if (strcmp(s, np->name) == 0)
            return np;  // Found

    return NULL; // Not found
}

/*
 * Put a value into the hash table
 */
static struct nlist *insertData(char *name, char *defn) {

    struct nlist *np;

    if ((np = lookup(name)) == NULL) {

        unsigned hashval;

        if ((np = (struct nlist *)malloc(sizeof(*np))) == NULL) {
            printf("Error: ran out of memory");
            exit(EXIT_FAILURE);
        }
        hashval  = hash(name);
        np->name = mystrdup(name);
        np->next = hashtab[hashval];
        hashtab[hashval] = np;

    } else

        free((void *)np->defn);

    np->defn = mystrdup(defn);

    return np;
}

/*
 * remove a name and definition from the table
 */
static void undef(char *s) {

    unsigned h;
    struct nlist *prior;
    struct nlist *np;

    prior = NULL;
    h = hash(s);

    for (np = hashtab[h]; np != NULL; np = np->next) {
        if (strcmp(s, np->name) == 0)
            break;
        prior = np;
    }

    if (np != NULL) {
        if (prior == NULL)
            hashtab[h] = np->next;
        else
            prior->next = np->next;

        free((void *)np->name);
        free((void *)np->defn);
        free((void *)np);
    }
}

/**
 * this method chunks the given data stream
 */
int chunkData(unsigned char *buffer, int n) {

    unsigned char *copy;
    unsigned char *max = buffer, *end = buffer + n - 8;
    int i=0;
    for (copy = buffer +1; copy <= end; copy++) {
        int comp_res =comp(copy, max);
        if (comp_res < 0) {
            max = copy;
            continue;
        }
        if (copy == max + windowSize || copy == buffer + MAX_CHUNK_SIZE){ //chunk max size
            return copy - buffer;
        }
        i++;
    }
    return n;
}

/**
 * This method compares the given pointers
 */
int comp(unsigned char *i,unsigned char *max) {

    uint64_t a = __builtin_bswap64(*((uint64_t *) i));
    uint64_t b = __builtin_bswap64(*((uint64_t *) max));

    if (a > b) {
        return 1;
    }
    return -1;
}
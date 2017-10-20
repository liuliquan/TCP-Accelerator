/*......................prototypes.................................*/
#ifndef headerIncluded
#include "../DataDeduplication/headerFiles/headers.h"
#endif

#include "../DataDeduplication/headerFiles/writeInLogger.h"
#include "../DataDeduplication/headerFiles/deletedIdBST.h"


#ifndef constantsIncluded
#include "../DataDeduplication/headerFiles/constants.h"
#endif

#ifndef IdMinHeapIncluded
#include "../DataDeduplication/headerFiles/deletedIdMinHeap.h"
#endif

#ifndef structIncluded
#include """
#endif


#ifndef LandataQueueIncluded
#include "../DataDeduplication/headerFiles/LanDataQueue.h"
#endif

#ifndef LanorderQueueIncluded
#include "../DataDeduplication/headerFiles/LanOrderQueue.h"
#endif

/*functions*/
int isTcp(unsigned char* buffer, int size);
void sessionHandler(unsigned char* buffer,int size,struct iphdr *iph, struct tcphdr *tcph);
int getMinimumIdFromDeletedSessions();
int getId(unsigned char* buffer, int size);
unsigned char* getTCPPayload(unsigned char* buffer, int size);
int getTCPPayloadSize(unsigned char* buffer, int size);
void stringCopy(unsigned char* dest, unsigned char* src, int size);


/*threads*/
void* readerThread(void);
void* dedupThread(void);

/*.....................variable definitions.........................*/
FILE *logfile, *queueLogfile_1, *queueLogfile_2, *orderQueueLogfile, *testLogfile,*testLogfile2;
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
    //printHeap(&min_heap);
    deleteId(&min_heap);
    //printHeap(&min_heap);

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


/*..............................Thread definitions............................................*/
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

    /*queue initialization*/


    //log files
    logfile=fopen("log.txt","w");
    queueLogfile_1=fopen("queueLogfile_1.txt","w");
    queueLogfile_2=fopen("queueLogfile_2.txt","w");
    orderQueueLogfile=fopen("orderQueueLogfile.txt","w");
    testLogfile=fopen("testLogfile.txt","w");
    testLogfile2=fopen("testLogfile2","w");


    if(logfile==NULL){ printf("Unable to create log.txt file.");}

    printf("Starting...\n");

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

    int lim =0;
    int dataSize = 0;

    for(lim =0;lim<orderQueue->size;lim++){

        u_int32_t sessID = orderQueue->orderRecordArray[orderQueue->front+lim]->sessionId;
        u_int16_t loc = orderQueue->orderRecordArray[orderQueue->front+lim]->location;
        DATA_QUEUE* dQueue =  sessionBuf[sessID].queue;
        unsigned char *dedupload = dQueue->dataRecordArray[loc]->tcpPayload;
        dataSize += sizeof(dedupload);

        fprintf(testLogfile2,"sessionId : %d\n",sessID);
        //PrintData(dedupload,dQueue->dataRecordArray[loc]->size,testLogfile2);
        fprintf(testLogfile2,"\n");
    }

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
    printf("Inside the deduplication Thread");
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
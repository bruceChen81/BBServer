#ifndef MYQUEUE_H_INCLUDED
#define MYQUEUE_H_INCLUDED

#include <iostream>
#include <arpa/inet.h>
#include <sys/queue.h>
#include <pthread.h>

#include "list.h"


//client event queue
typedef enum clientEv
{
    EV_ACCEPT = 1,
    EV_RECV

}clientEv;

typedef struct _clientEvent
{
    clientEv event;
    int fd;
    clientType type;
    sockaddr_in clientAddr;
    STAILQ_ENTRY(_clientEvent) p;

}clientEvent;


int create_client_event_queue();

void enClientEventQueue(clientEvent *pClientEv);

clientEvent *deClientEventQueue();

bool isClientEventQueueEmpty();


//data sync event queue

typedef enum dataSyncEvType
{
    SYNC_EV_PRECOMMIT_ACK = 1,
    SYNC_EV_PRECOMMIT_ERR,
    SYNC_EV_COMMIT_SUCCESS,
    SYNC_EV_COMMIT_UNSUCCESS,
    SYNC_EV_TIMEOUT

}dataSyncEvType;

typedef struct _dataSyncEvent
{
    dataSyncEvType event;
    std::string msg;

    STAILQ_ENTRY(_dataSyncEvent) p;

}dataSyncEvent;

int create_data_sync_event_queue();

void enDataSyncEventQueue(dataSyncEvent *pDataSyncEv);

dataSyncEvent *deDataSyncEventQueue();

bool isDataSyncEventQueueEmpty();

#endif // MYQUEUE_H_INCLUDED

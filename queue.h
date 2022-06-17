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
    EV_RECV,
    EV_SYNC_PRECOMMIT_ACK,
    EV_SYNC_PRECOMMIT_ERR,
    EV_SYNC_COMMIT_SUCCESS,
    EV_SYNC_COMMIT_UNSUCCESS,
    EV_SYNC_TIMEOUT

}clientEv;

typedef struct _clientEvent
{
    int fd;
    clientEv event;
    clientType type;
    std::string msgNumber;
    std::string response;

    STAILQ_ENTRY(_clientEvent) p;

}clientEvent;


int create_client_event_queue();

void enClientEventQueue(clientEvent *pClientEv);

clientEvent *deClientEventQueue();

bool isClientEventQueueEmpty();


#endif // MYQUEUE_H_INCLUDED

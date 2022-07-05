#ifndef MYQUEUE_H_INCLUDED
#define MYQUEUE_H_INCLUDED


#include "common.h"


typedef struct _clientEventCB
{
    int fd;
    clientEvType event;
    clientType type;
    std::string msgNumber;
    std::string response;

    STAILQ_ENTRY(_clientEventCB) p;

}clientEventCB;


int create_client_event_queue();

void destroy_client_event_queue();

void enClientEventQueue(clientEventCB *pClientEv);

clientEventCB *deClientEventQueue();

bool isClientEventQueueEmpty();


#endif // MYQUEUE_H_INCLUDED

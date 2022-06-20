#ifndef MYQUEUE_H_INCLUDED
#define MYQUEUE_H_INCLUDED

#include <iostream>
#include <arpa/inet.h>
#include <sys/queue.h>
#include <pthread.h>

#include "list.h"


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

void destroy_client_event_queue();

void enClientEventQueue(clientEvent *pClientEv);

clientEvent *deClientEventQueue();

bool isClientEventQueueEmpty();


#endif // MYQUEUE_H_INCLUDED

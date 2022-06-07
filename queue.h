#ifndef MYQUEUE_H_INCLUDED
#define MYQUEUE_H_INCLUDED

#include <iostream>
#include <arpa/inet.h>
#include <sys/queue.h>
#include <pthread.h>

typedef enum clientEv
{
    EV_ACCEPT = 1,
    EV_RECV

}clientEv;

typedef struct _clientEvent
{
    clientEv event;
    int fd;
    sockaddr_in clientAddr;
    STAILQ_ENTRY(_clientEvent) p;

}clientEvent;


int create_client_event_queue();

void enClientEventQueue(clientEvent *pClientEv);

clientEvent *deClientEventQueue();

bool isClientEventQueueEmpty();



typedef enum msgSaveEv
{
    MSG_SAVE_WRITE = 1,
    MSG_SAVE_REPLACE,
    MSG_SAVE_REPLACE_COPY

}msgSaveEv;

typedef struct _msgSaveEvent
{
    msgSaveEv event;
    std::string msg;

    STAILQ_ENTRY(_msgSaveEvent) p;

}msgSaveEvent;

int create_msg_save_event_queue();

void enMsgSaveEventQueue(msgSaveEvent *pMsgSaveEv);

msgSaveEvent *deMsgSaveEventQueue();

bool isMsgSaveEventQueueEmpty();

#endif // MYQUEUE_H_INCLUDED

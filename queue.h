#ifndef MYQUEUE_H_INCLUDED
#define MYQUEUE_H_INCLUDED

#include <iostream>
#include <arpa/inet.h>
#include <sys/queue.h>
#include <pthread.h>


typedef struct _client
{
    int fd;
    sockaddr_in clientAddr;
    STAILQ_ENTRY(_client) p;

}client;


int create_client_queue();

void enClientQueue(client *pClient);

client *deClientQueue();

bool isClientQueueEmpty();

#endif // MYQUEUE_H_INCLUDED

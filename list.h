#ifndef LIST_H_INCLUDED
#define LIST_H_INCLUDED

#include <sys/queue.h>

typedef struct _clientInfo
{
    int fd;
    char name[64];
    char ip[32];
    unsigned int port;
    LIST_ENTRY(_clientInfo) p;

}clientInfo;

int test_list();

int create_client_list();

int client_list_add(clientInfo *pClientInfo);

int client_list_del(clientInfo *pClientInfo);

int client_list_clear();

#endif // LIST_H_INCLUDED

#ifndef LIST_H_INCLUDED
#define LIST_H_INCLUDED

#include <sys/queue.h>

#include "sync.h"


typedef enum clientType
{
    CLIENT_USER = 1,
    CLIENT_SYNC_MASTER,
    CLIENT_SYNC_SLAVE

}clientType;



typedef struct _clientInfo
{
    int fd;
    clientType type;
    syncState state;
    char name[64];
    //std::string name;
    char ip[32];
    //std::string ip;
    unsigned int port;
    unsigned int msgNo;

    LIST_ENTRY(_clientInfo) p;

}clientInfo;

int test_list();

int create_client_list();

int client_list_add(clientInfo *pClientInfo);

int client_list_del(clientInfo *pClientInfo);

int client_list_clear();

clientInfo *client_list_find(int fd);

clientInfo *client_list_get_first();

clientInfo *client_list_get_next(clientInfo *pClient);

int client_list_save_name(int fd, const char *str);

bool client_list_empty();

int sync_set_client_state(clientInfo *pClient, syncState state);







typedef struct _syncServerInfo
{
    int fd;
    std::string hostname;
    std::string ip;
    unsigned int port;
    syncState state;

    LIST_ENTRY(_syncServerInfo) p;

}syncServerInfo;



int create_sync_server_list();

int sync_server_list_add(syncServerInfo *psyncServerInfo);

int sync_server_list_del(syncServerInfo *psyncServerInfo);

int sync_server_list_clear();

bool sync_server_list_empty();

syncServerInfo *sync_server_list_get_first();

syncServerInfo *sync_server_list_get_next(syncServerInfo *pServer);

syncServerInfo *sync_server_list_find(int fd);

int sync_server_list_set_fd(syncServerInfo *pServer, int fd);

int sync_server_list_set_state(syncServerInfo *pServer, syncState state);


#endif // LIST_H_INCLUDED

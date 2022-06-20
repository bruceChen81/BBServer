#ifndef LIST_H_INCLUDED
#define LIST_H_INCLUDED

#include <sys/queue.h>

#include "sync.h"





typedef struct _clientInfo
{
    int fd;
    clientType type;
    clientCmdType cmd;
    syncState slaveState;
    char name[64];
    char ip[32];
    unsigned int port;
    bool slaveTimeout;
    std::string msgNumber;
    std::string msg;

    LIST_ENTRY(_clientInfo) p;

}clientInfo;

int test_list();

int create_client_list();

void destroy_client_list();

int client_list_add(clientInfo *pClientInfo);

int client_list_del(clientInfo *pClientInfo);

int client_list_clear();

clientInfo *client_list_find(int fd);

clientInfo *client_list_get_first();

clientInfo *client_list_get_next(clientInfo *pClient);

int client_list_save_name(int fd, const char *str);

bool client_list_empty();

int sync_set_slave_state(clientInfo *pClient, syncState state);

syncState sync_get_slave_state(clientInfo *pClient);

int sync_set_client_state(clientInfo *pClient, syncState state);

int sync_save_client_cmd(clientInfo *pClient, clientCmdType cmd, std::string& msg);

int sync_clear_client_cmd(clientInfo *pClient);

clientInfo *sync_find_waiting_commit_user_client();

clientInfo *sync_find_waiting_save_user_client();

clientInfo *sync_find_waiting_commit_slave_client();






typedef struct _syncServerInfo
{
    int fd;
    std::string hostname;
    std::string ip;
    unsigned int port;
    syncState state;
    syncState masterState;

    LIST_ENTRY(_syncServerInfo) p;

}syncServerInfo;



int create_sync_server_list();

void destroy_sync_server_list();


int sync_server_list_add(syncServerInfo *psyncServerInfo);

int sync_server_list_del(syncServerInfo *psyncServerInfo);

int sync_server_list_clear();

bool sync_server_list_empty();

syncServerInfo *sync_server_list_get_first();

syncServerInfo *sync_server_list_get_next(syncServerInfo *pServer);

syncServerInfo *sync_server_list_find(int fd);

int sync_server_list_set_fd(syncServerInfo *pServer, int fd);

int sync_server_list_set_state(syncServerInfo *pServer, syncState state);

int sync_set_master_state(syncState state);

syncState sync_get_master_state();

void print_sync_state(syncState state);


#endif // LIST_H_INCLUDED

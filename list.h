#ifndef LIST_H_INCLUDED
#define LIST_H_INCLUDED



#include "sync.h"





typedef struct _clientCB
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

    LIST_ENTRY(_clientCB) p;

}clientCB;

int test_list();

int create_client_list();

void destroy_client_list();

int client_list_add(clientCB *pClientInfo);

int client_list_del(clientCB *pClientInfo);

int client_list_clear();

clientCB *client_list_find(int fd);

clientCB *client_list_get_first();

clientCB *client_list_get_next(clientCB *pClient);

int client_list_save_name(int fd, const char *str);

bool client_list_empty();

int sync_set_slave_state(clientCB *pClient, syncState state);

syncState sync_get_slave_state(clientCB *pClient);

int sync_set_client_state(clientCB *pClient, syncState state);

int sync_save_client_cmd(clientCB *pClient, clientCmdType cmd, std::string& msg);

int sync_clear_client_cmd(clientCB *pClient);

clientCB *sync_find_waiting_commit_user_client();

clientCB *sync_find_waiting_save_user_client();

clientCB *sync_find_waiting_commit_slave_client();






typedef struct _syncServerCB
{
    int fd;
    std::string hostname;
    std::string ip;
    unsigned int port;
    syncState state;
    syncState masterState;

    LIST_ENTRY(_syncServerCB) p;

}syncServerCB;



int create_sync_server_list();

void destroy_sync_server_list();


int sync_server_list_add(syncServerCB *psyncServerInfo);

int sync_server_list_del(syncServerCB *psyncServerInfo);

int sync_server_list_clear();

bool sync_server_list_empty();

syncServerCB *sync_server_list_get_first();

syncServerCB *sync_server_list_get_next(syncServerCB *pServer);

syncServerCB *sync_server_list_find(int fd);

int sync_server_list_set_fd(syncServerCB *pServer, int fd);

int sync_server_list_set_state(syncServerCB *pServer, syncState state);

int sync_set_master_state(syncState state);

syncState sync_get_master_state();

void print_sync_state(syncState state);


#endif // LIST_H_INCLUDED

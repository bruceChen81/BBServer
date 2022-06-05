#ifndef LIST_H_INCLUDED
#define LIST_H_INCLUDED

#include <sys/queue.h>

typedef struct _clientInfo
{
    int fd;
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

int client_list_save_name(int fd, const char *str);

int client_get_msg_number();

#endif // LIST_H_INCLUDED

#include <iostream>
#include <unistd.h>
#include <string.h>

#include "list.h"
#include "config.h"

using std::cout;
using std::endl;

LIST_HEAD(clientInfoHead, _clientInfo) clientList
        = LIST_HEAD_INITIALIZER(clientList);

pthread_mutex_t clientListLock;


int test_list()
{
    clientInfo *pClient1, *pClient2, *np;

    pClient1 = new clientInfo;

    pClient1->fd = 1;

    pClient2 = new clientInfo;

    pClient2->fd = 2;

    client_list_add(pClient1);

    client_list_add(pClient2);

    LIST_FOREACH(np, &clientList, p)
    {
        cout <<"client list: fd = " << np->fd << endl;
    }

    //client_list_del(pClient1);
    client_list_clear();

    if(LIST_EMPTY(&clientList))
    {
        cout << "client list is empty" <<endl;
    }

/*
//


LIST_INIT(&head);

clientInfo *n1, *n2, *n3, *np, *np_temp;

n1 = new clientInfo;
n2 = new clientInfo;
n3 = new clientInfo;

n1->fd =1;
n2->fd =2;
n3->fd =3;

LIST_INSERT_HEAD(&head, n1, p);

LIST_INSERT_HEAD(&head, n2, p);

LIST_INSERT_HEAD(&head, n3, p);

LIST_FOREACH(np, &head, p)
{
    cout << np->fd << endl;
}

LIST_REMOVE(n1, p);
delete n1;

LIST_FOREACH(np, &head, p)
//LIST_FOREACH_SAFE(np, &head, p, np_temp)
{
    if(np->fd == 2)
    {
        LIST_REMOVE(np, p);
        delete np;
    }
}

LIST_FOREACH(np, &head, p)
{
    cout <<"delete n2: " << np->fd << endl;
}

while(!LIST_EMPTY(&head))
{
    n1 = LIST_FIRST(&head);
    LIST_REMOVE(n1, p);
    delete n1;
}

*/
    return 0;

}

int create_client_list()
{
    if (pthread_mutex_init(&clientListLock, NULL) != 0)
    {
        std::cout << "init clientListLock failed!" << std::endl;
    }

    LIST_INIT(&clientList);

    return 0;
}

int client_list_add(clientInfo *pClientInfo)
{
    if(!pClientInfo)
    {
        return -1;
    }

    pthread_mutex_lock(&clientListLock);

    LIST_INSERT_HEAD(&clientList, pClientInfo, p);

    pthread_mutex_unlock(&clientListLock);

    if(CONFIG.debugLevel >= DEBUG_LEVEL_APP)
        cout << "client_list_add, fd:" << pClientInfo->fd << " IP:" << pClientInfo->ip << " Port:" << pClientInfo->port <<endl;

    return 0;

}

int client_list_del(clientInfo *pClientInfo)
{
    if(!pClientInfo)
    {
        return -1;
    }

    pthread_mutex_lock(&clientListLock);

    LIST_REMOVE(pClientInfo, p);

    pthread_mutex_unlock(&clientListLock);

    if(CONFIG.debugLevel >= DEBUG_LEVEL_APP)
        cout << "client_list_del, fd:" << pClientInfo->fd << " IP:" << pClientInfo->ip << " Port:" << pClientInfo->port <<endl;

    delete pClientInfo;

    return 0;

}

int client_list_clear()
{
    clientInfo *np;

    pthread_mutex_lock(&clientListLock);

    while(!LIST_EMPTY(&clientList))
    {
        np = LIST_FIRST(&clientList);
        LIST_REMOVE(np, p);
        delete np;
    }

    pthread_mutex_unlock(&clientListLock);

    return 0;
}

clientInfo *client_list_find(int fd)
{
    clientInfo *np;

    LIST_FOREACH(np, &clientList, p)
    {
        if(np->fd == fd)
        {
            if(CONFIG.debugLevel >= DEBUG_LEVEL_APP)
                cout << "client_list_find, fd:" << np->fd << " IP:" << np->ip << " Port:" << np->port <<" name:"<<np->name<<endl;

            return np;
        }
    }

    return nullptr;
}

int client_list_save_name(int fd, const char *str)
{
    clientInfo *np;
    int ret = -1;

    pthread_mutex_lock(&clientListLock);

    LIST_FOREACH(np, &clientList, p)
    {
        if(np->fd == fd)
        {
            strcpy(np->name, str);
            ret = 0;
            break;
        }
    }
    pthread_mutex_unlock(&clientListLock);

    if(CONFIG.debugLevel >= DEBUG_LEVEL_APP)
                cout << "client_list_save_name, fd:" << fd <<" name:"<<str<<endl;

    return ret;

}












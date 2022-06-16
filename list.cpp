#include <iostream>
#include <unistd.h>
#include <string.h>

#include "list.h"
#include "config.h"

using std::cout;
using std::endl;

std::string syncStateArray[SYNC_MAX] = {" ",
                                  "DISCONNECT",
                                  "IDLE",
                                  "M_PRECOMMIT_MULTICASTED",
                                  "M_PRECOMMITED",
                                  "M_PRECOMMIT_UNSUCCESS",
                                  "M_COMMITED",
                                  "M_OPERATION_PERFORMED",
                                  "M_OPERATION_UNSUCCESS",
                                  "S_PRECOMMIT_RECEIVED",
                                  "S_PRECOMMIT_ACK",
                                  "S_COMMITED",
                                  "S_UNDO"};


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
        cout << "client_list_add, fd:" << pClientInfo->fd << " IP:" << pClientInfo->ip << " Port:" << pClientInfo->port <<" Type:"<<pClientInfo->type<<endl;

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
        cout << "client_list_del, fd:" << pClientInfo->fd << " IP:" << pClientInfo->ip << " Port:" << pClientInfo->port <<" Type:"<<pClientInfo->type<<endl;

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
                cout << "client_list_find, fd:" << np->fd << " IP:" << np->ip << " Port:" << np->port <<" Type:"<<np->type<<" name:"<<np->name<<endl;

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

bool client_list_empty()
{
    return LIST_EMPTY(&clientList);
}

clientInfo *client_list_get_first()
{
    return LIST_FIRST(&clientList);

}

clientInfo *client_list_get_next(clientInfo *pClient)
{
    return LIST_NEXT(pClient,p);
}

int sync_set_slave_state(clientInfo *pClient, syncState state)
{
    if(!pClient)
    {
        return -1;
    }

    pthread_mutex_lock(&clientListLock);

    pClient->slaveState = state;

    pthread_mutex_unlock(&clientListLock);

    if(CONFIG.debugLevel >= DEBUG_LEVEL_D)
                cout << "set sync slave state [" <<syncStateArray[state] <<"]" << " Master:" << pClient->ip << ":" << pClient->port <<endl;

    return 1;
}








LIST_HEAD(syncServerInfoHead, _syncServerInfo) syncServerList
        = LIST_HEAD_INITIALIZER(syncServerList);


pthread_mutex_t syncServerListLock;

int create_sync_server_list()
{
    if (pthread_mutex_init(&syncServerListLock, NULL) != 0)
    {
        std::cout << "init syncServerListLock failed!" << std::endl;
    }

    LIST_INIT(&syncServerList);

    return 0;
}

int sync_server_list_add(syncServerInfo *psyncServerInfo)
{
    if(!psyncServerInfo)
    {
        return -1;
    }

    pthread_mutex_lock(&syncServerListLock);

    syncServerInfo *np = LIST_FIRST(&syncServerList);

    if(np == nullptr)
    {
        LIST_INSERT_HEAD(&syncServerList, psyncServerInfo, p);
    }
    else
    {
        while(LIST_NEXT(np, p) != nullptr)
        {
            np = LIST_NEXT(np, p);
        }

        //insert the last
        LIST_INSERT_AFTER(np, psyncServerInfo, p);
    }

    pthread_mutex_unlock(&syncServerListLock);


    if(CONFIG.debugLevel >= DEBUG_LEVEL_APP)
        cout << "sync_server_list_add, fd:" << psyncServerInfo->fd << " IP:" << psyncServerInfo->ip << " Port:" << psyncServerInfo->port <<endl;

    return 0;

}

int sync_server_list_del(syncServerInfo *psyncServerInfo)
{
    if(!psyncServerInfo)
    {
        return -1;
    }

    pthread_mutex_lock(&syncServerListLock);

    LIST_REMOVE(psyncServerInfo, p);

    pthread_mutex_unlock(&syncServerListLock);

    if(CONFIG.debugLevel >= DEBUG_LEVEL_APP)
        cout << "sync_server_list_del, fd:" << psyncServerInfo->fd << " IP:" << psyncServerInfo->ip << " Port:" << psyncServerInfo->port <<endl;

    delete psyncServerInfo;

    return 0;

}

int sync_server_list_clear()
{
    syncServerInfo *np;

    pthread_mutex_lock(&syncServerListLock);

    while(!LIST_EMPTY(&syncServerList))
    {
        np = LIST_FIRST(&syncServerList);
        LIST_REMOVE(np, p);
        delete np;
    }

    pthread_mutex_unlock(&syncServerListLock);

    return 0;
}

syncServerInfo *sync_server_list_find(int fd)
{
    syncServerInfo *np;

    LIST_FOREACH(np, &syncServerList, p)
    {
        if(np->fd == fd)
        {
            if(CONFIG.debugLevel >= DEBUG_LEVEL_APP)
                cout << "sync_server_list_find, fd:" << np->fd << " IP:" << np->ip << " Port:" << np->port <<" name:"<<np->hostname<<endl;

            return np;
        }
    }

    return nullptr;
}

bool sync_server_list_empty()
{
    return LIST_EMPTY(&syncServerList);
}

syncServerInfo *sync_server_list_get_first()
{
    return LIST_FIRST(&syncServerList);

}

syncServerInfo *sync_server_list_get_next(syncServerInfo *pServer)
{
    return LIST_NEXT(pServer,p);
}

int sync_server_list_set_state(syncServerInfo *pServer, syncState state)
{
    if(!pServer)
    {
        return -1;
    }

    pthread_mutex_lock(&syncServerListLock);

    pServer->state = state;

    pthread_mutex_unlock(&syncServerListLock);

    if(CONFIG.debugLevel >= DEBUG_LEVEL_APP)
                cout << "set sync server list master state [" <<syncStateArray[state] <<"]" << " Peer:" << pServer->ip << ":" << pServer->port <<endl;

    return 1;
}

int sync_server_list_set_fd(syncServerInfo *pServer, int fd)
{
    if(!pServer)
    {
        return -1;
    }

    pthread_mutex_lock(&syncServerListLock);

    pServer->fd = fd;

    pthread_mutex_unlock(&syncServerListLock);

    if(CONFIG.debugLevel >= DEBUG_LEVEL_APP)
                cout << "sync_server_set_fd [" <<fd<<"] IP:" << pServer->ip << " Port:" << pServer->port <<endl;

    return 1;
}

int sync_set_master_state(syncState state)
{
    syncServerInfo *pServer;

    pthread_mutex_lock(&syncServerListLock);

    pServer = LIST_FIRST(&syncServerList);

    if(pServer)
    {
        pServer->masterState = state;
    }

    pthread_mutex_unlock(&syncServerListLock);

    if(CONFIG.debugLevel >= DEBUG_LEVEL_D)
                cout << "set sync master state [" <<syncStateArray[state] <<"]" <<endl;

    return 1;
}










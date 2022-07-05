/*
list.cpp

Created by Dianyong Chen, 2022-05-26

CS590 Master Project(BBServer) @ Bishop's University

*/


#include "common.h"

#include "list.h"
#include "config.h"



std::string syncStateArray[SYNC_MAX] = {"NULL",
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
                                          "S_UNDO",

                                          "U_WAITING_COMMIT",
                                          "U_WAITING_SAVE",
                                          "U_SAVING",
                                          "U_SAVED"};


LIST_HEAD(clientCBHead, _clientCB) clientList
        = LIST_HEAD_INITIALIZER(clientList);

pthread_mutex_t clientListLock;


int test_list()
{
    clientCB *pClient1, *pClient2, *np;

    pClient1 = new clientCB;

    pClient1->fd = 1;

    pClient2 = new clientCB;

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

clientCB *n1, *n2, *n3, *np, *np_temp;

n1 = new clientCB;
n2 = new clientCB;
n3 = new clientCB;

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

int client_list_add(clientCB *pClient)
{
    if(!pClient)
    {
        return -1;
    }

    pthread_mutex_lock(&clientListLock);

    LIST_INSERT_HEAD(&clientList, pClient, p);

    pthread_mutex_unlock(&clientListLock);

    if(CONFIG.debugLevel >= DEBUG_LEVEL_APP)
        cout << "client_list_add, fd:" << pClient->fd << " IP:" << pClient->ip << " Port:" << pClient->port <<" Type:"<<pClient->type<<endl;

    return 0;

}

int client_list_del(clientCB *pClient)
{
    if(!pClient)
    {
        return -1;
    }

    pthread_mutex_lock(&clientListLock);

    LIST_REMOVE(pClient, p);

    pthread_mutex_unlock(&clientListLock);

    if(CONFIG.debugLevel >= DEBUG_LEVEL_APP)
        cout << "client_list_del, fd:" << pClient->fd << " IP:" << pClient->ip << " Port:" << pClient->port <<" Type:"<<pClient->type<<endl;

    delete pClient;

    return 0;

}

int client_list_clear()
{
    clientCB *np;

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

void destroy_client_list()
{
    client_list_clear();

    pthread_mutex_destroy(&clientListLock);

    if (CONFIG.debugLevel >= DEBUG_LEVEL_D)
        cout << "Client list deleted!" << endl;

    return;
}

clientCB *client_list_find(int fd)
{
    clientCB *np;

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
    clientCB *np;
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

clientCB *client_list_get_first()
{
    return LIST_FIRST(&clientList);

}

clientCB *client_list_get_next(clientCB *pClient)
{
    return LIST_NEXT(pClient,p);
}

/*
    //slave
    SYNC_S_PRECOMMIT_RECEIVED, //7 received precommit
    SYNC_S_PRECOMMIT_ACK,      //8 sent positive ack
    SYNC_S_COMMITED,           //9 received commit, performed operation, send successful
    SYNC_S_UNDO,               //10 operation unsuccessful, undo
*/
int sync_set_slave_state(clientCB *pClient, syncState state)
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

    switch(state)
    {
        case SYNC_S_PRECOMMIT_ACK:
            start_timer_slave(SYNC_STATE_TIMEOUT);
            break;

        case SYNC_S_COMMITED:
            stop_timer_slave();
            break;

        default:
            break;
    }

    return 0;
}

syncState sync_get_slave_state(clientCB *pClient)
{
    syncState state = SYNC_MAX;

    if(!pClient)
    {
        return SYNC_MAX;
    }

    pthread_mutex_lock(&clientListLock);

    state = pClient->slaveState;

    pthread_mutex_unlock(&clientListLock);


    return state;
}


/*
    //user client
    SYNC_U_WAITING_COMMIT,
    SYNC_U_WAITING_SAVE,
*/
int sync_set_client_state(clientCB *pClient, syncState state)
{
    if(!pClient)
    {
        return -1;
    }

    pthread_mutex_lock(&clientListLock);

    pClient->slaveState = state;

    pthread_mutex_unlock(&clientListLock);

    if(CONFIG.debugLevel >= DEBUG_LEVEL_D)
                cout << "set user client state [" <<syncStateArray[state] <<"]" <<endl;

    return 1;
}


int sync_save_client_cmd(clientCB *pClient, clientCmdType cmd, std::string& msg)
{
    if(!pClient)
    {
        return -1;
    }

    pthread_mutex_lock(&clientListLock);

    if(!pClient->msg.empty())
    {
        pClient->msg.clear();
    }

    if(!pClient->msgNumber.empty())
    {
        pClient->msgNumber.clear();
    }

    pClient->cmd = cmd;
    pClient->msg += msg;
    pClient->msgNumber += msg.substr(0, msg.find("/"));
    pClient->slaveState = SYNC_U_WAITING_COMMIT;

    pthread_mutex_unlock(&clientListLock);

    if(CONFIG.debugLevel >= DEBUG_LEVEL_APP)
                cout << "save sync client cmd:" <<cmd<<" msg:" << msg <<endl;

    return 1;
}

int sync_clear_client_cmd(clientCB *pClient)
{
    if(!pClient)
    {
        return -1;
    }

    pthread_mutex_lock(&clientListLock);

    //pClient->cmd.clear();
    pClient->msg.clear();
    pClient->msgNumber.clear();
    pClient->slaveState = SYNC_IDLE;
    //pClient->waitingSync = false;

    pthread_mutex_unlock(&clientListLock);

    if(CONFIG.debugLevel >= DEBUG_LEVEL_APP)
                cout << "clear sync client cmd" <<endl;

    return 1;
}

clientCB * sync_find_waiting_commit_user_client()
{
    clientCB *np, *ret = nullptr;


    pthread_mutex_lock(&clientListLock);

    LIST_FOREACH(np, &clientList, p)
    {
        if((np->type == CLIENT_USER) && (np->slaveState == SYNC_U_WAITING_COMMIT))
        {
            ret = np;
            break;
        }
    }
    pthread_mutex_unlock(&clientListLock);

    return ret;
}

clientCB * sync_find_waiting_save_user_client()
{
    clientCB *np, *ret = nullptr;


    pthread_mutex_lock(&clientListLock);

    LIST_FOREACH(np, &clientList, p)
    {
        if((np->type == CLIENT_USER) && (np->slaveState == SYNC_U_WAITING_SAVE))
        {
            ret = np;
            break;
        }
    }
    pthread_mutex_unlock(&clientListLock);

    return ret;
}

clientCB * sync_find_waiting_commit_slave_client()
{
    clientCB *np, *ret = nullptr;

    pthread_mutex_lock(&clientListLock);

    LIST_FOREACH(np, &clientList, p)
    {
        if((np->type == CLIENT_SYNC_SLAVE) && (np->slaveState == SYNC_S_PRECOMMIT_ACK))
        {
            ret = np;
            break;
        }
    }
    pthread_mutex_unlock(&clientListLock);

    return ret;
}







LIST_HEAD(syncServerCBHead, _syncServerCB) syncServerList
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

int sync_server_list_add(syncServerCB *psyncServer)
{
    if(!psyncServer)
    {
        return -1;
    }

    pthread_mutex_lock(&syncServerListLock);

    syncServerCB *np = LIST_FIRST(&syncServerList);

    if(np == nullptr)
    {
        LIST_INSERT_HEAD(&syncServerList, psyncServer, p);
    }
    else
    {
        while(LIST_NEXT(np, p) != nullptr)
        {
            np = LIST_NEXT(np, p);
        }

        //insert the last
        LIST_INSERT_AFTER(np, psyncServer, p);
    }

    pthread_mutex_unlock(&syncServerListLock);


    if(CONFIG.debugLevel >= DEBUG_LEVEL_APP)
        cout << "sync_server_list_add, fd:" << psyncServer->fd << " IP:" << psyncServer->ip << " Port:" << psyncServer->port <<endl;

    return 0;

}

int sync_server_list_del(syncServerCB *psyncServer)
{
    if(!psyncServer)
    {
        return -1;
    }

    pthread_mutex_lock(&syncServerListLock);

    LIST_REMOVE(psyncServer, p);

    pthread_mutex_unlock(&syncServerListLock);

    if(CONFIG.debugLevel >= DEBUG_LEVEL_APP)
        cout << "sync_server_list_del, fd:" << psyncServer->fd << " IP:" << psyncServer->ip << " Port:" << psyncServer->port <<endl;

    delete psyncServer;

    return 0;

}

int sync_server_list_clear()
{
    syncServerCB *np;

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

void destroy_sync_server_list()
{
    sync_server_list_clear();

    pthread_mutex_destroy(&syncServerListLock);

    if (CONFIG.debugLevel >= DEBUG_LEVEL_D)
            cout << "Sync server list deleted!" << endl;

    return;
}


syncServerCB *sync_server_list_find(int fd)
{
    syncServerCB *np;

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

syncServerCB *sync_server_list_get_first()
{
    return LIST_FIRST(&syncServerList);

}

syncServerCB *sync_server_list_get_next(syncServerCB *pServer)
{
    return LIST_NEXT(pServer,p);
}

int sync_server_list_set_state(syncServerCB *pServer, syncState state)
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

int sync_server_list_set_fd(syncServerCB *pServer, int fd)
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

/*
    //master
    SYNC_M_PRECOMMIT_MULTICASTED, //3 write request from a client, broadcasted precommit
    SYNC_M_PRECOMMITED,           //4 positive acked by everybody
    SYNC_M_PRECOMMIT_UNSUCCESS,
    SYNC_M_COMMITED,              //5 broadcasted commit and data
    SYNC_M_OPERATION_PERFORMED,   //6 positive acked by everybody, performed operation
    SYNC_M_OPERATION_UNSUCCESS,
*/
int sync_set_master_state(syncState state)
{
    syncServerCB *pServer;

    pthread_mutex_lock(&syncServerListLock);

    pServer = LIST_FIRST(&syncServerList);

    if(pServer)
    {
        pServer->masterState = state;
    }

    pthread_mutex_unlock(&syncServerListLock);

    if(CONFIG.debugLevel >= DEBUG_LEVEL_D)
                cout << "set sync master state [" <<syncStateArray[state] <<"]" <<endl;

    //timer
    switch(state)
    {
        case SYNC_M_PRECOMMIT_MULTICASTED:
        case SYNC_M_COMMITED:
            start_timer_master(SYNC_STATE_TIMEOUT);
            break;

        case SYNC_M_PRECOMMITED:
        case SYNC_M_PRECOMMIT_UNSUCCESS:
        case SYNC_M_OPERATION_PERFORMED:
        case SYNC_M_OPERATION_UNSUCCESS:
            stop_timer_master();
            break;

        default:
            break;
    }


    return 0;
}


syncState sync_get_master_state()
{
    syncServerCB *pServer;

    syncState state = SYNC_MAX;

    pthread_mutex_lock(&syncServerListLock);

    pServer = LIST_FIRST(&syncServerList);

    if(pServer)
    {
        state = pServer->masterState;
    }

    pthread_mutex_unlock(&syncServerListLock);

    return state;
}

void print_sync_state(syncState state)
{
    if(state < SYNC_MAX)
    {
        cout << "[" << syncStateArray[state] <<"]" <<endl;
    }

    return;
}









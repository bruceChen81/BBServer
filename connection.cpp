#include <iostream>
#include <sys/select.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>

#include "common.h"
#include "config.h"
#include "queue.h"
#include "connection.h"
#include "list.h"

using std::cout;
using std::endl;


int connections[MAX_CONN];

pthread_mutex_t clientConnLock;


int create_tcp_server_sock()
{
    int fd = 0;

    fd = socket(AF_INET, SOCK_STREAM, 0);

    if(fd < 0)
    {
        cout << "Can not create a socket!" << endl;
        return ERR;
    }

    //bind
    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(CONFIG.bbPort);
    hint.sin_addr.s_addr = INADDR_ANY;
    //inet_pton(AF_INET, "0.0.0.0", &hint.sin_addr);

    if (bind(fd,(sockaddr *)&hint, sizeof(hint)) < 0)
    {
        cout << "Error binding!" << endl;
        return ERR;
    }

    //listen
    cout << "Waiting for client!" << endl;

    //listen(listeningSock, SOMAXCONN);
    listen(fd, 5);

    return fd;
}

int start_conn_service()
{
    int server_fd, new_fd, ret;

    sockaddr_in clientAddr;
    socklen_t clientAddrSize = sizeof(clientAddr);

    fd_set read_fd_set;

    conn_init();

    //create server listening socket
    server_fd = create_tcp_server_sock();

    if (server_fd < 0)
    {
        return ERR;
    }

    conn_add(server_fd);

    while(true)
    {
        FD_ZERO(&read_fd_set);

        conn_set_fdset(&read_fd_set);

        //Invoke select()
        ret = select(FD_SETSIZE, &read_fd_set, NULL, NULL, NULL);

        if (ret < 0)
        {
            cout << "select return error!"<<endl;
            continue;
        }

        //check if the fd with event is the sever fd, accept new connection
        if(FD_ISSET(server_fd, &read_fd_set))
        {
            //en client event queue, to accept
            clientEvent *pClientEv = new clientEvent;

            pClientEv->event = EV_ACCEPT;
            pClientEv->fd = server_fd;
            //pClientEv->clientAddr = *pClientAddr;

            enClientEventQueue(pClientEv);
        }
        else
        {
            //check if the fd with event is a non-server fd, then reveive data
            conn_check_fd_set(&read_fd_set, &clientAddr);
        }

    }//while()

    // close all sockets
    //conn_close_all();

    return SUCCESS;
}

void conn_check_fd_set(fd_set *pFdSet, sockaddr_in *pClientAddr)
{
    unsigned int i;

    if(!pFdSet || !pClientAddr)
    {
        return;
    }

    // do not check server fd:connections[0]
    for(i=1;i<MAX_CONN;i++)
    {
        if((connections[i] > 0) && FD_ISSET(connections[i], pFdSet))
        {
            clientEvent *pClientEv = new clientEvent;

            pClientEv->event = EV_RECV;
            pClientEv->fd = connections[i];
            pClientEv->clientAddr = *pClientAddr;

            enClientEventQueue(pClientEv);

            //std::cout << "enQueue recv Client fd = "<< connections[i] << std::endl;
        }
    }

    return;
}


void conn_init()
{
    unsigned int i;

    if (pthread_mutex_init(&clientConnLock, NULL) != 0)
    {
        std::cout << "init clientConnLock failed!" << std::endl;
    }

    pthread_mutex_lock(&clientConnLock);

    for (i=0;i<MAX_CONN;i++)
    {
        connections[i] = -1;
    }

    pthread_mutex_unlock(&clientConnLock);

    return;
}

void conn_set_fdset(fd_set *pFdSet)
{
    unsigned int i;

    if(!pFdSet)
    {
        return;
    }

    for(i=0;i<MAX_CONN;i++)
    {
        if(connections[i] >= 0)
        {
            FD_SET(connections[i], pFdSet);
        }
    }

    return;
}



void conn_add(int fd)
{
    unsigned int i;

    bool isExist = false;

    for (i=0;i<MAX_CONN;i++)
    {
        if(connections[i] == fd)
        {
            isExist = true;
            return;
        }
    }

    pthread_mutex_lock(&clientConnLock);

    if(isExist == false)
    {
        for (i=0;i<MAX_CONN;i++)
        {
            if(connections[i] < 0)
            {
                connections[i] = fd;
                break;
            }
        }
    }

    pthread_mutex_unlock(&clientConnLock);

    return;
}

void conn_del(int fd)
{
    unsigned int i;

    if(fd < 0)
    {
        return;
    }

    pthread_mutex_lock(&clientConnLock);

    for (i=0;i<MAX_CONN;i++)
    {
        if(connections[i] == fd)
        {
            connections[i] = -1;
            close(fd);

            break;
        }
    }

    pthread_mutex_unlock(&clientConnLock);

    return;
}

void conn_close_all()
{
    unsigned int i;

    pthread_mutex_lock(&clientConnLock);

    for (i=0;i<MAX_CONN;i++)
    {
        if(connections[i] > 0)
        {
            close(connections[i]);
            connections[i] = -1;
        }
    }

    pthread_mutex_unlock(&clientConnLock);

    return;
}

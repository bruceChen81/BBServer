#ifndef CONNECTION_H_INCLUDED
#define CONNECTION_H_INCLUDED

#include "common.h"
#include "myqueue.h"

#define MAX_CONN 20

int connections[MAX_CONN];

void conn_init()
{
    unsigned int i;

    for (i=0;i<MAX_CONN;i++)
    {
        connections[i] = -1;
    }

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

void conn_check_fd_set(fd_set *pFdSet, sockaddr_in *pClientAddr)
{
    unsigned int i;

    if(!pFdSet || !pClientAddr)
    {
        return;
    }

    for(i=0;i<MAX_CONN;i++)
    {
        if((connections[i] >= 0) && FD_ISSET(connections[i], pFdSet))
        {
            client *pClient = new client;

            pClient->fd = connections[i];
            pClient->clientAddr = *pClientAddr;

            enClientQueue(pClient);

            std::cout << "enQueue Client fd = "<< pClient->fd << std::endl;
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

    return;
}

void conn_del(int fd)
{
    unsigned int i;

    if(fd < 0)
    {
        return;
    }

    for (i=0;i<MAX_CONN;i++)
    {
        if(connections[i] == fd)
        {
            connections[i] = -1;
            close(fd);

            break;
        }
    }

    return;
}

void conn_close_all()
{
    unsigned int i;

    for (i=0;i<MAX_CONN;i++)
    {
        if(connections[i] > 0)
        {
            close(connections[i]);
            connections[i] = -1;
        }
    }

    return;
}



#endif // CONNECTION_H_INCLUDED

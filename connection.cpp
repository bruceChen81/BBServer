#include <iostream>
#include <sys/select.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "common.h"
#include "config.h"
#include "queue.h"
#include "connection.h"

using namespace std;


int connections[MAX_CONN];


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

int start_comm_service()
{
    int server_fd, new_fd, ret;

    sockaddr_in clientAddr;
    socklen_t clientAddrSize = sizeof(clientAddr);

    fd_set read_fd_set;

    conn_init();

    //create server listening socket
    server_fd = create_tcp_server_sock();

    if (ERR == server_fd)
    {
        return ERR;
    }

    conn_add(server_fd);

    while(true)
    {
        FD_ZERO(&read_fd_set);

        conn_set_fdset(&read_fd_set);

        //Invoke select()
        //cout << "Invoke select to listen for incoming events" << endl;

        ret = select(FD_SETSIZE, &read_fd_set, NULL, NULL, NULL);

        //cout << "Select returned with " << ret << endl;

        if (ret < 0)
        {
            cout << "select return error!"<<endl;
            continue;
        }


        //check if the fd with event is the sever fd, accept new connection
        if(FD_ISSET(server_fd, &read_fd_set))
        {
            new_fd = accept(server_fd, (struct sockaddr *)&clientAddr, &clientAddrSize);

            if(new_fd >= 0)
            {
                cout << "Client: " << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port) << " connected!" << endl;
                //add new fd to fd set
                conn_add(new_fd);
            }
            else
            {
                cout << "Error on accepting!" << endl;
            }
        }
        else
        {
            //check if the fd with event is a non-server fd, reveive data
            conn_check_fd_set(&read_fd_set, &clientAddr);
        }

    }//while()

    // close all sockets
    conn_close_all();

    return SUCCESS;
}




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

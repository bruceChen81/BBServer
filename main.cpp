//main.cpp
//
//Created by Dianyong Chen, 2022-05-24
//
//


#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <string>
#include <pthread.h>
#include <sys/queue.h>
#include <fcntl.h>
#include <stdlib.h>

#include "common.h"
#include "config.h"
#include "queue.h"
#include "connection.h"
#include "list.h"

using std::cout;
using std::endl;
using std::string;


int main(int argc, char *argv[])
{
    load_config((char *)DEFAULT_CFG_FILE);

    load_option(argc, argv);

    //check if bbfile is set
    if(!strlen(CONFIG.bbFile))
    {
        cout << "BBFILE name is not configured, please set by config file or command line by -b !!!"<< endl;
        exit(-1);
    }

    create_client_list();

    create_client_event_queue();

    create_thread_pool();

    start_conn_service();


    return 0;
}

void *handle_client_event(void *arg)
{
    char *uargv = nullptr;

    int fd, new_fd, bytesRecved = -1;

    sockaddr_in clientAddr;
    socklen_t clientAddrSize = sizeof(clientAddr);

    clientEvent *pClientEv = nullptr;

    char buf[1024];

    memset(buf, 0, sizeof(buf));

    while(true)
    {
        pClientEv = deClientEventQueue();

        if(nullptr == pClientEv)
        {
            continue;
        }

        //cout << "deQueue Client fd = " << pClient->fd << endl;

        fd = pClientEv->fd;

        if(pClientEv->event == EV_ACCEPT)
        {
            new_fd = accept(fd, (struct sockaddr *)&clientAddr, &clientAddrSize);

            if(new_fd >= 0)
            {
                cout << "Client: " << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port) << " connected!" << endl;
                //add new fd to fd set
                conn_add(new_fd);

                //add new client to client list
                {
                    clientInfo *pClient = new clientInfo;

                    memset((void *)pClient, 0, sizeof(clientInfo));

                    pClient->fd = new_fd;

                    strcpy(pClient->ip, inet_ntoa(clientAddr.sin_addr));

                    pClient->port = ntohs(clientAddr.sin_port);

                    client_list_add(pClient);

                    cout << "add client list, fd:" << pClient->fd << " IP:" << pClient->ip << " Port:" << pClient->port <<endl;
                }

                //send greeting
                send(new_fd, MSG_GREETING,strlen(MSG_GREETING), 0);


            }
            else
            {
                cout << "Error on accepting!" << endl;
            }
        }
        else if(pClientEv->event == EV_RECV)
        {
            bytesRecved = recv(fd, buf, sizeof(buf), 0);

            if (bytesRecved == -1)
            {
                cout << "Error in recv()" << endl;
                //continue;
            }
            else if (bytesRecved == 0)
            {
                cout << "Client:" << inet_ntoa(pClientEv->clientAddr.sin_addr) << " disconnected!" << endl;

                conn_del(fd);

                //continue;
            }
            else if (bytesRecved > 0)
            {
                cout << "Msg recved from " << inet_ntoa(pClientEv->clientAddr.sin_addr) <<":" << ntohs(pClientEv->clientAddr.sin_port)
                    <<" [" << bytesRecved << " Bytes]: " << string(buf, 0, bytesRecved)<< endl;

                //echo
                send(fd, buf, bytesRecved+1, 0);
            }

        }

        delete pClientEv;
    }

    return uargv;
}

int create_thread_pool()
{
    pthread_t threadPool[CONFIG.thMax];

    for(unsigned int i=0;i<CONFIG.thMax;i++)
    {
        pthread_create(&threadPool[i], NULL, handle_client_event, NULL);
    }

    cout << "client event thread pool created!" << endl;

    return SUCCESS;
}




//main.cpp
//
//Created  2022-05-24
//
//


#include <iostream>
#include <fstream>
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
#include <time.h>
#include <signal.h>
#include <climits>
#include <netdb.h>

#include "common.h"
#include "config.h"
#include "queue.h"
#include "connection.h"
#include "list.h"
#include "msg.h"
#include "semaphore.h"
#include "sync.h"

using std::cout;
using std::endl;
using std::string;
using std::fstream;
//using std::string::npos;
using std::cin;




int main(int argc, char *argv[])
{
    if (load_config((char *)DEFAULT_CFG_FILE) >= 0)
    {
        if(CONFIG.debugLevel >= DEBUG_LEVEL_D)
                cout << "Load config file success!" << endl;
    }

    load_option(argc, argv);

    //check if bbfile is set
    if(!strlen(CONFIG.bbFile))
    {
        cout << "BBFILE name is not configured, please set by config file or command line -b !!!"<< endl;
        exit(-1);
    }

    load_msg_number();

    if(create_client_list() >= 0)
    {
        if (CONFIG.debugLevel >= DEBUG_LEVEL_D)
            cout << "Client list created!" << endl;
    }

    if(create_client_event_queue() >= 0)
    {
        if(CONFIG.debugLevel >= DEBUG_LEVEL_D)
            cout << "Client event queue created!" << endl;
    }

    if(create_data_sync_event_queue() >= 0)
    {
        if(CONFIG.debugLevel >= DEBUG_LEVEL_D)
            cout << "Data sync event queue created!" << endl;
    }

    if(create_thread_pool() >= 0)
    {
        if(CONFIG.debugLevel >= DEBUG_LEVEL_D)
            cout << "Client event process thread pool created!" << endl;
    }

//    if(create_data_sync_thread() >= 0)
//    {
//        if(CONFIG.debugLevel >= DEBUG_LEVEL_D)
//            cout << "Data sync event process thread created!" << endl;
//    }



    if(init_bbfile_access_semahpores() >= 0)
    {
        if (CONFIG.debugLevel >= DEBUG_LEVEL_D)
            cout << "Init bbfile access control semaphores success!" << endl;
    }

    if(init_sync_server_list() >= 0)
    {
        if (CONFIG.debugLevel >= DEBUG_LEVEL_D)
            cout << "Init sync server list success!" << endl;
    }

    if(create_tcp_connection_thread() >= 0)
    {
        if(CONFIG.debugLevel >= DEBUG_LEVEL_D)
            cout << "TCP connection process thread created!" << endl;
    }

    //sleep(1);



 /*
    if(init_sync_server_connection() >= 0)
    {
        if (CONFIG.debugLevel >= DEBUG_LEVEL_D)
            cout << "Init sync server connection success!" << endl;
    }

*/




    string str;

    while(getline(cin, str))
    {
        cout <<"Input: "<< str << endl;

        sleep(1);
    }


    return 0;
}

void *handle_tcp_connection(void *arg)
{
    char *uargv = nullptr;


    start_conn_service();

    return uargv;
}


void *handle_client_event(void *arg)
{
    char *uargv = nullptr;

    int fd, new_fd, bytesRecved, bytesSend;

    sockaddr_in clientAddr;
    socklen_t clientAddrSize = sizeof(clientAddr);

    clientEvent *pClientEv = nullptr;

    clientInfo *pClient = nullptr;

    clientEv evType;
    clientType cliType;

    char buf[1024];

    memset(buf, 0, sizeof(buf));

    while(true)
    {
        pClientEv = nullptr;
        pClient = nullptr;

        pClientEv = deClientEventQueue();

        if(nullptr == pClientEv)
        {
            continue;
        }

        evType = pClientEv->event;
        cliType = pClientEv->type;
        fd = pClientEv->fd;

        delete pClientEv;

        if(evType == EV_ACCEPT)
        {
            new_fd = accept(fd, (struct sockaddr *)&clientAddr, &clientAddrSize);

            CHECK(new_fd);

            if(new_fd >= 0)
            {
                if (CONFIG.debugLevel >= DEBUG_LEVEL_D)
                    cout << "Client: " << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port) << " connected!" << endl;

                //add new fd to fd set
                conn_add(new_fd);

                //add new client to client list
                clientInfo *pClient = new clientInfo;
                memset((void *)pClient, 0, sizeof(clientInfo));

                pClient->fd = new_fd;
                strcpy(pClient->ip, inet_ntoa(clientAddr.sin_addr));
                pClient->port = ntohs(clientAddr.sin_port);
                pClient->type = cliType;

                if(cliType == CLIENT_USER)
                {
                    pClient->slaveState = SYNC_IDLE;
                }

                client_list_add(pClient);

                //send greeting
                if(cliType == CLIENT_USER)
                {
                    CHECK(send(new_fd, MSG_GREETING,strlen(MSG_GREETING), 0));
                }
//                else if(cliType == CLIENT_SYNC_MASTER)
//                {
//                    //set state
//                    CHECK(send(new_fd, "hello I am master",strlen("hello I am master"), 0));
//                }
                else if(cliType == CLIENT_SYNC_SLAVE)
                {
                    //set state
                    //CHECK(send(new_fd, "hello I am slave",strlen("hello I am slave"), 0));
                }
            }
        }
        else if(evType == EV_RECV)
        {
            pClient = client_list_find(fd);

            if(!pClient)
            {
                //conn_del(fd);
                if(CONFIG.debugLevel >= DEBUG_LEVEL_APP)
                    cout << "client has been deleted! fd:" << fd << endl;

                continue;
            }

            bytesRecved = recv(fd, buf, sizeof(buf), 0);

            CHECK(bytesRecved);

            if (bytesRecved == 0) //client disconnected, delete
            {
                conn_del(fd);

                if(CONFIG.debugLevel >= DEBUG_LEVEL_D)
                    cout << "Client:" << pClient->ip<<":"<<pClient->port << " disconnected!" << endl;

                client_list_del(pClient);
            }
            else if (bytesRecved > 0)
            {
                if(CONFIG.debugLevel >= DEBUG_LEVEL_D)
                {
                    cout << "Recv msg from " << pClient->ip <<":" << pClient->port<<" [" << bytesRecved << " Bytes]:"
                          << string(buf, 0, bytesRecved)<< endl;
                }

                string response;

                response.clear();

                if(pClient->type == CLIENT_USER)
                {
                    process_client_msg(pClient,buf,bytesRecved, response);
                }
                else if(pClient->type == CLIENT_SYNC_MASTER)
                {
                    process_sync_master_msg(pClient,buf,bytesRecved, response);
                }
                else if(pClient->type == CLIENT_SYNC_SLAVE)
                {
                    process_sync_slave_msg(pClient,buf,bytesRecved, response);
                }

                if(!response.empty())
                {
                    response.append("\n");

                    if(CONFIG.debugLevel >= DEBUG_LEVEL_D)
                        cout << "Send response to "<< pClient->ip <<":" << pClient->port <<":" << response << endl;

                    bytesSend = send(fd,response.c_str(),response.size(),0);
                    CHECK(bytesSend);

                    // if QUIT cmd, disconnet the client
                    if(0 == response.compare(0, response.size()-2, "4.0 BYE"))
                    {
                        //conn_del(pClient->fd);
                        //client_list_del(pClient);
                    }
                }
            }
        }
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

    return 0;
}

int create_tcp_connection_thread()
{
    pthread_t threadConn;

    pthread_create(&threadConn, NULL, handle_tcp_connection, NULL);

    return 0;
}

int create_data_sync_thread()
{
    pthread_t threadDataSync;

    pthread_create(&threadDataSync, NULL, handle_data_sync_event, NULL);

    return 0;
}




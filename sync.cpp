#include <iostream>
#include <sys/select.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>

#include "common.h"
#include "config.h"
#include "list.h"
#include "sync.h"



using std::cout;
using std::endl;
using std::string;

int sync_connections[MAX_SYNC_CONN];

fd_set sync_fd_set;



int create_sync_server_sock()
{
    int fd = -1;

    fd = socket(AF_INET, SOCK_STREAM, 0);

    CHECK_EXIT(fd);

    //bind
    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(CONFIG.syncPort);
    hint.sin_addr.s_addr = INADDR_ANY;

    CHECK_EXIT(bind(fd,(sockaddr *)&hint, sizeof(hint)));

    //listen(listeningSock, SOMAXCONN);
    CHECK_EXIT(listen(fd, MAX_SYNC_CONN));

    return fd;
}

int start_sync_server()
{
    int server_fd, ret;

    int fd, new_fd, bytesRecved, bytesSend;

    sockaddr_in clientAddr;
    socklen_t clientAddrSize = sizeof(clientAddr);

    struct timeval tv;
    tv.tv_sec = 1; //1 second
    tv.tv_usec = 0;

    char buf[1024];
    memset(buf, 0, sizeof(buf));

    sync_conn_init();

    //create server listening socket
    server_fd = create_sync_server_sock();

    CHECK_EXIT(server_fd);

    sync_conn_add(server_fd);

    if(CONFIG.debugLevel >= DEBUG_LEVEL_NONE)
        cout << "Sync server created complete, listening --------->" << endl<<endl;

    while(true)
    {
        FD_ZERO(&sync_fd_set);

        sync_conn_set_fdset(&sync_fd_set);

        //Invoke select()
        ret = select(FD_SETSIZE, &sync_fd_set, NULL, NULL, &tv);

        CHECK(ret);

        if (ret >= 0)
        {
            if(FD_ISSET(server_fd, &sync_fd_set))//check if the fd with event is the sever fd, accept new connection
            {
                //accept
                new_fd = accept(server_fd, (struct sockaddr *)&clientAddr, &clientAddrSize);

                CHECK(new_fd);

                if(new_fd >= 0)
                {
                    if (CONFIG.debugLevel >= DEBUG_LEVEL_D)
                        cout << "Sync server: " << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port) << " connected!" << endl;

                    //add new fd to fd set
                    sync_conn_add(new_fd);

                    //add new client to client list
//                    clientInfo *pClient = new clientInfo;
//                    memset((void *)pClient, 0, sizeof(clientInfo));
//
//                    pClient->fd = new_fd;
//                    strcpy(pClient->ip, inet_ntoa(clientAddr.sin_addr));
//                    pClient->port = ntohs(clientAddr.sin_port);
//
//                    client_list_add(pClient);

                    //send greeting
                    send(new_fd, "Hello",strlen("Hello"), 0);
            }
            }
            else
            {
                //check if the fd with event is a non-server fd, then reveive data
                for(int i=1; i<MAX_SYNC_CONN; i++)
                {
                    if((sync_connections[i] > 0) && FD_ISSET(sync_connections[i], &sync_fd_set))
                    {
                        //receive

                        bytesRecved = recv(sync_connections[i], buf, sizeof(buf), 0);

                        CHECK(bytesRecved);

                        if (bytesRecved == 0) //client disconnected, delete
                        {
                            sync_conn_del(fd);

                            if(CONFIG.debugLevel >= DEBUG_LEVEL_D)
                                //cout << "Sync server:" << pClient->ip<<":"<<pClient->port << " disconnected!" << endl;
                                cout << "Sync server disconnected!" << endl;

                        }
                        else if (bytesRecved > 0)
                        {
                            if(CONFIG.debugLevel >= DEBUG_LEVEL_D)
                            {
                                //cout << "Recved sync msg from " << pClient->ip <<":" << pClient->port<<" [" << bytesRecved << " Bytes]: "
                                 //    << string(buf, 0, bytesRecved)<< endl;

                                 cout << "Recved sync msg [" << bytesRecved << " Bytes]: " << string(buf, 0, bytesRecved)<< endl;
                            }

                        }

                    }
                }
            }
        }

        usleep(1000);

    }//while()

    // close all sockets
    //conn_close_all();

    return 0;
}













void sync_conn_init()
{
    unsigned int i;

    for (i=0;i<MAX_SYNC_CONN;i++)
    {
        sync_connections[i] = -1;
    }

    FD_ZERO(&sync_fd_set);

    return;
}

void sync_conn_set_fdset(fd_set *pFdSet)
{
    unsigned int i;

    if(!pFdSet)
    {
        return;
    }

    for(i=0;i<MAX_SYNC_CONN;i++)
    {
        if(sync_connections[i] >= 0)
        {
            FD_SET(sync_connections[i], pFdSet);
        }
    }

    return;
}

bool sync_conn_is_exist(int fd)
{
    if(fd < 0)
    {
        return false;
    }

    for (int i=0;i<MAX_SYNC_CONN;i++)
    {
        if(sync_connections[i] == fd)
        {
            return true;
        }
    }

    return false;
}

void sync_conn_add(int fd)
{
    unsigned int i;

    if(sync_conn_is_exist(fd))
    {
        return;
    }

    for (i=0;i<MAX_SYNC_CONN;i++)
    {
        if(sync_connections[i] < 0)
        {
            sync_connections[i] = fd;

            break;
        }
    }

    FD_SET(fd, &sync_fd_set);

    if(CONFIG.debugLevel >= DEBUG_LEVEL_CONN)
        cout << "sync connection add fd:" <<fd<<endl;

    return;
}

void sync_conn_del(int fd)
{
    unsigned int i;

    if(!sync_conn_is_exist(fd))
    {
        return;
    }

    CHECK(close(fd));

    FD_CLR(fd, &sync_fd_set);

    for (i=0;i<MAX_SYNC_CONN;i++)
    {
        if(sync_connections[i] == fd)
        {
            sync_connections[i] = -1;

            //break;
        }
    }


    if(CONFIG.debugLevel >= DEBUG_LEVEL_CONN)
        cout << "sync connection del fd:" <<fd<<endl;

    return;
}

void sync_conn_close_all()
{
    unsigned int i;

    FD_ZERO(&sync_fd_set);

    for (i=0;i<MAX_SYNC_CONN;i++)
    {
        if(sync_connections[i] > 0)
        {
            CHECK(close(sync_connections[i]));
            sync_connections[i] = -1;
        }
    }


    return;
}









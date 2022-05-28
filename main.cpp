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

#include "common.h"
#include "myqueue.h"
#include "connection.h"

using namespace std;


#define SUCCESS 1
#define ERR -1

#define THMAX 20
#define BBPORT 8000+254
#define SYNCPORT 10000+254
#define BBFILE "bbfile"
#define PEERS "127.0.0.1:8000, "
#define DAEMON true
#define DEBUG true


int load_config()
{
    memset((void *) &CONFIG, 0, sizeof(CONFIG));

    CONFIG.thMax = 20;
    CONFIG.bbPort = BBPORT;
    CONFIG.syncPort = SYNCPORT;
    strcpy(CONFIG.bbFile,"bbfile");
    strcpy(CONFIG.peers, "127.0.0.1:8000");
    CONFIG.daemon = DAEMON;
    CONFIG.debug = DEBUG;

    CONFIG.maxConnections = CONFIG.thMax;

    cout << "load config success!" << endl;

    if (CONFIG.debug)
    {
        cout << "THMAX = " << CONFIG.thMax << endl << "BBPORT = " << CONFIG.bbPort << endl << "SYNCPORT = " << CONFIG.bbPort << endl;
        cout << "BBFILE = " << CONFIG.bbFile << endl << "PEERS = " << CONFIG.peers << endl;
        cout << "DAEMON = " << CONFIG.daemon << endl << "DEBUG = " << CONFIG.debug << endl;
    }


    return SUCCESS;
}

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

void * handle_client(void * arg)
{
    char *argv = nullptr;

    int fd, bytesRecved = -1;

    sockaddr_in clientAddr;

    client *pClient = nullptr;

    char buf[4096];

    memset(buf, 0, sizeof(buf));

    while(true)
    {
        pClient = deClientQueue();

        if(nullptr == pClient)
        {
            continue;
        }

        cout << "deQueue Client fd = " << pClient->fd << endl;

        fd = pClient->fd;

        clientAddr = pClient->clientAddr;

        delete pClient;

        bytesRecved = recv(fd, buf, sizeof(buf), 0);

        if (bytesRecved == -1)
        {
            cout << "Error in recv()" << endl;
            continue;
        }
        else if (bytesRecved == 0)
        {
            cout << "Client:" << inet_ntoa(clientAddr.sin_addr) << " disconnected!" << endl;

            conn_del(fd);

            continue;
        }
        else if (bytesRecved > 0)
        {
            cout << "Msg recved from " << inet_ntoa(clientAddr.sin_addr) <<":" << ntohs(clientAddr.sin_port)<<" [" << bytesRecved << " Bytes]: "
                << string(buf, 0, bytesRecved)<< endl;

            //echo
            send(fd, buf, bytesRecved+1, 0);
        }
    }

    return argv;
}

int create_thread_pool()
{
    pthread_t threadPool[CONFIG.thMax];

    for(unsigned int i=0;i<CONFIG.thMax;i++)
    {
        pthread_create(&threadPool[i], NULL, handle_client, NULL);
    }

    cout << "client thread pool created!" << endl;

    return SUCCESS;
}


int main(int argc, char **argv)
{
    load_config();

    create_client_queue();

    create_thread_pool();

    start_comm_service();

//    client *c1, *c2, *c3;
//
//    c1 = new client;
//    c2 = new client;
//    c3 = new client;
//    c1->fd =1;
//    c2->fd =2;
//    c3->fd =3;
//
//
//
//    create_client_queue();
//
//    enClientQueue(c1);
//    enClientQueue(c2);
//    enClientQueue(c3);
//
//    client *c;
//
//    while(!isClientQueueEmpty())
//    {
//        c = deClientQueue();
//        cout << c->fd << endl;
//    }
//
//    delete c1;
//    delete c2;
//    delete c3;



    //int lis = socket()
    return 0;
}

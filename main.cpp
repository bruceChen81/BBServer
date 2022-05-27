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

#include "common.h"

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




typedef struct sysCfg
{
    unsigned int thMax;
    unsigned short bbPort;
    unsigned short syncPort;
    string bbFile;
    string peers;
    bool daemon;
    bool debug;
    unsigned int maxConnections;
}sysCfg;

sysCfg CONFIG;

int load_config()
{
    memset((void *) &CONFIG, 0, sizeof(CONFIG));

    CONFIG.thMax = 20;
    CONFIG.bbPort = BBPORT;
    CONFIG.syncPort = SYNCPORT;
    CONFIG.bbFile = "bbfile";
    CONFIG.peers = "127.0.0.1:8000";
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
    int server_fd, new_fd, ret, i;

    int maxConn = CONFIG.maxConnections;

    sockaddr_in clientAddr;
    socklen_t clientAddrSize = sizeof(clientAddr);

    fd_set read_fd_set;

    int connections[maxConn];

    char buf[4096];
    int bytesRecved = -1;

    memset(buf, 0, sizeof(buf));

    for (i=0;i<maxConn;i++)
    {
        connections[i] = -1;
    }

    //create server listening socket
    server_fd = create_tcp_server_sock();

    if (ERR == server_fd)
    {
        return ERR;
    }


    connections[0] = server_fd;

    while(true)
    {
        FD_ZERO(&read_fd_set);

        for(i=0;i<maxConn;i++)
        {
            if(connections[i] >= 0)
            {
                FD_SET(connections[i], &read_fd_set);
            }

        }

        //Invoke select()
        //cout << "Invoke select to listen for incoming events" << endl;

        ret = select(FD_SETSIZE, &read_fd_set, NULL, NULL, NULL);

        //cout << "Select returned with " << ret << endl;

        if (ret >= 0)
        {
            //check if the fd with event is the sever fd, accept new connection
            if(FD_ISSET(server_fd, &read_fd_set))
            {
                new_fd = accept(server_fd, (struct sockaddr *)&clientAddr, &clientAddrSize);

                if(new_fd >= 0)
                {
                    cout << "Client: " << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port) << " connected!" << endl;
                    //add new fd to fd set
                    for (i=0;i<maxConn;i++)
                    {
                        if(connections[i] < 0)
                        {
                            connections[i] = new_fd;
                            break;
                        }
                    }
                }
                else{

                    cout << "Error on accepting!" << endl;
                }
            }
         }

        //check if the fd with event is a non-server fd, reveive data
        for (i=0;i<maxConn;i++)
        {
            if((connections[i] > 0) && (connections[i] != server_fd) && FD_ISSET(connections[i], &read_fd_set))
            {
                bytesRecved = recv(connections[i], buf, sizeof(buf), 0);

                if (bytesRecved == -1)
                {
                    cout << "Error in recv()" << endl;
                    break;
                }
                else if (bytesRecved == 0)
                {
                    cout << "Client:" << inet_ntoa(clientAddr.sin_addr) << " disconnected!" << endl;

                    close(connections[i]);
                    connections[i] = -1;

                    break;
                }
                else if (bytesRecved > 0)
                {
                    cout << "Msg recved from " << inet_ntoa(clientAddr.sin_addr) <<":" << ntohs(clientAddr.sin_port)<<" [" << bytesRecved << " Bytes]: "
                        << string(buf, 0, bytesRecved)<< endl;

                    //echo
                    send(connections[i], buf, bytesRecved+1, 0);
                }

            }
        }
    }//while()

    // close all sockets
    for (i=0;i<maxConn;i++)
    {
        if(connections[i] > 0)
        {
            close(connections[i]);
            connections[i] = -1;
        }
    }


    return SUCCESS;

}

int main(int argc, char **argv)
{
    load_config();

    start_comm_service();



    //int lis = socket()
    return 0;
}

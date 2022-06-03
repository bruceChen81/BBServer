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
//using std::string::npos;
using std::cin;


int process_msg(clientInfo *pClient, char *buf, int length)
{
    std::size_t pos1, pos2;

    string arg;
    string arg1, arg2;
    string response;

    string msg = string(buf,length);

    cout << "Msg: "<< msg << endl;

    //COMMANDS: USER, READ, WRITE, REPLACE, QUIT

    pos1 = msg.find(" ");

    if(pos1 != string::npos)
    {
//        cout << "pos1:" << pos1 <<endl;
//        cout << "size:" << msg.size()<<endl;

        arg1 = msg.substr(0, pos1);
        arg2 = msg.substr(pos1+1, msg.size()-pos1-2); //delete /n in the end

        if(!arg1.empty())
        {
            cout << "arg1:" << arg1 <<endl;
        }

        if(!arg2.empty())
        {
            cout << "arg2:" <<arg2 <<endl;
        }

        if(0 == arg1.compare(string("USER")))
        {
            cout << "USER" << endl;

            strcpy(pClient->name, arg2.c_str());

            cout << "add client name: " << pClient->name << endl;
        }

        if((arg2.find("/") != string::npos) || (arg2.find(" ") != string::npos))
        {
            cout << "ERROR USER" << endl;
            response.append("1.2 ERROR USER");
        }
    }

    return 0;


    //USER

}


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

    int fd, new_fd, bytesRecved, bytesSend;

    sockaddr_in clientAddr;
    socklen_t clientAddrSize = sizeof(clientAddr);

    clientEvent *pClientEv = nullptr;

    clientInfo *pClient = nullptr;

    clientEv evType;

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

        //cout << "deQueue Client fd = " << pClient->fd << endl;

        evType = pClientEv->event;
        fd = pClientEv->fd;
        delete pClientEv;

        if(evType == EV_ACCEPT)
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

                    //pClient->ip = string(inet_ntoa(clientAddr.sin_addr));

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
        else if(evType == EV_RECV)
        {
            pClient = client_list_find(fd);

            if(!pClient)
            {
                //conn_del(fd);
                cout << "client has been deleted! fd:" << fd << endl;
                continue;
            }

            bytesRecved = recv(fd, buf, sizeof(buf), 0);

            CHECK(bytesRecved);

            if (bytesRecved == 0) //client disconnected, delete
            {
                conn_del(fd);

                cout << "Client:" << pClient->ip<<":"<<pClient->port << " disconnected!" << endl;

                client_list_del(pClient);
            }
            else if (bytesRecved > 0)
            {
                //cout << "Msg recved from " << pClient->ip <<":" << pClient->port<<" [" << bytesRecved << " Bytes]: " << string(buf, 0, bytesRecved)<< endl;

                process_msg(pClient,buf,bytesRecved);

//                char *str = "haha";
////
//                client_list_save_name(fd, str);
//
//                pClient = client_list_find(fd);
//
//
//
//                //pClient->name = string("haha");
//
//                cout << "add client name: " << pClient->name <<endl;

                //echo
                //send(fd, buf, bytesRecved+1, 0);
                bytesSend = send(fd,MSG_GREETING,strlen(MSG_GREETING),0);
                CHECK(bytesSend);
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

    cout << "client event thread pool created!" << endl;

    return SUCCESS;
}




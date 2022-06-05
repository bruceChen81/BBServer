//main.cpp
//
//Created by Dianyong Chen, 2022-05-24
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
#include <climits>

#include "common.h"
#include "config.h"
#include "queue.h"
#include "connection.h"
#include "list.h"

using std::cout;
using std::endl;
using std::string;
using std::fstream;
//using std::string::npos;
using std::cin;


int process_msg(clientInfo *pClient, char *buf, int length, string& response)
{
    std::size_t pos1;

    string arg;
    string arg1, arg2;
    //string response;

    string msg = string(buf,length);

    cout << "Msg: "<< msg << endl;

    //COMMANDS: USER, READ, WRITE, REPLACE, QUIT

    pos1 = msg.find(" ");

    if(pos1 == string::npos)
    {
        //only QUIT command has no space
        if((msg.size()== 6) && (0 == msg.compare(0,4,"QUIT")))
        {
            response.append("4.0 BYE");
        }

        return 0;
    }


    arg1 = msg.substr(0, pos1);
    arg2 = msg.substr(pos1+1, msg.size()-pos1-2); //delete \n in the end

    if(arg1.empty() || arg2.empty())
    {
        response.append("ERROR COMMAND");
        cout << "error command received!" << endl;

        return 0;
    }

    cout << "arg1:" << arg1 <<" arg2:" <<arg2 <<endl;



    if(0 == arg1.compare(string("USER")))
    {
        //USER name
        //1.0 HELLO name text
        //1.2 ERROR USER text

        if((arg2.find("/") != string::npos) || (arg2.find(" ") != string::npos))
        {
            cout << "ERROR USER" << endl;
            response.append("1.2 ERROR USER");
        }
        else
        {
            cout << "USER" << endl;
            response.append("1.0 HELLO ");
            response.append(arg2);

            //strcpy(pClient->name, arg2.c_str());
            client_list_save_name(pClient->fd, arg2.c_str());

            cout << "add client name: " << pClient->name << endl;
        }
    }
    else if(0 == arg1.compare(string("READ")))
    {
        //READ message-number
        //2.0 MESSAGE message-number poster/message
        //2.1 UNKNOWN message-number text
        //2.2 ERROR READ text



    }
    else if(0 == arg1.compare(string("WRITE")))
    {
        //WRITE message
        //save:message-number/poster/message
        //3.0 WROTE message-number
        //3.2 ERROR WRITE text

        fstream myFile;
        string msgSave;
        string username;

        if(strlen(pClient->name) != 0)
        {
            username = string(pClient->name, strlen(pClient->name)-1);
        }
        else
        {
            username.append("nobody");
        }

        int msg_number = client_get_msg_number();

        msgSave.append(std::to_string(msg_number));
        msgSave += "/";
        msgSave += username;
        msgSave += "/";
        msgSave += arg2;

        cout <<"MSG save:"<< msgSave << endl;

        myFile.open(CONFIG.bbFile, std::ios::app);//write, append
        if(myFile.is_open())
        {
            myFile << msgSave << endl;
            myFile.close();

            response.append("3.0 WROTE ");
            response.append(std::to_string(msg_number));
        }
    }
    else if(0 == arg1.compare(string("REPLACE")))
    {
        //REPLACE message-number/message
        //3.1 UNKNOWN message-number

    }
    else if(0 == arg1.compare(string("QUIT")))
    {
        //QUIT text
        //4.0 BYE text

        response.append("4.0 BYE");
    }
    else
    {
        response.append("ERROR COMMAND");
    }

    return 0;
}


int main(int argc, char *argv[])
{
    load_config((char *)DEFAULT_CFG_FILE);

    load_option(argc, argv);

    //check if bbfile is set
    if(!strlen(CONFIG.bbFile))
    {
        cout << "BBFILE name is not configured, please set by config file or command line -b !!!"<< endl;
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

                string response;

                response.clear();

                process_msg(pClient,buf,bytesRecved, response);

                response.append("\n");

                cout << "response:" << response << endl;

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




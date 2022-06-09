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
#include <climits>

#include "common.h"
#include "config.h"
#include "queue.h"
#include "connection.h"
#include "list.h"
#include "msg.h"

using std::cout;
using std::endl;
using std::string;
using std::fstream;
//using std::string::npos;
using std::cin;




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

    load_msg_number();

    create_client_list();

    create_client_event_queue();

    create_thread_pool();

    create_msg_save_thread();

    init_bbfile_access_semahpores();

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

            CHECK(new_fd);

            if(new_fd >= 0)
            {
                cout << "Client: " << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port) << " connected!" << endl;
                //add new fd to fd set
                conn_add(new_fd);

                //add new client to client list
                clientInfo *pClient = new clientInfo;
                memset((void *)pClient, 0, sizeof(clientInfo));

                pClient->fd = new_fd;
                strcpy(pClient->ip, inet_ntoa(clientAddr.sin_addr));
                pClient->port = ntohs(clientAddr.sin_port);

                client_list_add(pClient);

                //send greeting
                send(new_fd, MSG_GREETING,strlen(MSG_GREETING), 0);
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

                if(CONFIG.debugLevel >= DEBUG_LEVEL_APP)
                    cout << "Client:" << pClient->ip<<":"<<pClient->port << " disconnected!" << endl;

                client_list_del(pClient);
            }
            else if (bytesRecved > 0)
            {
                if(CONFIG.debugLevel >= DEBUG_LEVEL_APP)
                {
                    cout << "receive msg from " << pClient->ip <<":" << pClient->port<<" [" << bytesRecved << " Bytes]: "
                          << string(buf, 0, bytesRecved)<< endl;
                }

                string response;

                response.clear();

                process_msg(pClient,buf,bytesRecved, response);

                response.append("\n");

                if(CONFIG.debugLevel >= DEBUG_LEVEL_APP)
                    cout << "send response:" << response << endl;

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

void *handle_msg_save_event(void *arg)
{
    char *uargv = nullptr;

    msgSaveEvent *pMsgSaveEv;

    fstream myFile;

    while(true)
    {
        pMsgSaveEv = nullptr;

        pMsgSaveEv = deMsgSaveEventQueue();

        if(CONFIG.debugLevel >= DEBUG_LEVEL_APP)
            cout << "handle_msg_save_event: "<<pMsgSaveEv->msg << endl;

        if(nullptr == pMsgSaveEv)
        {
            continue;
        }

        if(pMsgSaveEv->event == MSG_SAVE_WRITE)
        {
            write_start();

            myFile.open(CONFIG.bbFile, std::ios::app);//write, append

            if(myFile.is_open())
            {
                myFile << pMsgSaveEv->msg << endl;
                myFile.close();
            }

            write_end();

        }
        else if(pMsgSaveEv->event == MSG_SAVE_REPLACE)
        {
            long posLineStart;
            string line, numberSaved,numberMsg;

            write_start();

            myFile.open(CONFIG.bbFile, std::ios::in|std::ios::out);//read and write
            if(myFile.is_open())
            {
                while(getline(myFile, line))
                {
                    posLineStart = myFile.tellg()-(long)line.length()-(long)1;

                    get_msg_number_byline(numberSaved,line);
                    get_msg_number_byline(numberMsg, pMsgSaveEv->msg);

                    //cout << "numberSaved:"<<numberSaved<<endl;
                    //cout << "numberMsg:"<<numberMsg<<endl;

                    //compare message-number
                    if(numberMsg == numberSaved)
                    {
                        //replace
                        myFile.seekp(posLineStart);

                        myFile << pMsgSaveEv->msg;

                        if(CONFIG.debugLevel >= DEBUG_LEVEL_APP)
                            cout << "replace msg saved successfully!" << endl;

                        break;
                    }
                }

                myFile.close();
            }

            write_end();
        }
        else if(pMsgSaveEv->event == MSG_SAVE_REPLACE_PLUS)
        {
            long posLineStart;
            string line, numberSaved,numberMsg;

            write_start();

            myFile.open(CONFIG.bbFile, std::ios::in|std::ios::out);//read and write
            if(myFile.is_open())
            {
                while(getline(myFile, line))
                {
                    posLineStart = myFile.tellg()-(long)line.length()-(long)1;

                    get_msg_number_byline(numberSaved,line);
                    get_msg_number_byline(numberMsg, pMsgSaveEv->msg);

                    //cout << "numberSaved:"<<numberSaved<<endl;
                    //cout << "numberMsg:"<<numberMsg<<endl;

                    //compare message-number
                    if(numberMsg == numberSaved)
                    {
                        //replace
                        myFile.seekp(posLineStart);

                        myFile << pMsgSaveEv->msg;

                        if(line.length() > pMsgSaveEv->msg.length())
                        {
                            string str = string(line.length() - pMsgSaveEv->msg.length(), ' ');

                            myFile << str;
                        }

                        if(CONFIG.debugLevel >= DEBUG_LEVEL_APP)
                            cout << "replace msg saved successfully!" << endl;

                        break;
                    }
                }

                myFile.close();
            }

            write_end();
        }

        delete pMsgSaveEv;
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

    if(CONFIG.debugLevel >= DEBUG_LEVEL_APP)
        cout << "client event thread pool created!" << endl;

    return SUCCESS;
}

int create_msg_save_thread()
{
    pthread_t threadMsgSave;

    pthread_create(&threadMsgSave, NULL, handle_msg_save_event, NULL);


    if(CONFIG.debugLevel >= DEBUG_LEVEL_APP)
        cout << "client msg save thread created!" << endl;

    return SUCCESS;
}




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
    std::size_t pos1, pos2;

    string arg;
    string arg1, arg2;
    //string response;

    string msg = string(buf,length);

    string msgSave;
    string username;
    string strNumber;

    fstream myFile;
    string line;

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
    arg2 = msg.substr(pos1+1, msg.size()-pos1-3); //delete \n in the end

    if(arg1.empty() || arg2.empty())
    {
        response.append("ERROR COMMAND");
        cout << "error command received!" << endl;

        return 0;
    }

    cout << "arg1:" << arg1 << " len:" << arg1.size()<<endl;
    cout << "arg2:" << arg2 << " len:" << arg2.size()<<endl;



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
        myFile.open(CONFIG.bbFile, std::ios::in);//read
        if(myFile.is_open())
        {
            while(getline(myFile, line))
            {
                pos2 = line.find("/");
                if(pos2 != std::string::npos)
                {
                    if(arg2 == line.substr(0, pos2))
                    {
                        response += line;
                        break;
                    }
                }
            }
            // msg number no found
            if(response.empty())
            {
                response += "2.1 UNKNOWN ";
                response += arg2;
            }
            //cout << myFile;
            myFile.close();
        }
        else
        {
            response += "2.2 ERROR READ";
        }
    }
    else if(0 == arg1.compare(string("WRITE")))
    {
        //WRITE message
        //save:message-number/poster/message
        //3.0 WROTE message-number
        //3.2 ERROR WRITE text

        msgSaveEvent *pMsgSaveEv = new msgSaveEvent;

        get_new_msg_number(strNumber);

        pMsgSaveEv->msg += strNumber;
        pMsgSaveEv->msg += "/";

        if(strlen(pClient->name) != 0)
        {
            pMsgSaveEv->msg += string(pClient->name, strlen(pClient->name));
        }
        else
        {
            pMsgSaveEv->msg += "nobody";
        }

        pMsgSaveEv->msg += username;
        pMsgSaveEv->msg += "/";
        pMsgSaveEv->msg += arg2;

        cout <<"MSG save:"<< pMsgSaveEv->msg << endl;

        response.append("3.0 WROTE ");
        response.append(strNumber);

        pMsgSaveEv->event = MSG_SAVE_WRITE;

        enMsgSaveEventQueue(pMsgSaveEv);

        /*
        myFile.open(CONFIG.bbFile, std::ios::app);//write, append
        if(myFile.is_open())
        {
            myFile << msgSave << endl;
            myFile.close();

            response.append("3.0 WROTE ");
            response.append(strNumber);
        }
        */
    }
    else if(0 == arg1.compare(string("REPLACE")))
    {
        //REPLACE message-number/message
        //3.1 UNKNOWN message-number
        long posLineStart;
        string msgSaved,userSaved,numberSaved,msgInput,newLine;

        myFile.open(CONFIG.bbFile, std::ios::in);//read
        if(myFile.is_open())
        {
            while(getline(myFile, line))
            {
                posLineStart = myFile.tellg()-(long)line.length()-(long)1;

                cout << "line:"<<line << endl;
                cout << "Pos:"<<posLineStart << endl;

                pos1 = line.find_first_of("/");
                pos2 = line.find_last_of("/");

                get_msg_number_byline(numberSaved,line);
                get_msg_username_byline(userSaved, line);
                get_msg_body_byline(msgSaved, line);

                msgInput = arg2.substr(arg2.find("/")+1);

                cout << "numberSaved:"<<numberSaved<<endl;
                cout << "msgSaved:"<<msgSaved<<endl;
                cout << "userSaved:"<<userSaved<<endl;
                cout << "msgInput:"<<msgInput<<endl;

                //compare message-number
                if(arg2.substr(0, arg2.find("/")) == line.substr(0, line.find("/")))
                {
                    //cout << "Pos3:"<<std::ios_base::cur << endl;

                    msgSaveEvent *pMsgSaveEvReplace = new msgSaveEvent;

                    pMsgSaveEvReplace->msg = line.substr(0, line.find_last_of("/")+1); // include '/'
                    pMsgSaveEvReplace->msg += msgInput;

                    cout << "new line:" << pMsgSaveEvReplace->msg <<endl;

                    pMsgSaveEvReplace->event = MSG_SAVE_REPLACE;

                    enMsgSaveEventQueue(pMsgSaveEvReplace);
/*
                    //replace
                    myFile.seekp(posLineStart);

                    //myFile.write("",line.length());

                    myFile.seekp(posLineStart);

                    myFile << newLine;

                    if(msgSaved.length() > msgInput.length())
                    {
                        //copy entire file to another file, replace the line meanwhile

                    }
*/
                    response.append("3.3 REPLACED ");
                    response.append(arg2);

                    break;
                }
            }

            myFile.close();

            if(response.empty())
            {
                response.append("3.1 UNKNOWN ");
                response.append(arg2);
            }
        }
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

    load_msg_number();

    create_client_list();

    create_client_event_queue();

    create_thread_pool();

    create_msg_save_thread();

//    {
//        string lastline;
//
//        get_last_line(lastline);
//    }

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

void *handle_msg_save_event(void *arg)
{
    char *uargv = nullptr;

    msgSaveEvent *pMsgSaveEv;

    fstream myFile;

    while(true)
    {
        pMsgSaveEv = nullptr;

        pMsgSaveEv = deMsgSaveEventQueue();

        cout << "handle_msg_save_event: "<<pMsgSaveEv->msg << endl;

        if(nullptr == pMsgSaveEv)
        {
            continue;
        }

        if(pMsgSaveEv->event == MSG_SAVE_WRITE)
        {
            myFile.open(CONFIG.bbFile, std::ios::app);//write, append

            if(myFile.is_open())
            {
                myFile << pMsgSaveEv->msg << endl;
                myFile.close();
            }

        }
        else if(pMsgSaveEv->event == MSG_SAVE_REPLACE)
        {
            long posLineStart;
            string line, numberSaved,numberMsg;

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
                            //copy entire file to another file, replace the line meanwhile

                        }

                        cout << "replace msg saved successfully!" << endl;

                        break;
                    }
                }

                myFile.close();
            }
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

    cout << "client event thread pool created!" << endl;

    return SUCCESS;
}

int create_msg_save_thread()
{
    pthread_t threadMsgSave;

    pthread_create(&threadMsgSave, NULL, handle_msg_save_event, NULL);


    cout << "client msg save thread created!" << endl;

    return SUCCESS;
}




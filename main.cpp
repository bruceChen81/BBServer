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








int send_response_to_client(clientInfo* pClient, std::string& response)
{
    int bytesSend;

    response += "\n";

    bytesSend = send(pClient->fd, response.c_str(), response.size(), 0);
    CHECK(bytesSend);

    if(CONFIG.debugLevel >= DEBUG_LEVEL_D)
            cout << "Send response to "<< pClient->ip <<":" << pClient->port <<":" << response << endl;

    return 0;
}

int proc_client_ev_accept(clientEvent *pClientEv)
{
    sockaddr_in clientAddr;
    socklen_t clientAddrSize = sizeof(clientAddr);

    int new_fd = accept(pClientEv->fd, (struct sockaddr *)&clientAddr, &clientAddrSize);

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
        pClient->type = pClientEv->type;

        if(pClientEv->type == CLIENT_USER)
        {
            pClient->slaveState = SYNC_IDLE;
        }

        client_list_add(pClient);

        //send greeting
        if(pClientEv->type == CLIENT_USER)
        {
            CHECK(send(new_fd, MSG_GREETING,strlen(MSG_GREETING), 0));
        }
//                else if(cliType == CLIENT_SYNC_MASTER)
//                {
//                    //set state
//                    CHECK(send(new_fd, "hello I am master",strlen("hello I am master"), 0));
//                }
        else if(pClientEv->type == CLIENT_SYNC_SLAVE)
        {
            //set state
            //CHECK(send(new_fd, "hello I am slave",strlen("hello I am slave"), 0));
        }
    }

    return 0;
}

int proc_client_ev_recv(clientEvent* pClientEv)
{
    int bytesRecved;

    char buf[1024];
    memset(buf, 0, sizeof(buf));

    clientInfo *pClient = client_list_find(pClientEv->fd);

    if(!pClient)
    {
        if(CONFIG.debugLevel >= DEBUG_LEVEL_APP)
            cout << "client has been deleted! fd:" << pClientEv->fd << endl;

        return -1;
    }

    bytesRecved = recv(pClient->fd, buf, sizeof(buf), 0);

    CHECK(bytesRecved);

    if (bytesRecved == 0) //client disconnected, delete
    {

        conn_del(pClient->fd);

        if(CONFIG.debugLevel >= DEBUG_LEVEL_D)
            cout << "Client:" << pClient->ip<<":"<<pClient->port << " disconnected!" << endl;

        if(pClient->type == CLIENT_SYNC_MASTER)
        {
            //sync_set_master_state(SYNC_DISCONNECT);
        }

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
            send_response_to_client(pClient, response);

//            if(CONFIG.debugLevel >= DEBUG_LEVEL_D)
//                cout << "Send response to "<< pClient->ip <<":" << pClient->port <<":" << response << endl;
//
//            bytesSend = send(pClient->fd, response.c_str(), response.size(),0);
//            CHECK(bytesSend);

            // if QUIT cmd, disconnet the client
            if(0 == response.compare(0, response.size()-2, "4.0 BYE"))
            {
                //conn_del(pClient->fd);
                //client_list_del(pClient);
            }
        }
    }

    return 0;
}


int proc_sync_ev_precommit_ack(clientEvent *pClientEv)
{
    cout << "sync precommited ack" <<endl;

    clientInfo* pClientUser;

    while((pClientUser = sync_find_waiting_commit_user_client()) != nullptr)
    {
        sync_send_commit(pClientUser->cmd, pClientUser->msg);

        sync_set_client_state(pClientUser, SYNC_U_WAITING_SAVE);
    }

    return 0;
}

int proc_sync_ev_precommit_err(clientEvent *pClientEv)
{
    cout << "sync precommited error:" <<endl;

    sync_send_abort();

    clientInfo* pClientUser;

    while((pClientUser = sync_find_waiting_commit_user_client()) != nullptr)
    {
        string response = "3.2 ERROR WRITE";

        send_response_to_client(pClientUser, response);

        sync_clear_client_cmd(pClientUser);
    }

    return 0;
}

int proc_sync_ev_commit_success(clientEvent *pClientEv)
{
    clientInfo* pClientUser;

    int ret;
    bool isSuccess;

    string response;

    while((pClientUser = sync_find_waiting_save_user_client()) != nullptr)
    {
        if(pClientEv->msgNumber != pClientUser->msgNumber)
        {
            continue;
        }

        response.clear();

        if(pClientUser->cmd == CLIENT_CMD_WRITE)
        {
            ret = save_msg_write(pClientUser->msg);
            if(ret == 0)
            {
                response.append("3.0 WROTE ");
                response.append(pClientUser->msg.substr(0, pClientUser->msg.find("/")));

                isSuccess = true;
            }
            else
            {
                response.append("3.2 ERROR WRITE");

                isSuccess = false;
            }

        }
        else if(pClientUser->cmd == CLIENT_CMD_REPLACE)
        {
            ret = save_msg_write(pClientUser->msg);
            if(ret == 0)
            {
                response.append("3.0 WROTE ");
                response.append(pClientUser->msg.substr(0, pClientUser->msg.find("/")));

                isSuccess = true;
            }
            else if(ret == -1)
            {
                response.append("3.1 UNKNOWN ");
                response.append(pClientUser->msg.substr(0, pClientUser->msg.find("/")));

                isSuccess = false;
            }
            else
            {
                response.append("3.2 ERROR WRITE");

                isSuccess = false;
            }
        }

        sync_send_success(isSuccess, pClientEv->msgNumber);

        send_response_to_client(pClientUser, response);

        sync_clear_client_cmd(pClientUser);
    }

    return 0;
}

int proc_sync_ev_commit_unsuccess(clientEvent *pClientEv)
{
    clientInfo* pClientUser;

    string response;

    while((pClientUser = sync_find_waiting_save_user_client()) != nullptr)
    {
        if(pClientEv->msgNumber != pClientUser->msgNumber)
        {
            continue;
        }

        response.clear();

        if(pClientUser->cmd == CLIENT_CMD_WRITE)
        {
            response.append("3.2 ERROR WRITE");
        }
        else if(pClientUser->cmd == CLIENT_CMD_REPLACE)
        {
            string str = "6.1 COMMITED UNSUCCESS UNKNOWN";

            if(pClientEv->response.compare(0, str.length(), str) == 0) //"6.1 COMMITED UNSUCCESS unknown-message-number")
            {
                response.append("3.1 UNKNOWN ");
                //response.append(pClientUser->msg.substr(0, pClientUser->msg.find("/")));
                response.append(pClientUser->msgNumber);
            }
            else
            {
                response.append("3.2 ERROR WRITE");
            }
        }

        send_response_to_client(pClientUser, response);

        sync_clear_client_cmd(pClientUser);

        sync_send_success(false, pClientEv->msgNumber);
    }

    return 0;
}

int proc_sync_ev_master_timeout(clientEvent *pClientEv)
{
    string msgNum, msg;

    syncState state = sync_get_master_state();

    if(state == SYNC_M_PRECOMMIT_MULTICASTED)
    {
        sync_send_event(EV_SYNC_PRECOMMIT_ERR, msgNum, -1, msg);
    }
    else if(state == SYNC_M_COMMITED)
    {
        sync_send_event(EV_SYNC_COMMIT_UNSUCCESS, msgNum, -1, msg);
    }

    return 0;
}

int proc_sync_ev_slave_timeout(clientEvent *pClientEv)
{
    clientInfo* pClientSlave = client_list_get_first();

    while(pClientSlave)
    {
        if((pClientSlave->type == CLIENT_SYNC_SLAVE) && (pClientSlave->slaveState == SYNC_S_PRECOMMIT_ACK))
        {
            pClientSlave->slaveTimeout = true;
        }

        pClientSlave = client_list_get_next(pClientSlave);
    }

    sleep(1);

    while((pClientSlave = sync_find_waiting_commit_slave_client()) != nullptr)
    {
        sync_set_slave_state(pClientSlave, SYNC_IDLE);
        pClientSlave->slaveTimeout = false;
    }

    return 0;
}


void *handle_client_event(void *arg)
{
    char *uargv = nullptr;

    clientEvent *pClientEv = nullptr;

    //clientInfo* pClient = nullptr;

    while(true)
    {
        pClientEv = deClientEventQueue();

        if(nullptr == pClientEv)
        {
            continue;
        }

        if(pClientEv->event == EV_ACCEPT)
        {
            proc_client_ev_accept(pClientEv);
        }
        else if(pClientEv->event == EV_RECV)
        {
            proc_client_ev_recv(pClientEv);
        }
        else if(pClientEv->event == EV_SYNC_PRECOMMIT_ACK)
        {
            proc_sync_ev_precommit_ack(pClientEv);
        }
        else if(pClientEv->event == EV_SYNC_PRECOMMIT_ERR)
        {
            proc_sync_ev_precommit_err(pClientEv);
        }
        else if(pClientEv->event == EV_SYNC_COMMIT_SUCCESS)
        {
            proc_sync_ev_commit_success(pClientEv);
        }
        else if(pClientEv->event == EV_SYNC_COMMIT_UNSUCCESS)
        {
            proc_sync_ev_commit_unsuccess(pClientEv);
        }
        else if(pClientEv->event == EV_SYNC_TIMEOUT_MASTER)
        {
            proc_sync_ev_master_timeout(pClientEv);
        }
        else if(pClientEv->event == EV_SYNC_TIMEOUT_SLAVE)
        {
            proc_sync_ev_slave_timeout(pClientEv);
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

    return 0;
}

void *handle_tcp_connection(void *arg)
{
    char *uargv = nullptr;

    start_conn_service();

    return uargv;
}

int create_tcp_connection_thread()
{
    pthread_t threadConn;

    pthread_create(&threadConn, NULL, handle_tcp_connection, NULL);

    return 0;
}



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


    if(create_thread_pool() >= 0)
    {
        if(CONFIG.debugLevel >= DEBUG_LEVEL_D)
            cout << "Client event process thread pool created!" << endl;
    }

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


    if(create_timer_master() >= 0)
    {
        if(CONFIG.debugLevel >= DEBUG_LEVEL_D)
            cout << "Sync master timer created!" << endl;
    }

    if(create_timer_slave() >= 0)
    {
        if(CONFIG.debugLevel >= DEBUG_LEVEL_D)
            cout << "Sync slave timer created!" << endl;
    }


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












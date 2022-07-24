/*
event.cpp

Created by Dianyong Chen, 2022-05-28

CS590 Master Project(BBServer) @ Bishop's University

*/

#include "common.h"
#include "config.h"
#include "queue.h"
#include "connection.h"
#include "list.h"
#include "msg.h"
#include "semaphore.h"
#include "sync.h"

int proc_client_ev_accept(clientEventCB *pClientEv);
int proc_client_ev_recv(clientEventCB* pClientEv);
int proc_sync_ev_precommit_ack(clientEventCB *pClientEv);
int proc_sync_ev_precommit_err(clientEventCB *pClientEv);
int proc_sync_ev_commit_success(clientEventCB *pClientEv);
int proc_sync_ev_commit_unsuccess(clientEventCB *pClientEv);
int proc_sync_ev_master_timeout(clientEventCB *pClientEv);
int proc_sync_ev_slave_timeout(clientEventCB *pClientEv);

void *handle_client_event(void *arg)
{
    char *uargv = nullptr;
    clientEventCB *pClientEv = nullptr;

    while(true){
        pClientEv = deClientEventQueue();

        if(nullptr == pClientEv){
            continue;
        }

        switch(pClientEv->event){
            case EV_ACCEPT:
                proc_client_ev_accept(pClientEv);
                break;

            case EV_RECV:
                proc_client_ev_recv(pClientEv);
                break;

            case EV_SYNC_PRECOMMIT_ACK:
                proc_sync_ev_precommit_ack(pClientEv);
                break;

            case EV_SYNC_PRECOMMIT_ERR:
                proc_sync_ev_precommit_err(pClientEv);
                break;

            case EV_SYNC_COMMIT_SUCCESS:
                proc_sync_ev_commit_success(pClientEv);
                break;

            case EV_SYNC_COMMIT_UNSUCCESS:
                proc_sync_ev_commit_unsuccess(pClientEv);
                break;

            case EV_SYNC_TIMEOUT_MASTER:
                proc_sync_ev_master_timeout(pClientEv);
                break;

            case EV_SYNC_TIMEOUT_SLAVE:
                proc_sync_ev_slave_timeout(pClientEv);
                break;

            default:
                break;
        }

        delete pClientEv;
    }

    return uargv;
}


int send_response_to_client(clientCB* pClient, std::string& response)
{
    int bytesSend;
    response += "\n";

    bytesSend = send(pClient->fd, response.c_str(), response.size(), 0);
    CHECK(bytesSend);

    LOG(DEBUG_LEVEL_D)
        cout << "Send response to "<< pClient->ip <<":" << pClient->port <<":" << response << endl;

    return 0;
}

int proc_client_ev_accept(clientEventCB *pClientEv)
{
    sockaddr_in clientAddr;
    socklen_t clientAddrSize = sizeof(clientAddr);

    int new_fd = accept(pClientEv->fd, (struct sockaddr *)&clientAddr, &clientAddrSize);
    CHECK(new_fd);

    if(new_fd >= 0){
        LOG(DEBUG_LEVEL_D)
            cout << "Client:" << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port) << " connected!" << endl;

        //add new fd to fd set
        conn_add(new_fd);

        //add new client to client list
        clientCB *pClient = new clientCB;
        memset((void *)pClient, 0, sizeof(clientCB));

        pClient->fd = new_fd;
        strcpy(pClient->ip, inet_ntoa(clientAddr.sin_addr));
        pClient->port = ntohs(clientAddr.sin_port);
        pClient->type = pClientEv->type;

        if(pClientEv->type == CLIENT_USER){
            pClient->slaveState = SYNC_IDLE;
        }

        client_list_add(pClient);

        //send greeting
        if(pClientEv->type == CLIENT_USER){
            CHECK(send(new_fd, MSG_GREETING,strlen(MSG_GREETING), 0));
        }
//                else if(cliType == CLIENT_SYNC_MASTER)
//                {
//                    //set state
//                    CHECK(send(new_fd, "hello I am master",strlen("hello I am master"), 0));
//                }
        else if(pClientEv->type == CLIENT_SYNC_SLAVE){
            //set state
            //CHECK(send(new_fd, "hello I am slave",strlen("hello I am slave"), 0));
        }
    }

    return 0;
}

int proc_client_ev_recv(clientEventCB* pClientEv)
{
    int bytesRecved;
    char buf[1024];
    memset(buf, 0, sizeof(buf));

    clientCB *pClient = client_list_find(pClientEv->fd);
    if(!pClient){
        if(SysCfgCB.debugLevel >= DEBUG_LEVEL_APP)
            cout << "client has been deleted! fd:" << pClientEv->fd << endl;

        return -1;
    }

    bytesRecved = recv(pClient->fd, buf, sizeof(buf), 0);
    CHECK(bytesRecved);

    if (bytesRecved == 0) //client disconnected, delete
    {
        conn_del(pClient->fd);

        LOG(DEBUG_LEVEL_D){

            if(pClient->type == CLIENT_USER){
                cout << "Client:" << pClient->ip<<":"<<pClient->port << " disconnected!" << endl;
            }
            else{
                cout << "Sync peer:" << pClient->ip<<":"<<pClient->port << " disconnected!" << endl;
            }

        }
            

        if(pClient->type == CLIENT_SYNC_MASTER)
        {
            //find server node by fd
            syncServerCB *pServer = sync_server_list_find(pClient->fd);
            sync_server_list_set_state(pServer, SYNC_DISCONNECT);
        }

        client_list_del(pClient);
    }
    else if (bytesRecved > 0)
    {
        LOG(DEBUG_LEVEL_D){
            cout << endl << "Recv msg from " << pClient->ip <<":" << pClient->port<<" [" << bytesRecved << " Bytes]:"<< string(buf, 0, bytesRecved);
        }

        string response;
        response.clear();

        if(pClient->type == CLIENT_USER){
            process_client_msg(pClient,buf,bytesRecved, response);
        }
        else if(pClient->type == CLIENT_SYNC_MASTER){
            process_sync_master_msg(pClient,buf,bytesRecved, response);
        }
        else if(pClient->type == CLIENT_SYNC_SLAVE){
            process_sync_slave_msg(pClient,buf,bytesRecved, response);
        }

        if(!response.empty()){
            send_response_to_client(pClient, response);

            // if QUIT cmd, disconnet the client
            string bye = "4.0 BYE";
            if(0 == response.compare(0, bye.size(), bye)){               

                LOG(DEBUG_LEVEL_D)
                    cout << "Disconnect user Client[" << pClient->name << "]:" << pClient->ip << ":" << pClient->port << endl;

                usleep(100000);
                
                conn_del(pClient->fd);
                client_list_del(pClient);
            }
        }
    }

    return 0;
}


int proc_sync_ev_precommit_ack(clientEventCB *pClientEv)
{   
    clientCB* pClientUser;
    while((pClientUser = sync_find_waiting_commit_user_client()) != nullptr)
    {
        sync_send_commit(pClientUser->cmd, pClientUser->msg);

        sync_set_client_state(pClientUser, SYNC_U_WAITING_SAVE);
    }

    return 0;
}

int proc_sync_ev_precommit_err(clientEventCB *pClientEv)
{
    sync_send_abort();

    clientCB* pClientUser;
    while((pClientUser = sync_find_waiting_commit_user_client()) != nullptr)
    {
        string response = "3.2 ERROR WRITE";
        send_response_to_client(pClientUser, response);
        sync_clear_client_cmd(pClientUser);
    }

    return 0;
}

int proc_sync_ev_commit_success(clientEventCB *pClientEv)
{
    clientCB* pClientUser;
    int ret;
    bool isSuccess;
    string response;

    while((pClientUser = sync_find_waiting_save_user_client()) != nullptr){
        if(pClientEv->msgNumber != pClientUser->msgNumber){
            continue;
        }

        response.clear();

        if(pClientUser->cmd == CLIENT_CMD_WRITE){
            ret = save_msg_write(pClientUser->msg);
            if(ret == 0){
                response.append("3.0 WROTE ");
                response.append(pClientUser->msg.substr(0, pClientUser->msg.find("/")));
                isSuccess = true;
            }
            else{
                response.append("3.2 ERROR WRITE");
                isSuccess = false;
            }

        }
        else if(pClientUser->cmd == CLIENT_CMD_REPLACE){
            ret = save_msg_replace(pClientUser->msg);
            if(ret == 0){
                response.append("3.0 WROTE ");
                response.append(pClientUser->msg.substr(0, pClientUser->msg.find("/")));
                isSuccess = true;
            }
            else if(ret == -1){
                response.append("3.1 UNKNOWN ");
                response.append(pClientUser->msg.substr(0, pClientUser->msg.find("/")));
                isSuccess = false;
            }
            else{
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

int proc_sync_ev_commit_unsuccess(clientEventCB *pClientEv)
{
    clientCB* pClientUser;
    string response;

    while((pClientUser = sync_find_waiting_save_user_client()) != nullptr){
        if((!pClientEv->msgNumber.empty()) && (pClientEv->msgNumber != pClientUser->msgNumber)){
            continue;
        }

        response.clear();

        if(pClientUser->cmd == CLIENT_CMD_WRITE){
            response.append("3.2 ERROR WRITE");
        }
        else if(pClientUser->cmd == CLIENT_CMD_REPLACE){
            string str = "6.2 COMMITED UNSUCCESS UNKNOWN";

            if(pClientEv->response.compare(0, str.length(), str) == 0) //"6.2 COMMITED UNSUCCESS UNKNOWN message-number")
            {
                response.append("3.1 UNKNOWN ");
                //response.append(pClientUser->msg.substr(0, pClientUser->msg.find("/")));
                response.append(pClientUser->msgNumber);
            }
            else{
                response.append("3.2 ERROR WRITE");
            }
        }

        send_response_to_client(pClientUser, response);
        sync_clear_client_cmd(pClientUser);
        sync_send_success(false, pClientEv->msgNumber);
    }

    return 0;
}

int proc_sync_ev_master_timeout(clientEventCB *pClientEv)
{
    string msgNum, msg;
    syncState state = sync_get_master_state();

    if(state == SYNC_M_PRECOMMIT_MULTICASTED){
        sync_send_event(EV_SYNC_PRECOMMIT_ERR, msgNum, -1, msg);
    }
    else if(state == SYNC_M_COMMITED){
        sync_send_event(EV_SYNC_COMMIT_UNSUCCESS, msgNum, -1, msg);
    }

    return 0;
}

int proc_sync_ev_slave_timeout(clientEventCB *pClientEv)
{
    clientCB* pClientSlave = client_list_get_first();

    while(pClientSlave){
        if((pClientSlave->type == CLIENT_SYNC_SLAVE) && (pClientSlave->slaveState == SYNC_S_PRECOMMIT_ACK)){
            pClientSlave->slaveTimeout = true;
        }

        pClientSlave = client_list_get_next(pClientSlave);
    }

    sleep(1);

    while((pClientSlave = sync_find_waiting_commit_slave_client()) != nullptr){
        sync_set_slave_state(pClientSlave, SYNC_IDLE);
        pClientSlave->slaveTimeout = false;
    }

    return 0;
}














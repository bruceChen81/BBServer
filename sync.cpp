#include <iostream>
#include <sys/select.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>
#include <time.h>
#include <signal.h>

#include "common.h"
#include "config.h"
#include "list.h"
#include "connection.h"
#include "queue.h"
#include "sync.h"



using std::cout;
using std::endl;
using std::string;


int init_sync_server_list()
{
    std::size_t pos, pos1, pos2;
    bool keepsearching = true;

    string peers = string(CONFIG.peers);

    if(peers.empty())
    {
        cout << "No sync peer configured!" << endl;
        return -1;
    }

    create_sync_server_list();

    pos = 0;

    string serverip, serverport;

    while(keepsearching)
    {
        serverip.clear();
        serverport.clear();

        pos1 = peers.find(":", pos);

        if(pos1 != string::npos)
        {
            pos2 = peers.find(" ", pos1);

            if(pos2 != string::npos)
            {
                serverip += peers.substr(pos, pos1-pos);
                serverport += peers.substr(pos1+1, pos2-pos1-1);

                pos = pos2+(long)1; //search next : from pos2
            }
            else
            {
                //the last peer
                serverip += peers.substr(pos, pos1-pos);
                serverport += peers.substr(pos1+(long)1, peers.length()-pos1);
                keepsearching = false;
            }

            if(CONFIG.debugLevel >= DEBUG_LEVEL_APP)
                cout << "init_sync_server_list find server:" << serverip <<":"<<serverport<< endl;

            //add to server list
            syncServerInfo *pServer = new syncServerInfo;

            pServer->fd = -1;

            //if config hostname, use gethostbyname to convert to ip ???
            pServer->ip += serverip;

            pServer->port = atoi(serverport.c_str());
            pServer->state = SYNC_DISCONNECT;

            sync_server_list_add(pServer);
        }
        else
        {
            keepsearching = false;
        }
    }

    return 0;
}

int init_sync_server_connection()
{
    syncServerInfo *pServer;
    int fd = -1;
    int initState = 0;

    if(sync_server_list_empty())
    {
        cout << "sync_server_list_empty"<<endl;
        return -1;
    }

    pServer = sync_server_list_get_first();

    while(pServer != nullptr)
    {
        //check server state
        if(pServer->state == SYNC_DISCONNECT)
        {
            fd = sync_connect_to_server(pServer->ip, pServer->port);

            if( fd >= 0)
            {
                sync_server_list_set_fd(pServer,fd);
                sync_server_list_set_state(pServer, SYNC_IDLE);
            }
            else
            {
                //return -1;
                initState = -1;
            }
        }

        pServer = sync_server_list_get_next(pServer);
    }

    return initState;
}

int sync_send_precommit()
{
    //send precommit to sync server list
    string msg = string("PRECOMMIT");
    msg += "\n";

    syncServerInfo *pServer = sync_server_list_get_first();

    while(pServer != nullptr)
    {
        //check server state
        if(pServer->state == SYNC_IDLE)
        {
            CHECK(send(pServer->fd, msg.c_str(), msg.size(),0));

            sync_server_list_set_state(pServer, SYNC_M_PRECOMMIT_MULTICASTED);

            if(CONFIG.debugLevel >= DEBUG_LEVEL_D)
                cout << "send sync msg to " << pServer->ip <<":"<<pServer->port<<": " <<msg<< endl;

        }
        else
        {
            cout << "sync server state [" << pServer->state << "] error! "<<pServer->ip<<":"<<pServer->port<<endl;
        }

        pServer = sync_server_list_get_next(pServer);
    }

    //start timer, when timeout check state

    return 0;
}


int sync_send_commit(string& msgbody)
{
    //send commit to sync server list
    //COMMIT WRITE(REPLACE)/message-number/user/msgbody

    string msg = string("COMMIT");
    msg += " ";
    msg += msgbody;
    msg += "\n";

    syncServerInfo *pServer = sync_server_list_get_first();

    while(pServer != nullptr)
    {
        //check server state
        if(pServer->state == SYNC_M_PRECOMMITED)
        {
            CHECK(send(pServer->fd, msg.c_str(), msg.size(),0));

            sync_server_list_set_state(pServer, SYNC_M_COMMITED);

            if(CONFIG.debugLevel >= DEBUG_LEVEL_D)
                cout << "send sync msg to " << pServer->ip <<":"<<pServer->port<<": " <<msg<< endl;

        }
        else
        {
            cout << "sync server state [" << pServer->state << "] error! "<<pServer->ip<<":"<<pServer->port<<endl;
        }

        pServer = sync_server_list_get_next(pServer);
    }

    //start timer, when timeout check state

    return 0;
}

int sync_send_success(bool isSuccessful)
{

    string msg;

    if(isSuccessful)
    {
        msg += "SUCCESS";
    }
    else
    {
        msg += "NOT SUCCESSFUL";
    }

    msg += "\n";

    syncServerInfo *pServer = sync_server_list_get_first();

    while(pServer != nullptr)
    {
        //check server state
        if(pServer->state == SYNC_M_OPERATION_PERFORMED)
        {
            CHECK(send(pServer->fd, msg.c_str(), msg.size(),0));

            sync_server_list_set_state(pServer, SYNC_IDLE);

            if(CONFIG.debugLevel >= DEBUG_LEVEL_D)
                cout << "send sync msg to " << pServer->ip <<":"<<pServer->port<<": " <<msg<< endl;

        }
        else
        {
            cout << "sync server state [" << pServer->state << "] error! "<<pServer->ip<<":"<<pServer->port<<endl;
        }

        pServer = sync_server_list_get_next(pServer);
    }

    //start timer, when timeout check state

    return 0;
}


int sync_connect_to_server(string& ip, unsigned int port)
{
    int sockfd, ret;
    sockaddr_in servaddr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    CHECK_EXIT(sockfd);

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(ip.c_str());
    servaddr.sin_port = htons(port);

    ret = connect(sockfd, (sockaddr *)&servaddr, sizeof(servaddr));

    CHECK(ret);

    if(ret == 0)
    {
        cout << "connected to sync server:" <<ip<<":"<<port<<endl;
    }
    else
    {
        cout << "failed to connect to sync server:" <<ip<<":"<<port<<endl;

        return -1;
    }

    //connected
    conn_add(sockfd);


    //add new client to client list
    clientInfo *pClient = new clientInfo;
    memset((void *)pClient, 0, sizeof(clientInfo));

    pClient->fd = sockfd;
    strcpy(pClient->ip, ip.c_str());
    pClient->port = port;
    pClient->type = CLIENT_SYNC_MASTER;

    client_list_add(pClient);


    return sockfd;
}

bool sync_check_server_state(syncState state)
{
    syncServerInfo *pServer = sync_server_list_get_first();

    while(pServer != nullptr)
    {
        //check server state
        if(pServer->state != state)
        {
            return false;
        }

        pServer = sync_server_list_get_next(pServer);
    }

    return true;
}


void *handle_data_sync_event(void *arg)
{
    char *uargv = nullptr;

    dataSyncEvent *pDataSyncEv;
    //syncServerInfo *pServer;

    int ret;

    while(true)
    {
        pDataSyncEv = nullptr;

        pDataSyncEv = deDataSyncEventQueue();

        if(CONFIG.debugLevel >= DEBUG_LEVEL_APP)
            cout << "handle_data_sync_event: "<<pDataSyncEv->msg << endl;

        if(nullptr == pDataSyncEv)
        {
            continue;
        }

        ret = init_sync_server_connection();

        if(ret < 0)
        {
            //abort sync if one of servers failed
            cout << "init_sync_server_connection failed!" <<endl;

            //break;
        }

        //send precommit to sync server list

        syncServerInfo *pServer = sync_server_list_get_first();

        while(pServer != nullptr)
        {
            //check server state
            if(pServer->state == SYNC_IDLE)
            {
                string msg = string("PRECOMMIT");

                CHECK(send(pServer->fd,msg.c_str(),msg.size(),0));

                sync_server_list_set_state(pServer, SYNC_M_PRECOMMIT_MULTICASTED);

            }
            else
            {
                cout << "sync server state [" << pServer->state << "] error! "<<pServer->ip<<":"<<pServer->port<<endl;
            }

            pServer = sync_server_list_get_next(pServer);
        }

        //start timer, when timeout check state










        delete pDataSyncEv;
    }

    return uargv;
}






void handler(int sig)
{
    cout << "caught signal: " << sig <<endl;

    //signal(sig, SIG_IGN);

}



timer_t start_timer(int t_second)
{
    timer_t timerid;
    struct sigevent se;
    struct itimerspec timerspec;
    sigset_t mask;
    struct sigaction sa;

    //sa.sa_flags = SA_SIGINFO;
    sa.sa_flags = SA_RESTART;
    //sa.sa_sigaction = handler;
    sa.sa_handler = handler;

    sigemptyset(&sa.sa_mask);
    CHECK(sigaction(SIGRTMIN, &sa, NULL));

    //block timer signal
    sigemptyset(&mask);
    sigaddset(&mask, SIGRTMIN);

    CHECK(sigprocmask(SIG_SETMASK, &mask, NULL));

    //create timer
    se.sigev_notify = SIGEV_SIGNAL;
    se.sigev_signo = SIGRTMIN;
    se.sigev_value.sival_ptr = &timerid;


    CHECK(timer_create(CLOCK_REALTIME, &se, &timerid));

    //start timer
    timerspec.it_interval.tv_sec = t_second;
    timerspec.it_interval.tv_nsec = 0;
    timerspec.it_value.tv_sec = 1; //delay
    timerspec.it_value.tv_nsec = 0;

    CHECK(timer_settime(timerid, 0, &timerspec, NULL));

    //unlock thmer signal
    CHECK(sigprocmask(SIG_UNBLOCK, &mask, NULL));



    return timerid;
}












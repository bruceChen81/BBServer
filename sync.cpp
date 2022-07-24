/*
bootup.cpp

Created by Dianyong Chen, 2022-06-18

CS590 Master Project(BBServer) @ Bishop's University

*/

#include "common.h"
#include "config.h"
#include "list.h"
#include "connection.h"
#include "queue.h"
#include "sync.h"


int init_sync_server_list()
{
    std::size_t pos, pos1, pos2;
    bool keepsearching = true;

    string peers = string(SysCfgCB.peers);

    if(peers.empty()){
        cout << "No sync peer configured!" << endl;
        return -1;
    }

    create_sync_server_list();

    pos = 0;
    string server, serverIp, serverport;

    while(keepsearching){
        server.clear();
        serverport.clear();
        pos1 = peers.find(":", pos);

        if(pos1 != string::npos){
            pos2 = peers.find(" ", pos1);

            if(pos2 != string::npos){
                server += peers.substr(pos, pos1-pos);
                serverport += peers.substr(pos1+1, pos2-pos1-1);

                pos = pos2+(long)1; //search next : from pos2
            }
            else{
                //the last peer
                server += peers.substr(pos, pos1-pos);
                serverport += peers.substr(pos1+(long)1, peers.length()-pos1);
                keepsearching = false;
            }

            LOG(DEBUG_LEVEL_APP)
                cout << "init_sync_server_list find server:" << server <<":"<<serverport<< endl;

            //add to server list
            syncServerCB *pServer = new syncServerCB;

            pServer->fd = -1;

            //if config hostname, use gethostbyname to convert to ip
            struct hostent *host = gethostbyname(server.c_str());

            serverIp.clear();

            if(host){
                serverIp += inet_ntoa(*((struct in_addr**)host->h_addr_list)[0]);
            }
            else{
                serverIp += server;
            }

            pServer->ip += serverIp;

            pServer->port = atoi(serverport.c_str());
            pServer->state = SYNC_DISCONNECT;

            sync_server_list_add(pServer);
        }
        else{
            keepsearching = false;
        }
    }

    return 0;
}

int init_sync_server_connection()
{
    syncServerCB *pServer;
    int fd = -1;
    int initState = 0;

    if(sync_server_list_empty()){
        LOG(DEBUG_LEVEL_APP)
                cout << "sync server list is empty"<<endl;

        return -1;
    }

    pServer = sync_server_list_get_first();

    while(pServer != nullptr){
        //check server state
        if(pServer->state == SYNC_DISCONNECT){
            fd = sync_connect_to_server(pServer->ip, pServer->port);

            if( fd >= 0){
                sync_server_list_set_fd(pServer,fd);
                sync_server_list_set_state(pServer, SYNC_IDLE);
            }
            else{
                //return -1;
                initState = -1;
            }
        }
        pServer = sync_server_list_get_next(pServer);
    }

    if(sync_check_server_state(SYNC_IDLE)){
        sync_set_master_state(SYNC_IDLE);
    }

    return initState;
}

int sync_send_precommit()
{
    //send precommit to sync server list
    string msg = string("PRECOMMIT");
    msg += "\n";

    syncServerCB *pServer = sync_server_list_get_first();

    while(pServer != nullptr){
        int ret = send(pServer->fd, msg.c_str(), msg.size(),0);
        CHECK(ret);

        if(ret >= 0){
            sync_server_list_set_state(pServer, SYNC_M_PRECOMMIT_MULTICASTED);

            LOG(DEBUG_LEVEL_D)
                cout << "Send sync msg to " << pServer->ip <<":"<<pServer->port<<":" <<msg<< endl;
        }
        else{
            return -1;
        }

        pServer = sync_server_list_get_next(pServer);
    }

    if(sync_check_server_state(SYNC_M_PRECOMMIT_MULTICASTED)){
        sync_set_master_state(SYNC_M_PRECOMMIT_MULTICASTED);
    }

    return 0;
}

int sync_send_abort()
{
    //send abort to sync server list
    string msg = string("ABORT");
    msg += "\n";

    syncServerCB *pServer = sync_server_list_get_first();

    while(pServer != nullptr){
        int ret = send(pServer->fd, msg.c_str(), msg.size(),0);
        CHECK(ret);

        if(ret >= 0){
            sync_server_list_set_state(pServer, SYNC_IDLE);

            LOG(DEBUG_LEVEL_D)
                cout << "Send sync msg to " << pServer->ip <<":"<<pServer->port<<":" <<msg<< endl;
        }
        else{
            return -1;
        }

        pServer = sync_server_list_get_next(pServer);
    }

    if(sync_check_server_state(SYNC_IDLE)){
        sync_set_master_state(SYNC_IDLE);
    }

    return 0;
}

//flag: 1-write, 2-replace
int sync_send_commit(clientCmdType type, string& msgbody)
{
    //send commit to sync server list
    //COMMIT WRITE(REPLACE)/message-number/user/msgbody

    string msg = string("COMMIT");

    msg += " ";
    if(type == CLIENT_CMD_WRITE){
        msg += "WRITE";
    }
    else if(type == CLIENT_CMD_REPLACE){
        msg += "REPLACE";
    }

    msg += " ";

    msg += msgbody;
    msg += "\n";

    syncServerCB *pServer = sync_server_list_get_first();

    while(pServer != nullptr){
        int ret = send(pServer->fd, msg.c_str(), msg.size(),0);
        CHECK(ret);

        if(ret >= 0){
            sync_server_list_set_state(pServer, SYNC_M_COMMITED);

            LOG(DEBUG_LEVEL_D)
                cout << "Send sync msg to " << pServer->ip <<":"<<pServer->port<<":" <<msg<< endl;
        }
        else{
            return -1;
        }

        pServer = sync_server_list_get_next(pServer);
    }

    if(sync_check_server_state(SYNC_M_COMMITED)){
        sync_set_master_state(SYNC_M_COMMITED);
    }

    return 0;
}

int sync_send_success(bool isSuccessful, string& msgNumber)
{
    string msg;

    if(isSuccessful){
        msg += "END SUCCESS";
    }
    else{
        msg += "END UNSUCCESS";
    }

    msg += " ";
    msg += msgNumber;

    msg += "\n";

    syncServerCB *pServer = sync_server_list_get_first();

    while(pServer != nullptr){
        int ret = send(pServer->fd, msg.c_str(), msg.size(),0);
        CHECK(ret);

        if(ret >= 0){
            sync_server_list_set_state(pServer, SYNC_IDLE);

            LOG(DEBUG_LEVEL_D)
                cout << "Send sync msg to " << pServer->ip <<":"<<pServer->port<<":" <<msg<< endl;
        }
        else{
            return -1;
        }

        pServer = sync_server_list_get_next(pServer);
    }

    if(sync_check_server_state(SYNC_IDLE)){
        sync_set_master_state(SYNC_IDLE);
    }

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

    //CHECK(ret);

    if(ret == 0){
        LOG(DEBUG_LEVEL_D)
            cout << "Connected to sync server peer:" <<ip<<":"<<port<<endl;
    }
    else{
        LOG(DEBUG_LEVEL_D)
            cout << "ERROR: Failed to connect to sync server peer:" <<ip<<":"<<port<<endl;

        return -1;
    }

    //connected
    conn_add(sockfd);

    //add new client to client list
    clientCB *pClient = new clientCB;
    memset((void *)pClient, 0, sizeof(clientCB));

    pClient->fd = sockfd;
    strcpy(pClient->ip, ip.c_str());
    pClient->port = port;
    pClient->type = CLIENT_SYNC_MASTER;

    client_list_add(pClient);

    return sockfd;
}


bool sync_check_server_state(syncState state)
{
    syncServerCB *pServer = sync_server_list_get_first();

    while(pServer != nullptr){
        //check server state
        if(pServer->state != state){
            return false;
        }

        pServer = sync_server_list_get_next(pServer);
    }

    return true;
}

bool sync_check_service_enable()
{
    return SysCfgCB.syncEnalbe;
}


int sync_send_event(clientEvType type, std::string& msgNumber, int fd, std::string& msg)
{
    clientEventCB *pSyncEv = new clientEventCB;

    pSyncEv->event = type;
    pSyncEv->msgNumber = msgNumber;
    pSyncEv->fd = fd;
    pSyncEv->response = msg;

    enClientEventQueue(pSyncEv);

    return 0;
}

int sync_send_event_timeout(clientEvType type)
{
    clientEventCB *pSyncEv = new clientEventCB;

    pSyncEv->event = type;

    enClientEventQueue(pSyncEv);

    return 0;
}



timer_t timerid_master, timerid_slave;

void handle_master_timeout(int sig)
{
    LOG(DEBUG_LEVEL_D)
        cout <<"Sync master state timeout!"<<endl;  

    sync_send_event_timeout(EV_SYNC_TIMEOUT_MASTER);

    return;
}

void handle_slave_timeout(int sig)
{

    //signal(sig, SIG_INT);
    //signal(SIG_INT, handler);
    LOG(DEBUG_LEVEL_D)
            cout <<"Sync slave state timeout!"<<endl;   

    sync_send_event_timeout(EV_SYNC_TIMEOUT_SLAVE);

    return;
}



int create_timer_master()
{
    //timer_t timerid_master;
    struct sigevent se;
    //struct itimerspec timerspec;
    sigset_t mask;
    struct sigaction sa;

    //sa.sa_flags = SA_SIGINFO;
    sa.sa_flags = SA_RESTART;
    //sa.sa_sigaction = handler;
    sa.sa_handler = handle_master_timeout;

    sigemptyset(&sa.sa_mask);
    CHECK(sigaction(SIGRTMIN, &sa, NULL));

    //block timer signal
    sigemptyset(&mask);
    sigaddset(&mask, SIGRTMIN);

    CHECK(sigprocmask(SIG_SETMASK, &mask, NULL));

    //create timer
    se.sigev_notify = SIGEV_SIGNAL;
    se.sigev_signo = SIGRTMIN;
    se.sigev_value.sival_ptr = &timerid_master;


    CHECK(timer_create(CLOCK_REALTIME, &se, &timerid_master));

    //start timer
//    timerspec.it_interval.tv_sec = 3;
//    timerspec.it_interval.tv_nsec = 0;
//    timerspec.it_value.tv_sec = 0; //delay
//    timerspec.it_value.tv_nsec = 10;

    //CHECK(timer_settime(timerid_master, 0, &timerspec, NULL));

    //unlock thmer signal
    CHECK(sigprocmask(SIG_UNBLOCK, &mask, NULL));

    return 0;
}

int create_timer_slave()
{
    //timer_t timerid_master;
    struct sigevent se;
    //struct itimerspec timerspec;
    sigset_t mask;
    struct sigaction sa;

    //sa.sa_flags = SA_SIGINFO;
    sa.sa_flags = SA_RESTART;
    //sa.sa_sigaction = handler;
    sa.sa_handler = handle_slave_timeout;

    sigemptyset(&sa.sa_mask);
    CHECK(sigaction(SIGRTMIN+1, &sa, NULL));

    //block timer signal
    sigemptyset(&mask);
    sigaddset(&mask, SIGRTMIN+1);

    CHECK(sigprocmask(SIG_SETMASK, &mask, NULL));

    //create timer
    se.sigev_notify = SIGEV_SIGNAL;
    se.sigev_signo = SIGRTMIN+1;
    se.sigev_value.sival_ptr = &timerid_slave;


    CHECK(timer_create(CLOCK_REALTIME, &se, &timerid_slave));

    //start timer
//    timerspec.it_interval.tv_sec = 3;
//    timerspec.it_interval.tv_nsec = 0;
//    timerspec.it_value.tv_sec = 0; //delay
//    timerspec.it_value.tv_nsec = 10;

    //CHECK(timer_settime(timerid_master, 0, &timerspec, NULL));

    //unlock thmer signal
    CHECK(sigprocmask(SIG_UNBLOCK, &mask, NULL));

    return 0;
}


int start_timer_master(int sec)
{
    struct itimerspec timerspec;

    timerspec.it_interval.tv_sec = 0;
    timerspec.it_interval.tv_nsec = 0;
    timerspec.it_value.tv_sec = sec; //delay
    timerspec.it_value.tv_nsec = 0;

    CHECK(timer_settime(timerid_master, 0, &timerspec, NULL));

    LOG(DEBUG_LEVEL_D)
        cout << "Start sync master state timer:" <<sec<<"s"<<endl;

    return 0;
}

int start_timer_slave(int sec)
{
    struct itimerspec timerspec;

    timerspec.it_interval.tv_sec = 0; //when 0, timer expire once
    timerspec.it_interval.tv_nsec = 0;
    timerspec.it_value.tv_sec = sec; // first expire time
    timerspec.it_value.tv_nsec = 0;

    CHECK(timer_settime(timerid_slave, 0, &timerspec, NULL));

    LOG(DEBUG_LEVEL_D)
        cout << "Start sync slave state timer:" <<sec<<"s"<<endl;

    return 0;
}

int stop_timer_master()
{
    struct itimerspec timerspec;

    timerspec.it_interval.tv_sec = 0;
    timerspec.it_interval.tv_nsec = 0;
    timerspec.it_value.tv_sec = 0; //delay
    timerspec.it_value.tv_nsec = 0;

    CHECK(timer_settime(timerid_master, 0, &timerspec, NULL));

    LOG(DEBUG_LEVEL_D)
        cout << "Stop sync master state timer" << endl;

    return 0;
}

int stop_timer_slave()
{
    struct itimerspec timerspec;

    timerspec.it_interval.tv_sec = 0;
    timerspec.it_interval.tv_nsec = 0;
    timerspec.it_value.tv_sec = 0; //delay
    timerspec.it_value.tv_nsec = 0;

    CHECK(timer_settime(timerid_slave, 0, &timerspec, NULL));

    LOG(DEBUG_LEVEL_D)
        cout << "Stop sync slave state timer" << endl;

    return 0;
}

int delete_timer_master()
{
    timer_delete(timerid_master);

    LOG(DEBUG_LEVEL_D)
        cout << "Sync master state timer deleted!" << endl;

    return 0;
}

int delete_timer_slave()
{
    timer_delete(timerid_slave);

    LOG(DEBUG_LEVEL_D)
        cout << "Sync slave state timer deleted!" << endl;

    return 0;
}

void destroy_timer()
{
    delete_timer_master();

    delete_timer_slave();

    return;
}









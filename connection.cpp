/*
connection.cpp

Created by Dianyong Chen, 2022-05-26

CS590 Master Project(BBServer) @ Bishop's University

*/

#include "common.h"

#include "config.h"
#include "queue.h"
#include "connection.h"
#include "list.h"
#include "sync.h"



int connections[MAX_CONN];

fd_set read_fd_set;

pthread_mutex_t clientConnLock;


int create_tcp_server_sock(unsigned int port)
{
    int fd = -1;
    fd = socket(AF_INET, SOCK_STREAM, 0);
    CHECK_EXIT(fd);

    int optval = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));

    //bind
    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(port);
    hint.sin_addr.s_addr = INADDR_ANY;

    CHECK_EXIT(bind(fd,(sockaddr *)&hint, sizeof(hint)));

    //listen(listeningSock, SOMAXCONN);
    CHECK_EXIT(listen(fd, MAX_CONN));

    return fd;
}

int start_conn_service()
{
    int server_fd, sync_server_fd, ret;

    struct timeval tv;
    tv.tv_sec = 1; //1 second
    tv.tv_usec = 0;

    conn_init();

    //create server listening socket
    server_fd = create_tcp_server_sock(SysCfgCB.bbPort);
    CHECK_EXIT(server_fd);

    conn_add(server_fd);

    //create sync server listening socket
    //start_sync_server();
    sync_server_fd = create_tcp_server_sock(SysCfgCB.syncPort);
    CHECK_EXIT(sync_server_fd);

    conn_add(sync_server_fd);


    LOG(DEBUG_LEVEL_NONE){
        cout << "Sync server socket created, listening ......" << endl;
        cout << "Bulletin Board Server startup complete, waiting for client --------->" << endl<<endl;
    }

    while(true){
        FD_ZERO(&read_fd_set);
        conn_set_fdset(&read_fd_set);

        //Invoke select()
        ret = select(FD_SETSIZE, &read_fd_set, NULL, NULL, &tv);

        //CHECK(ret);

        if (ret >= 0){
            if(FD_ISSET(server_fd, &read_fd_set))//check if the fd with event is the sever fd, accept new connection
            {
                //en client event queue, to accept
                clientEventCB *pClientEv = new clientEventCB;

                pClientEv->event = EV_ACCEPT;
                pClientEv->fd = server_fd;
                pClientEv->type = CLIENT_USER;

                enClientEventQueue(pClientEv);
            }
            else if(FD_ISSET(sync_server_fd, &read_fd_set))//check if the fd with event is the sync sever fd, accept new connection
            {
                //en client event queue, to accept
                clientEventCB *pClientEv = new clientEventCB;

                pClientEv->event = EV_ACCEPT;
                pClientEv->fd = sync_server_fd;
                pClientEv->type = CLIENT_SYNC_SLAVE;

                enClientEventQueue(pClientEv);
            }
            else
            {
                //check if the fd with event is a non-server fd, then reveive data
                conn_check_fd_set(&read_fd_set);
            }
        }

        usleep(1000);//1ms

    }//while()

    // close all sockets
    //conn_close_all();

    return SUCCESS;
}

void conn_check_fd_set(fd_set *pFdSet)
{    
    if(!pFdSet){
        return;
    }
    unsigned int i;

    // do not check server fd:connections[0] and [1]
    for(i=2;i<MAX_CONN;i++){
        if((connections[i] > 0) && FD_ISSET(connections[i], pFdSet)){
            clientEventCB *pClientEv = new clientEventCB;   

            pClientEv->event = EV_RECV;
            pClientEv->fd = connections[i];

            enClientEventQueue(pClientEv);
        }
    }

    return;
}


void conn_init()
{
    unsigned int i;

    if (pthread_mutex_init(&clientConnLock, NULL) != 0){
        std::cout << "Init clientConnLock failed!" << std::endl;
    }

    pthread_mutex_lock(&clientConnLock);
    for (i=0;i<MAX_CONN;i++){
        connections[i] = -1;
    }
    pthread_mutex_unlock(&clientConnLock);

    FD_ZERO(&read_fd_set);

    return;
}

void conn_set_fdset(fd_set *pFdSet)
{
    unsigned int i;

    if(!pFdSet){
        return;
    }

    for(i=0;i<MAX_CONN;i++){
        if(connections[i] >= 0){
            FD_SET(connections[i], pFdSet);
        }
    }

    return;
}

bool conn_is_exist(int fd)
{
    if(fd < 0){
        return false;
    }

    for (int i=0;i<MAX_CONN;i++){
        if(connections[i] == fd){
            return true;
        }
    }

    return false;
}

void conn_add(int fd)
{
    unsigned int i;

    if(conn_is_exist(fd)){
        return;
    }

    pthread_mutex_lock(&clientConnLock);
    for (i=0;i<MAX_CONN;i++){
        if(connections[i] < 0){
            connections[i] = fd;

            break;
        }
    }
    pthread_mutex_unlock(&clientConnLock);

    FD_SET(fd, &read_fd_set);

    LOG(DEBUG_LEVEL_CONN)
        cout << "connection add fd:" <<fd<<endl;

    return;
}

void conn_del(int fd)
{
    unsigned int i;

    if(!conn_is_exist(fd)){
        return;
    }

    CHECK(close(fd));

    FD_CLR(fd, &read_fd_set);

    pthread_mutex_lock(&clientConnLock);
    for (i=0;i<MAX_CONN;i++){
        if(connections[i] == fd){
            connections[i] = -1;
            //break;
        }
    }
    pthread_mutex_unlock(&clientConnLock);

    LOG(DEBUG_LEVEL_CONN)
        cout << "connection del fd:" <<fd<<endl;

    return;
}

void conn_close_all()
{
    unsigned int i;
    FD_ZERO(&read_fd_set);

    pthread_mutex_lock(&clientConnLock);
    for (i=0;i<MAX_CONN;i++){
        if(connections[i] > 0){
            CHECK(close(connections[i]));
            connections[i] = -1;
        }
    }
    pthread_mutex_unlock(&clientConnLock);

    return;
}

void destroy_connection()
{
    conn_close_all();
    pthread_mutex_destroy(&clientConnLock);

    LOG(DEBUG_LEVEL_D)
        cout << "All server and client connections closed!" << endl;

    return;
}

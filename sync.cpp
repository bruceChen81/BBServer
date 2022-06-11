#include <iostream>
#include <sys/select.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>

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


int sync_connect_to_server(string& ip, unsigned int port)
{
    int sockfd, connfd, ret;
    sockaddr_in servaddr, clientaddr;

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
    pClient->type = CLIENT_SYNC;

    client_list_add(pClient);

    return sockfd;
}


void *handle_data_sync_event(void *arg)
{
    char *uargv = nullptr;

    dataSyncEvent *pDataSyncEv;
    syncServerInfo *pServer;

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


        if(sync_server_list_empty())
        {
            cout << "sync_server_list_empty"<<endl;
            break;
        }

        pServer = sync_server_list_get_first();

        while(pServer != nullptr)
        {
            //check server state
            if(pServer->state == SYNC_DISCONNECT)
            {
                if(sync_connect_to_server(pServer->ip, pServer->port) >= 0)
                {
                    sync_server_list_set_state(pServer, SYNC_IDLE);
                }
                else
                {
                    //connect error, abort
                }
            }

            pServer = sync_server_list_get_next(pServer);
        }




        if(pDataSyncEv->event == DATA_SYNC_WRITE)
        {


        }
        else if(pDataSyncEv->event == DATA_SYNC_REPLACE)
        {


        }


        delete pDataSyncEv;
    }

    return uargv;
}















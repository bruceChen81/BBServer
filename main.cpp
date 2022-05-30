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

using namespace std;




void * handle_client(void * arg)
{
    char *argv = nullptr;

    int fd, bytesRecved = -1;

    sockaddr_in clientAddr;

    client *pClient = nullptr;

    char buf[4096];

    memset(buf, 0, sizeof(buf));

    while(true)
    {
        pClient = deClientQueue();

        if(nullptr == pClient)
        {
            continue;
        }

        cout << "deQueue Client fd = " << pClient->fd << endl;

        fd = pClient->fd;

        clientAddr = pClient->clientAddr;

        delete pClient;

        bytesRecved = recv(fd, buf, sizeof(buf), 0);

        if (bytesRecved == -1)
        {
            cout << "Error in recv()" << endl;
            continue;
        }
        else if (bytesRecved == 0)
        {
            cout << "Client:" << inet_ntoa(clientAddr.sin_addr) << " disconnected!" << endl;

            conn_del(fd);

            continue;
        }
        else if (bytesRecved > 0)
        {
            cout << "Msg recved from " << inet_ntoa(clientAddr.sin_addr) <<":" << ntohs(clientAddr.sin_port)<<" [" << bytesRecved << " Bytes]: "
                << string(buf, 0, bytesRecved)<< endl;

            //echo
            send(fd, buf, bytesRecved+1, 0);
        }
    }

    return argv;
}

int create_thread_pool()
{
    pthread_t threadPool[CONFIG.thMax];

    for(unsigned int i=0;i<CONFIG.thMax;i++)
    {
        pthread_create(&threadPool[i], NULL, handle_client, NULL);
    }

    cout << "client thread pool created!" << endl;

    return SUCCESS;
}


int main(int argc, char *argv[])
{
    load_config();

    int option;

    int bflag = 0, Tflag = 0, pflag = 0, sflag = 0,fflag = 0, dflag = 0;

    while(true)
    {



    while ((option = getopt(argc,argv, "bTpsfd:")) != -1)
    {
        switch(option)
        {
            case 'b':
                bflag = 1;
                cout<<optarg<<endl;
                break;
            case 'T':
                Tflag = 1;
                cout<<optarg<<endl;
                break;
            case 'p':
                pflag = 1;
                cout<<optarg<<endl;
                break;

            case 's':
                sflag = 1;
                cout<<optarg<<endl;
                break;

            case 'f':
                fflag = 1;
                cout<<optarg<<endl;
                break;

            case 'd':
                dflag = 1;
                cout<<optarg<<endl;
                break;

            default:
                //bflag = 1;
                cout<<"Invalid option!"<<endl;
                break;
        }
    }

    }

//    create_client_queue();
//
//    create_thread_pool();
//
//    start_comm_service();


    return 0;
}

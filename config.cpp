#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include "common.h"
#include "config.h"

using namespace std;

sysCfg CONFIG;

int print_config()
{
    if (CONFIG.debug)
        {
            cout << "THMAX: " << CONFIG.thMax << endl << "BBPORT: " << CONFIG.bbPort << endl << "SYNCPORT: " << CONFIG.syncPort << endl;
            cout << "BBFILE: " << CONFIG.bbFile << endl << "PEERS: " << CONFIG.peers << endl;
            cout << "DAEMON: " << CONFIG.daemon << endl << "DEBUG: " << CONFIG.debug << endl;
        }

    return 0;
}

int load_config()
{
    int fd;
    unsigned int tmax, bp, sp;
    unsigned int parameter;

    ssize_t bytesRead;

    char buf[256];
    char para[128];

    memset(buf, 0, sizeof(buf));

    //CHECK(fd = open("bbserv1111.conf", O_RDONLY));

    fd = open("bbserv.conf", O_RDONLY);

    CHECK(fd);

    bytesRead = read(fd, buf, sizeof(buf));

    memset((void *) &CONFIG, 0, sizeof(CONFIG));

    //THMAX
    CONFIG.thMax = 20;

    memset(para, 0, sizeof(para));

    if(SUCCESS == getParaFromBuf((char *)buf, (char *)para,(char *)"THMAX="))
    {
        tmax = atoi(para);

        if(0<tmax && tmax<MAX_THREAD_POOL)
        {
            CONFIG.thMax = tmax;
        }
    }

    //BBPORT
    CONFIG.bbPort = 9000;

    memset(para, 0, sizeof(para));

    if(SUCCESS == getParaFromBuf((char *)buf, (char *)para,(char *)"BBPORT="))
    {
        bp = atoi(para);

        if(0<bp && bp<65535)
        {
            CONFIG.bbPort = bp;
        }
    }


    //SYNCPORT
    CONFIG.syncPort = 10000;

    memset(para, 0, sizeof(para));

    if(SUCCESS == getParaFromBuf((char *)buf, (char *)para,(char *)"SYNCPORT="))
    {
        sp = atoi(para);

        if(0<sp && sp<65535)
        {
            CONFIG.syncPort = sp;
        }
    }

    //BBFILE --- mandatory
    strcpy(CONFIG.bbFile, "");

    memset(para, 0, sizeof(para));

    if(SUCCESS == getParaFromBuf((char *)buf, (char *)para,(char *)"BBFILE="))
    {
        strcpy(CONFIG.bbFile, para);
    }

    //PEERS
    strcpy(CONFIG.peers, "");

    memset(para, 0, sizeof(para));

    if(SUCCESS == getParaFromBuf((char *)buf, (char *)para,(char *)"PEERS="))
    {
        strcpy(CONFIG.peers, para);
    }

    //DAEMON
    CONFIG.daemon = true;

    memset(para, 0, sizeof(para));

    if(SUCCESS != getParaFromBuf((char *)buf, (char *)para,(char *)"DAEMON="))
    {
        CONFIG.daemon = false;
    }

    //DEBUG
    CONFIG.debug = false;

    memset(para, 0, sizeof(para));

    if(SUCCESS == getParaFromBuf((char *)buf, (char *)para,(char *)"DEBUG="))
    {
        if(*para == 'D')
        {
            CONFIG.debug = true;
        }
    }

    //CONFIG.maxConnections = CONFIG.thMax;

    cout << "load config file success!" << endl;

    print_config();

    return SUCCESS;
}

int load_option(int argc, char **argv)
{
    int option;
    unsigned int tmax, bp, sp;

    bool isOptionSet = false;

//    extern char *optarg;
//    extern int optind, optopt;

    while ((option = getopt(argc,argv, "b:t:T:p:s:fd")) != -1)
    {
        switch(option)
        {
            case 'b':
                strcpy(CONFIG.bbFile,optarg);
                isOptionSet = true;
                cout<<CONFIG.bbFile<<endl;
                break;

            case 'T':
            case 't':
                tmax = atoi(optarg);
                if(0<tmax && tmax<MAX_THREAD_POOL)
                {
                    CONFIG.thMax = tmax;
                    isOptionSet = true;
                    cout<<CONFIG.thMax<<endl;
                }
                break;

            case 'p':
                bp = atoi(optarg);
                if(0<bp && bp<65535)
                {
                    CONFIG.bbPort = bp;
                    isOptionSet = true;
                    cout<<CONFIG.bbPort<<endl;
                }
                break;

            case 's':
                sp = atoi(optarg);
                if(0<sp && sp<65535)
                {
                    CONFIG.syncPort = sp;
                    isOptionSet = true;
                    cout<<CONFIG.syncPort<<endl;
                }
                break;

            case 'f':
                CONFIG.daemon = false;
                isOptionSet = true;
                break;

            case 'd':
                CONFIG.debug = true;
                isOptionSet = true;
                break;

            default:
                //peers ?? host:port
                cout<<"Invalid option!"<<endl;
                break;
        }
    }

    if(isOptionSet)
    {
        cout << "load config option success!" << endl;

        print_config();
    }

    return 0;
}

int getParaFromBuf(char *buf, char *para, char *keyword)
{
    char *p1 = nullptr, *p2 = nullptr;

    p1 = strstr(buf,keyword);

    if(!p1)
    {
        return ERR;
    }
    //cout << p1 <<endl;

    p2 = strstr(p1,"\n");

    if(!p2)
    {
        return ERR;
    }
    //cout << p2 << endl;

    int offset = strlen(keyword);

    memcpy(para, p1+offset, p2-(p1+offset));

    //cout<<para<<endl;

    return SUCCESS;
}

int file_ope()
{
    fstream myFile;

    myFile.open("bbserv.conf", ios::in);//read
    if(myFile.is_open())
    {
        string line;
        while(getline(myFile, line))
        {
            cout<<line<<endl;
        }
        //cout << myFile;
        myFile.close();
    }

    myFile.open("bbserv.conf", ios::app);//write, append
    if(myFile.is_open())
    {
        myFile << "Hello\n";
        myFile.close();
    }

    return 1;
}



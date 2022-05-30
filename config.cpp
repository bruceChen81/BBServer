#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include "common.h"
#include "config.h"

using namespace std;

sysCfg CONFIG;

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


int load_config()
{
    int fd, parameter;

    ssize_t bytesRead;

    char buf[256];
    char para[128];

    memset(buf, 0, sizeof(buf));

    fd = open("bbserv.conf", O_RDONLY);

    if(fd == -1)
    {
        cout<<"open conf file error!"<<endl;
        return ERR;
    }

    bytesRead = read(fd, buf, sizeof(buf));

    //cout << buf<< endl;

    memset((void *) &CONFIG, 0, sizeof(CONFIG));

    //THMAX
    CONFIG.thMax = 20;

    memset(para, 0, sizeof(para));

    if(SUCCESS == getParaFromBuf((char *)buf, (char *)para,"THMAX="))
    {
        parameter = atoi(para);

        if(parameter >0 && parameter < 1024)
        {
            CONFIG.thMax = parameter;
        }
    }

    //BBPORT
    CONFIG.bbPort = 9000;

    memset(para, 0, sizeof(para));

    if(SUCCESS == getParaFromBuf((char *)buf, (char *)para,"BBPORT="))
    {
        parameter = atoi(para);

        if(parameter >0 && parameter < 65536)
        {
            CONFIG.bbPort = parameter;
        }
    }


    //SYNCPORT
    CONFIG.syncPort = 10000;

    memset(para, 0, sizeof(para));

    if(SUCCESS == getParaFromBuf((char *)buf, (char *)para,"SYNCPORT="))
    {
        parameter = atoi(para);

        if(parameter >0 && parameter < 65536)
        {
            CONFIG.syncPort = parameter;
        }
    }

    //BBFILE --- mandatory
    strcpy(CONFIG.bbFile, "");

    memset(para, 0, sizeof(para));

    if(SUCCESS == getParaFromBuf((char *)buf, (char *)para,"BBFILE="))
    {
        strcpy(CONFIG.bbFile, para);
    }

    //PEERS
    strcpy(CONFIG.peers, "");

    memset(para, 0, sizeof(para));

    if(SUCCESS == getParaFromBuf((char *)buf, (char *)para,"PEERS="))
    {
        strcpy(CONFIG.peers, para);
    }

    //DAEMON
    CONFIG.daemon = true;

    memset(para, 0, sizeof(para));

    if(SUCCESS != getParaFromBuf((char *)buf, (char *)para,"DAEMON="))
    {
        CONFIG.daemon = false;
    }

    //DEBUG
    CONFIG.debug = false;

    memset(para, 0, sizeof(para));

    if(SUCCESS == getParaFromBuf((char *)buf, (char *)para,"DEBUG="))
    {
        if(*para == 'D')
        {
            CONFIG.debug = true;
        }
    }


    //CONFIG.maxConnections = CONFIG.thMax;

    cout << "load config success!" << endl;

    if (CONFIG.debug)
    {
        cout << "THMAX = " << CONFIG.thMax << endl << "BBPORT = " << CONFIG.bbPort << endl << "SYNCPORT = " << CONFIG.bbPort << endl;
        cout << "BBFILE = " << CONFIG.bbFile << endl << "PEERS = " << CONFIG.peers << endl;
        cout << "DAEMON = " << CONFIG.daemon << endl << "DEBUG = " << CONFIG.debug << endl;
    }

    return SUCCESS;
}


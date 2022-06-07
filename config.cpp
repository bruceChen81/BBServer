#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <climits>

#include "common.h"
#include "config.h"

using std::cout;
using std::endl;
using std::string;
using std::fstream;
using std::ios;

sysCfg CONFIG;

int print_config()
{
    if (CONFIG.debugLevel >= DEBUG_LEVEL_D)
    {
        cout << "THMAX: " << CONFIG.thMax << endl << "BBPORT: " << CONFIG.bbPort << endl << "SYNCPORT: " << CONFIG.syncPort << endl;
        cout << "BBFILE: " << CONFIG.bbFile << endl << "PEERS: " << CONFIG.peers << endl;
        cout << "DAEMON: " << CONFIG.daemon << endl << "DEBUG: " << CONFIG.debug << endl;
        cout << "CFGFILE: " << CONFIG.cfgFile << endl;
        cout << "DBGLEVEL: " << CONFIG.debugLevel << endl;
    }

    return 0;
}

int load_config(char *pCfgFile)
{
    int fd;
    unsigned int tmax, bp, sp;

    ssize_t bytesRead;

    char buf[256];
    char arg[128];

    if(!pCfgFile)
    {
        return -1;
    }

    memset(buf, 0, sizeof(buf));
    memset((void *) &CONFIG, 0, sizeof(CONFIG));

    //save config file name
    //strcpy(arg, pCfgFile);

    //set default value
    CONFIG.thMax = 20;
    CONFIG.bbPort = 9000;
    CONFIG.syncPort = 10000;
    strcpy(CONFIG.bbFile, "");
    strcpy(CONFIG.peers, "");
    CONFIG.daemon = true;
    CONFIG.debug = false;
    strcpy(CONFIG.cfgFile, pCfgFile);
    //CONFIG.maxConnections = CONFIG.thMax;


    //fd = open("bbserv.conf", O_RDONLY);

    fd = open(CONFIG.cfgFile, O_RDONLY);

    CHECK(fd);

    bytesRead = read(fd, buf, sizeof(buf));

    CHECK(bytesRead);

    //THMAX
    memset(arg, 0, sizeof(arg));

    if(SUCCESS == getParaFromBuf((char *)buf, (char *)arg,(char *)"THMAX="))
    {
        tmax = atoi(arg);

        if(0<tmax && tmax<MAX_THREAD_POOL)
        {
            CONFIG.thMax = tmax;
        }
    }

    //BBPORT
    memset(arg, 0, sizeof(arg));

    if(SUCCESS == getParaFromBuf((char *)buf, (char *)arg,(char *)"BBPORT="))
    {
        bp = atoi(arg);

        if(0<bp && bp<65535)
        {
            CONFIG.bbPort = bp;
        }
    }

    //SYNCPORT
    memset(arg, 0, sizeof(arg));

    if(SUCCESS == getParaFromBuf((char *)buf, (char *)arg,(char *)"SYNCPORT="))
    {
        sp = atoi(arg);

        if(0<sp && sp<65535)
        {
            CONFIG.syncPort = sp;
        }
    }

    //BBFILE --- mandatory
    memset(arg, 0, sizeof(arg));

    if(SUCCESS == getParaFromBuf((char *)buf, (char *)arg,(char *)"BBFILE="))
    {
        strcpy(CONFIG.bbFile, arg);
    }

    //PEERS
    memset(arg, 0, sizeof(arg));

    if(SUCCESS == getParaFromBuf((char *)buf, (char *)arg,(char *)"PEERS="))
    {
        strcpy(CONFIG.peers, arg);
    }

    //DAEMON
    memset(arg, 0, sizeof(arg));

    if(SUCCESS != getParaFromBuf((char *)buf, (char *)arg,(char *)"DAEMON="))
    {
        CONFIG.daemon = false;
    }

    //DEBUG
    memset(arg, 0, sizeof(arg));

    if(SUCCESS == getParaFromBuf((char *)buf, (char *)arg,(char *)"DEBUG="))
    {
        if(*arg == 'D')
        {
            CONFIG.debug = true;
        }
    }


    if(CONFIG.debugLevel >= DEBUG_LEVEL_D)
        cout << "load config file success!" << endl;

    print_config();

    return SUCCESS;
}

int load_option(int argc, char **argv)
{
    int option;

    int bflag = 0, tflag = 0, pflag = 0, sflag = 0, fflag = 0, dflag = 0, cflag = 0, peerflag = 0, Dflag = 0;
    unsigned int tmax, bp, sp, dLevel;

    sysCfg optionCFG;
    memset((void *)&optionCFG, 0, sizeof(optionCFG));

//    extern char *optarg;
//    extern int optind, optopt;

    while ((option = getopt(argc,argv, "b:c:t:T:p:s:fdD:")) != -1)
    {
        switch(option)
        {
            case 'b':
                bflag = 1;
                strcpy(optionCFG.bbFile,optarg);
                break;

            case 'T':
            case 't':
                tflag = 1;
                tmax = atoi(optarg);
                if(0<tmax && tmax<MAX_THREAD_POOL)
                {
                    optionCFG.thMax = tmax;
                }
                break;

            case 'p':
                pflag = 1;
                bp = atoi(optarg);
                if(0<bp && bp<65535)
                {
                    optionCFG.bbPort = bp;
                }
                break;

            case 's':
                sflag = 1;
                sp = atoi(optarg);
                if(0<sp && sp<65535)
                {
                    optionCFG.syncPort = sp;
                }
                break;

            case 'f':
                fflag = 1;
                optionCFG.daemon = false;
                break;

            case 'd':
                dflag = 1;
                optionCFG.debug = true;
                optionCFG.debugLevel = DEBUG_LEVEL_D;
                break;

            case 'D':
                Dflag = 1;
                dLevel = atoi(optarg);
                if(dLevel >=0 && dLevel <= DEBUG_LEVEL_MAX)
                {
                    optionCFG.debugLevel = dLevel;
                }

                break;

            case 'c':
                //change config file name
                cflag = 1;
                strcpy(optionCFG.cfgFile,optarg);
                break;

            default:
                //peers ?? host:port
                //peerflag = 1;
                cout<<"Invalid option!"<<endl;
                break;
        }
    }

    //First: process cflag(config file name), reload config file
    if(cflag)
    {
        //reload config file
        load_config((char *)&optionCFG.cfgFile);
    }

    //Second: process option argument, overwrite config file arg

    if(bflag)
    {
        strcpy(CONFIG.bbFile, optionCFG.bbFile);
    }

    if(tflag)
    {
        CONFIG.thMax = optionCFG.thMax;
    }

    if(pflag)
    {
        CONFIG.bbPort = optionCFG.bbPort;
    }

    if(sflag)
    {
        CONFIG.syncPort = optionCFG.syncPort;
    }

    if(fflag)
    {
        CONFIG.daemon = optionCFG.daemon;
    }

    if(dflag)
    {
        CONFIG.debug = optionCFG.debug;
    }

    if(Dflag)
    {
        CONFIG.debugLevel = optionCFG.debugLevel;
    }

    if(peerflag)
    {
        strcpy(CONFIG.peers, optionCFG.peers);
    }

    if(cflag || bflag || tflag || pflag || sflag || fflag || dflag ||Dflag || peerflag)
    {
        if (CONFIG.debugLevel >= DEBUG_LEVEL_D)
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

    p2 = strstr(p1,"\n");

    if(!p2)
    {
        return ERR;
    }

    int offset = strlen(keyword);

    memcpy(para, p1+offset, p2-(p1+offset));


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

int get_last_line(string& lastline)
{
    //string filename = "bbfile";
    fstream myFile;

    myFile.open(CONFIG.bbFile, ios::in);//read
    if(myFile.is_open())
    {
        myFile.seekg(-2, std::ios::end);

        bool keeplooping = true;

        while(keeplooping)
        {
            char ch;
            myFile.get(ch);

            if((int)myFile.tellg()<=1)
            {
                myFile.seekg(0);
                keeplooping = false;
            }
            else if(ch == '\n')
            {
                keeplooping = false;
            }
            else
            {
                myFile.seekg(-2, std::ios_base::cur);
            }
        }

        //string lastline;
        getline(myFile, lastline);

        if (CONFIG.debugLevel >= DEBUG_LEVEL_APP)
            cout<<"get_last_line:"<<lastline<<endl;

        myFile.close();
    }

    return 0;
}



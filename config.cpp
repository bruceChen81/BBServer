/*
config.cpp

Created by Dianyong Chen, 2022-05-26

CS590 Master Project(BBServer) @ Bishop's University

*/


#include "common.h"
#include "config.h"

//global config CB
sysCfg SysCfgCB;

int getParaFromBuf(char *buf, char *para, char *keyword){
    char *p1 = nullptr, *p2 = nullptr;
    p1 = strstr(buf,keyword);

    if(!p1){
        return ERR;
    }

    p2 = strstr(p1,"\n");

    if(!p2){
        return ERR;
    }

    int offset = strlen(keyword);
    memcpy(para, p1+offset, p2-(p1+offset));

    if(strlen(para) == 0)
        return -1;
    else
        return SUCCESS;
}

int print_config()
{
    LOG(DEBUG_LEVEL_NONE){
        cout << "THMAX: " << SysCfgCB.thMax << endl;
        cout << "BBPORT: " << SysCfgCB.bbPort << endl;
        cout << "SYNCPORT: " << SysCfgCB.syncPort << endl;
        cout << "BBFILE: " << SysCfgCB.bbFile << endl;
        cout << "PEERS: " << SysCfgCB.peers << endl;
        cout << "DAEMON: " << SysCfgCB.daemon << endl;
        cout << "DEBUG: " << SysCfgCB.debug << endl;
        cout << "CFGFILE: " << SysCfgCB.cfgFile << endl;
        cout << "SYNC: "<<SysCfgCB.syncEnalbe<<endl;
        cout << "DBGLEVEL: " << SysCfgCB.debugLevel << endl<<endl;
    }

    return 0;
}

int load_config(const char *pCfgFile)
{
    int fd;
    unsigned int tmax, bp, sp;
    ssize_t bytesRead;

    char buf[256];
    char arg[128];

    if(!pCfgFile){
        return -1;
    }

    memset(buf, 0, sizeof(buf));
    memset((void *) &SysCfgCB, 0, sizeof(SysCfgCB));

    //set default value
    SysCfgCB.thMax = 20;
    SysCfgCB.bbPort = 9000;
    SysCfgCB.syncPort = 10000;
    strcpy(SysCfgCB.bbFile, "");
    strcpy(SysCfgCB.peers, "");
    SysCfgCB.daemon = true;
    SysCfgCB.debug = false;
    strcpy(SysCfgCB.cfgFile, pCfgFile);
    SysCfgCB.syncEnalbe = false;
    SysCfgCB.debugLevel = DEBUG_LEVEL_NONE;
    //SysCfgCB.maxConnections = SysCfgCB.thMax;   

    fd = open(SysCfgCB.cfgFile, O_RDONLY);
    //CHECK(fd);

    if(fd < 0)
        return -1;

    bytesRead = read(fd, buf, sizeof(buf));
    CHECK(bytesRead);

    close(fd);

    //THMAX
    memset(arg, 0, sizeof(arg));
    if(SUCCESS == getParaFromBuf((char *)buf, (char *)arg,(char *)"THMAX=")){
        tmax = atoi(arg);
        if(0<tmax && tmax<MAX_THREAD_POOL){
            SysCfgCB.thMax = tmax;
        }
    }

    //BBPORT
    memset(arg, 0, sizeof(arg));
    if(SUCCESS == getParaFromBuf((char *)buf, (char *)arg,(char *)"BBPORT=")){
        bp = atoi(arg);
        if(0<bp && bp<65535){
            SysCfgCB.bbPort = bp;
        }
    }

    //SYNCPORT
    memset(arg, 0, sizeof(arg));
    if(SUCCESS == getParaFromBuf((char *)buf, (char *)arg,(char *)"SYNCPORT=")){
        sp = atoi(arg);
        if(0<sp && sp<65535){
            SysCfgCB.syncPort = sp;
        }
    }

    //BBFILE --- mandatory
    memset(arg, 0, sizeof(arg));
    if(SUCCESS == getParaFromBuf((char *)buf, (char *)arg,(char *)"BBFILE=")){
        strcpy(SysCfgCB.bbFile, arg);
    }

    //PEERS
    memset(arg, 0, sizeof(arg));
    if(SUCCESS == getParaFromBuf((char *)buf, (char *)arg,(char *)"PEERS=")){
        strcpy(SysCfgCB.peers, arg);
        SysCfgCB.syncEnalbe = true;
    }

    //DAEMON
    memset(arg, 0, sizeof(arg));
    if(SUCCESS == getParaFromBuf((char *)buf, (char *)arg,(char *)"DAEMON=")){
        if(!strcmp(arg,"false") || !strcmp(arg, "0")){
            SysCfgCB.daemon = false;
        }
        else if(!strcmp(arg,"true") || !strcmp(arg, "1")){
            SysCfgCB.daemon = true;
        }
    }

    //DEBUG
    memset(arg, 0, sizeof(arg));
    if(SUCCESS == getParaFromBuf((char *)buf, (char *)arg,(char *)"DEBUG=")){
        if(!strcmp(arg,"false") || !strcmp(arg, "0")){
            SysCfgCB.debug = false;
            SysCfgCB.debugLevel = DEBUG_LEVEL_NONE;
        }
        else if(!strcmp(arg,"true") || !strcmp(arg, "1")){
            SysCfgCB.debug = true;
            SysCfgCB.debugLevel = DEBUG_LEVEL_D;
        }
    }

    //print_config();

    return SUCCESS;
}

int load_option(int argc, char **argv)
{
    int option;
    int bflag = 0, tflag = 0, pflag = 0, sflag = 0, fflag = 0, dflag = 0, cflag = 0, peerflag = 0, Dflag = 0;
    unsigned int tmax, bp, sp, dLevel;

    sysCfg optionCFG;
    memset((void *)&optionCFG, 0, sizeof(optionCFG));

    while ((option = getopt(argc,argv, "b:c:t:T:p:s:fdD:")) != -1){
        switch(option){
            case 'b':
                bflag = 1;
                strcpy(optionCFG.bbFile,optarg);
                break;

            case 'T':
            case 't':
                tflag = 1;
                tmax = atoi(optarg);
                if(0<tmax && tmax<MAX_THREAD_POOL){
                    optionCFG.thMax = tmax;
                }
                break;

            case 'p':
                pflag = 1;
                bp = atoi(optarg);
                if(0<bp && bp<65535){
                    optionCFG.bbPort = bp;
                }
                break;

            case 's':
                sflag = 1;
                sp = atoi(optarg);
                if(0<sp && sp<65535){
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
                if(dLevel >=0 && dLevel <= DEBUG_LEVEL_MAX){
                    optionCFG.debugLevel = dLevel;
                }
                break;

            case 'c':
                //change config file name
                cflag = 1;
                strcpy(optionCFG.cfgFile,optarg);
                break;

            default:
                {
                    cout<<"Invalid option or argument!"<<endl;
                }
                break;
        }
    }

    argc -= optind;
    argv += optind;

    int cnt_peers = argc;

    if(cnt_peers > 0)
    {
        peerflag = true;

        //cout << "cnt peers:" << cnt_peers << endl;

        for(int i=0;i<cnt_peers;i++)
        {
            //cout <<argv[i] <<endl;

            if(strstr(argv[i], ":") != nullptr){
                strcat(optionCFG.peers, argv[i]);

                if(i != cnt_peers-1){
                    strcat(optionCFG.peers, " ");
                }
            }
        }
    }

    //First: process cflag(config file name), reload config file
    if(cflag){
        //reload config file
        load_config((char *)&optionCFG.cfgFile);
    }

    //Second: process option argument, overwrite config file arg

    if(bflag){
        strcpy(SysCfgCB.bbFile, optionCFG.bbFile);
    }

    if(tflag){
        SysCfgCB.thMax = optionCFG.thMax;
    }

    if(pflag){
        SysCfgCB.bbPort = optionCFG.bbPort;
    }

    if(sflag){
        SysCfgCB.syncPort = optionCFG.syncPort;
    }

    if(fflag){
        SysCfgCB.daemon = optionCFG.daemon;
    }

    if(dflag){
        SysCfgCB.debug = optionCFG.debug;
        if(SysCfgCB.debugLevel < DEBUG_LEVEL_D){
            SysCfgCB.debugLevel = DEBUG_LEVEL_D;
        }
    }

    if(Dflag){
        SysCfgCB.debugLevel = optionCFG.debugLevel;
    }

    if(peerflag){
        strcpy(SysCfgCB.peers, optionCFG.peers);
    }

    if(cflag || bflag || tflag || pflag || sflag || fflag || dflag ||Dflag || peerflag){
        LOG(DEBUG_LEVEL_NONE)
            cout << "Load config option command line success!" << endl;

        print_config();
    }

    return 0;
}




//message number

int msgNumber = 0;

pthread_mutex_t clientMsgNoLock;

int get_last_line(string& lastline)
{
    //string filename = "bbfile";
    fstream myFile;

    myFile.open(SysCfgCB.bbFile, ios::in);//read
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

        LOG(DEBUG_LEVEL_APP)
            cout<<"bbfile get_last_line:"<<lastline<<endl;

        myFile.close();
    }

    return 0;
}


string alloc_new_msg_number()
{
    int number;

    pthread_mutex_lock(&clientMsgNoLock);

    msgNumber++;

    if(msgNumber == INT_MAX)
    {
        msgNumber = 1;
    }

    number = msgNumber;

    pthread_mutex_unlock(&clientMsgNoLock);

    string strNumber = std::to_string(number);

    LOG(DEBUG_LEVEL_D)
        cout << "Alloc new msg number:" << strNumber <<endl;

    return strNumber;
}


int update_msg_number(std::string& strNumber)
{
    LOG(DEBUG_LEVEL_APP)
        cout << "update msg number, input:" << strNumber <<endl;

    int number = stoi(strNumber);

    if(number <= 0)
        return -1;

    pthread_mutex_lock(&clientMsgNoLock);

    msgNumber = number;

    if(msgNumber == INT_MAX)
    {
        msgNumber = 1;
    }

    pthread_mutex_unlock(&clientMsgNoLock);

    LOG(DEBUG_LEVEL_APP)
        cout << "update msg number:" << msgNumber <<endl;

    return 0;
}


int load_msg_number()
{
    string lastline;
    string msgNumLast;

    pthread_mutex_init(&clientMsgNoLock, NULL);

    get_last_line(lastline);

    msgNumLast = lastline.substr(0, lastline.find("/"));

    if(msgNumLast.empty())
    {
        LOG(DEBUG_LEVEL_APP)
            cout << "can not read last message number!" << endl;

        return -1;
    }

    msgNumber = stoi(msgNumLast);

    LOG(DEBUG_LEVEL_APP)
        cout << "load message number:" << msgNumber << endl;

    return 0;
}


//millisecond
string get_time_ms()
{
    //auto t = std::time(nullptr);
    //auto tm = *std::localtime(&t);
    //cout << std::put_time(&tm, "%Y%m%d%H%M%S");

    time_t t = time(nullptr);
    std::string datetime(32,0);
    //datetime.resize(std::strftime(&datetime[0], datetime.size(), "%a %d %b %Y - %I:%M:%S%p", std::localtime(&t)));
    datetime.resize(strftime(&datetime[0], datetime.size(), "%Y%m%d%H%M%S", localtime(&t)));
   
    struct timeval tt;
    gettimeofday(&tt, NULL); 
    int ms = tt.tv_usec/1000;  
    
    if(ms < 10){
        datetime += "00";
    }
    else if(ms < 100){
        datetime += "0";
    }

    datetime += std::to_string(ms);   

    return datetime;
}


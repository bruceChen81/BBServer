/*
bootup.cpp

Created by Dianyong Chen, 2022-05-24

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

int sys_load_config(int argc, char *argv[]);
int sys_bootup();

void sigint_handler(int sig);
void sigquit_handler(int sig);
void sighup_handler(int sig);

void *handle_client_event(void *arg);


int main(int argc, char *argv[])
{
    
    //signal(SIGINT, sigint_handler);

    signal(SIGINT, sighup_handler);

    signal(SIGQUIT, sigquit_handler);

    //signal(SIGHUP, sighup_handler);


    sys_load_config(argc, argv);    

    if(SysCfgCB.daemon)
    {
        daemon(1,1);

        //redirect stdout to bbserv.log
        int file = open("bbserv.log", O_WRONLY | O_CREAT, 0777);

        dup2(file, STDOUT_FILENO);
        close(file);
    }

    //write bbserv.pid
    int fd = open("bbserv.pid", O_WRONLY | O_CREAT, 0777);
    CHECK(fd);

    int pid = getpid();
    string strpid = std::to_string(pid);
    write(fd, strpid.c_str(), strpid.length());
    close(fd);


    sys_bootup();


    string str;

    while(getline(cin, str))
    {
        cout <<"Input: "<< str << endl;

        sleep(1);
    }


    return 0;
}




int create_thread_pool()
{
    pthread_t threadPool[SysCfgCB.thMax];

    for(unsigned int i=0;i<SysCfgCB.thMax;i++)
    {
        pthread_create(&threadPool[i], NULL, handle_client_event, NULL);
    }

    return 0;
}

void *handle_tcp_connection(void *arg)
{
    char *uargv = nullptr;

    start_conn_service();

    return uargv;
}

int create_tcp_connection_thread()
{
    pthread_t threadConn;

    pthread_create(&threadConn, NULL, handle_tcp_connection, NULL);

    return 0;
}


int sys_load_config(int argc, char *argv[])
{
    if (load_config((char *)DEFAULT_CFG_FILE) >= 0)
    {
        if(SysCfgCB.debugLevel >= DEBUG_LEVEL_D)
            cout << "Load config file success!" << endl;
    }

    print_config();

    load_option(argc, argv);

    //check if bbfile is set
    if(!strlen(SysCfgCB.bbFile))
    {
        cout << "BBFILE name is not configured, please set by config file or command line -b !!!"<< endl;
        exit(-1);
    }

    //load_msg_number();

    return 0;
}

int sys_reload_config_sighub()
{

    //relaod config file
    bool debug = SysCfgCB.debug;

    string cfgFile = string(SysCfgCB.cfgFile);

    //load_config((char *)DEFAULT_CFG_FILE);
    load_config(cfgFile.c_str());


    SysCfgCB.debug = debug;

    SysCfgCB.debugLevel = debug ? DEBUG_LEVEL_D : DEBUG_LEVEL_NONE;

    print_config();

    //check if bbfile is set
    if(!strlen(SysCfgCB.bbFile))
    {
        cout << "BBFILE name is not configured, please set by config file or command line -b !!!"<< endl;
        exit(-1);
    }

    //load_msg_number();

    return 0;
}



int sys_bootup()
{
    cout << "System booting up......................" <<endl;

    if(create_client_list() >= 0)
    {
        LOG(DEBUG_LEVEL_D)
            cout << "Client list created!" << endl;
    }

    if(create_client_event_queue() >= 0)
    {
        LOG(DEBUG_LEVEL_D)
            cout << "Client event queue created!" << endl;
    }


    if(create_thread_pool() >= 0)
    {
        LOG(DEBUG_LEVEL_D)
            cout << "Client event process thread pool created!" << endl;
    }

    if(init_bbfile_access_semahpores() >= 0)
    {
        LOG(DEBUG_LEVEL_D)
            cout << "Init bbfile access control semaphores success!" << endl;
    }

    if(init_sync_server_list() >= 0)
    {
        LOG(DEBUG_LEVEL_D)
            cout << "Init sync server list success!" << endl;
    }

    if(create_tcp_connection_thread() >= 0)
    {
        LOG(DEBUG_LEVEL_D)
            cout << "TCP connection process thread created!" << endl;
    }

    if(create_timer_master() >= 0)
    {
        LOG(DEBUG_LEVEL_D)
            cout << "Sync master timer created!" << endl;
    }

    if(create_timer_slave() >= 0)
    {
        LOG(DEBUG_LEVEL_D)
            cout << "Sync slave timer created!" << endl;
    }

    return 0;
}

int sys_terminate()
{
    cout << "System terminating......" <<endl;


    destroy_client_list();

    destroy_client_event_queue();

    destroy_sync_server_list();

    destroy_timer();

    destroy_bbfile_access_semahpores();

    destroy_connection();


    //close opend file

    return 0;
}

void sigint_handler(int sig)
{
    cout << "catch SIGINT signal!" <<endl;

    sys_terminate();

    signal(SIGINT, SIG_DFL);

    kill(getpid(), SIGINT);
}

void sigquit_handler(int sig)
{
    cout << "catch SIGQUIT signal!" <<endl;

    //waiting end of file ope

    //write_start();

    sys_terminate();

    kill(getpid(), SIGTERM);
}

void sighup_handler(int sig)
{
    cout << "catch SIGHUP signal!" <<endl;

    sys_terminate();

    sleep(1);

    const char *args1 = "./bbserv";
    char *const args[] = {"./bbserv", "-f", NULL};

    //execve(args[0], args, NULL);
    execve(args1, args, NULL);
}













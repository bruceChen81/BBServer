#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED

#define MAX_THREAD_POOL 1024

#define SYNC_STATE_TIMEOUT 10

#define DEFAULT_CFG_FILE "bbserv.conf"

typedef enum DbgLevel
{
    DEBUG_LEVEL_NONE = 0,
    DEBUG_LEVEL_D    = 1,
    DEBUG_LEVEL_APP  = 2,
    DEBUG_LEVEL_CONN = 3,
    DEBUG_LEVEL_QUEUE= 4,
    DEBUG_LEVEL_MAX

}DbgLevel;


typedef struct sysCfg
{
    unsigned short thMax;
    unsigned short bbPort;
    unsigned short syncPort;
    char bbFile[256];
    char peers[256];
    bool daemon;
    bool debug;
    char cfgFile[128];
    bool syncEnalbe;
    unsigned int maxConnections;
    unsigned int debugLevel; //1-debug == true, 2-app message, 3-queue, 4-connection

}sysCfg;



extern sysCfg SysCfgCB;

int print_config();

int load_config(const char *pCfgFile);

int load_option(int argc, char **argv);

int load_msg_number();

std::string alloc_new_msg_number();

int get_new_msg_number(std::string& strNumber);

int update_msg_number(std::string& strNumber);




#endif // CONFIG_H_INCLUDED

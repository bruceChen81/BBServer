#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED

#define MAX_THREAD_POOL 1024

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
    unsigned int maxConnections;
    unsigned int debugLevel; //1-debug == true, 2-app message, 3-queue, 4-connection

}sysCfg;



extern sysCfg CONFIG;

int print_config();

int load_config(char *pCfgFile);

int load_option(int argc, char **argv);

int load_msg_number();

int getParaFromBuf(char *buf, char *arg, char *keyword);

int get_last_line(std::string& lastline);




#endif // CONFIG_H_INCLUDED

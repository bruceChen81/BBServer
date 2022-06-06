#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED

#define MAX_THREAD_POOL 1024

#define DEFAULT_CFG_FILE "bbserv.conf"

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

}sysCfg;

extern sysCfg CONFIG;

int print_config();

int load_config(char *pCfgFile);

int load_option(int argc, char **argv);

int load_msg_number();

int getParaFromBuf(char *buf, char *arg, char *keyword);

int get_last_line(std::string& lastline);

int get_new_msg_number(std::string& strNumber);


#endif // CONFIG_H_INCLUDED

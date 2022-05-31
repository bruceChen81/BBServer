#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED

#define MAX_THREAD_POOL 1024

typedef struct sysCfg
{
    unsigned short thMax;
    unsigned short bbPort;
    unsigned short syncPort;
    char bbFile[256];
    char peers[256];
    bool daemon;
    bool debug;
    unsigned int maxConnections;

}sysCfg;

extern sysCfg CONFIG;

int print_config();

int load_config();

int load_option(int argc, char **argv);

int getParaFromBuf(char *buf, char *para, char *keyword);


#endif // CONFIG_H_INCLUDED

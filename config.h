#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED

typedef struct sysCfg
{
    unsigned int thMax;
    unsigned short bbPort;
    unsigned short syncPort;
    char bbFile[256];
    char peers[256];
    bool daemon;
    bool debug;
    unsigned int maxConnections;

}sysCfg;

extern sysCfg CONFIG;


int load_config();

int getParaFromBuf(char *buf, char *para, char *keyword);


#endif // CONFIG_H_INCLUDED

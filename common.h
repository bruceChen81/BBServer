#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED


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

sysCfg CONFIG;

#endif // COMMON_H_INCLUDED

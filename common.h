#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED


#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/queue.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <string>
#include <pthread.h>
#include <sys/queue.h>
#include <fcntl.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <climits>
#include <netdb.h>
#include <semaphore.h>


using std::cout;
using std::endl;
using std::cin;
using std::string;
using std::fstream;
using std::ios;



#define SUCCESS 0
#define ERR -1

typedef enum clientType
{
    CLIENT_USER = 1,
    CLIENT_SYNC_MASTER,
    CLIENT_SYNC_SLAVE

}clientType;

typedef enum clientCmdType
{
    CLIENT_CMD_WRITE = 1,
    CLIENT_CMD_REPLACE

}clientCmdType;

//client event queue
typedef enum clientEvType
{
    EV_ACCEPT = 1,
    EV_RECV,
    EV_SYNC_PRECOMMIT_ACK,
    EV_SYNC_PRECOMMIT_ERR,
    EV_SYNC_COMMIT_SUCCESS,
    EV_SYNC_COMMIT_UNSUCCESS,
    EV_SYNC_TIMEOUT_MASTER,
    EV_SYNC_TIMEOUT_SLAVE

}clientEvType;


#define CHECK_EXIT(X) ({int __val = (X); (__val == -1 ? ({fprintf(stderr, "ERROR ("__FILE__":%d) -- %s\n", __LINE__, strerror(errno)); exit(-1);-1;}) : __val);})

#define CHECK(X) ({int __val = (X); (__val == -1 ? ({fprintf(stderr, "ERROR ("__FILE__":%d) -- %s\n", __LINE__, strerror(errno)); -1;}) : __val);})

//fprintf(stderr, "ERROR = %s\n", strerror(errno));
//perror("ERROR");
//exit(-1);




#endif // COMMON_H_INCLUDED

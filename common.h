#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED

//#include <sys/uio.h>
//#include <errno.h>

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
typedef enum clientEv
{
    EV_ACCEPT = 1,
    EV_RECV,
    EV_SYNC_PRECOMMIT_ACK,
    EV_SYNC_PRECOMMIT_ERR,
    EV_SYNC_COMMIT_SUCCESS,
    EV_SYNC_COMMIT_UNSUCCESS,
    EV_SYNC_TIMEOUT_MASTER,
    EV_SYNC_TIMEOUT_SLAVE

}clientEv;


int sys_bootup(int argc, char *argv[]);

int sys_terminate();


#define CHECK_EXIT(X) ({int __val = (X); (__val == -1 ? \
                ({fprintf(stderr, "ERROR ("__FILE__":%d) -- %s\n", __LINE__, strerror(errno)); \
                exit(-1);-1;}) : __val);})

#define CHECK(X) ({int __val = (X); (__val == -1 ? \
                ({fprintf(stderr, "ERROR ("__FILE__":%d) -- %s\n", __LINE__, strerror(errno)); \
                -1;}) : __val);})

//fprintf(stderr, "ERROR = %s\n", strerror(errno));
//perror("ERROR");
//exit(-1);




#endif // COMMON_H_INCLUDED

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

#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED

//#include <sys/uio.h>
//#include <errno.h>

#define SUCCESS 1
#define ERR -1

#define MSG_GREETING "0.0 greeting COMMANDS: USER, READ, WRITE, REPLACE, QUIT\n"

#define CHECK(X) ({int __val = (X); (__val == -1 ? \
                ({fprintf(stderr, "ERROR ("__FILE__":%d) -- %s\n", __LINE__, strerror(errno)); \
                exit(-1);-1;}) : __val);})


//fprintf(stderr, "ERROR = %s\n", strerror(errno));
//perror("ERROR");
//exit(-1);

int create_thread_pool();

void *handle_client(void *arg);



#endif // COMMON_H_INCLUDED

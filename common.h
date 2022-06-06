#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED

//#include <sys/uio.h>
//#include <errno.h>

#define SUCCESS 1
#define ERR -1

#define MSG_GREETING "0.0 greeting COMMANDS: USER, READ, WRITE, REPLACE, QUIT\n"


#define CHECK_EXIT(X) ({int __val = (X); (__val == -1 ? \
                ({fprintf(stderr, "ERROR ("__FILE__":%d) -- %s\n", __LINE__, strerror(errno)); \
                exit(-1);-1;}) : __val);})

#define CHECK(X) ({int __val = (X); (__val == -1 ? \
                ({fprintf(stderr, "ERROR ("__FILE__":%d) -- %s\n", __LINE__, strerror(errno)); \
                -1;}) : __val);})

//fprintf(stderr, "ERROR = %s\n", strerror(errno));
//perror("ERROR");
//exit(-1);

int create_thread_pool();

int create_msg_save_thread();

void *handle_client_event(void *arg);

void *handle_msg_save_event(void *arg);

//int process_msg(char *buf, int length);


#endif // COMMON_H_INCLUDED

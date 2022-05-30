#ifndef CONNECTION_H_INCLUDED
#define CONNECTION_H_INCLUDED



#define MAX_CONN 100

void conn_init();

void conn_set_fdset(fd_set *pFdSet);

void conn_check_fd_set(fd_set *pFdSet, sockaddr_in *pClientAddr);

void conn_add(int fd);

void conn_del(int fd);

void conn_close_all();


#endif // CONNECTION_H_INCLUDED

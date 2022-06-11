#ifndef SYNC_H_INCLUDED
#define SYNC_H_INCLUDED

#define MAX_SYNC_CONN 10

int start_sync_server();

void sync_conn_init();

void sync_conn_set_fdset(fd_set *pFdSet);

bool sync_conn_is_exist(int fd);

void sync_conn_add(int fd);

void sync_conn_del(int fd);

void sync_conn_close_all();

#endif // SYNC_H_INCLUDED

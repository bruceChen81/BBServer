#ifndef SEMAPHORE_H_INCLUDED
#define SEMAPHORE_H_INCLUDED

int init_bbfile_access_semahpores();

int destroy_bbfile_access_semahpores();

void read_start();

void read_end();

void write_start();

void write_end();

void bbfile_debug_wait(int time);

#endif // SEMAPHORE_H_INCLUDED

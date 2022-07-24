/*
semaphore.cpp

Created by Dianyong Chen, 2022-06-01

CS590 Master Project(BBServer) @ Bishop's University

*/

#include "common.h"

#include "semaphore.h"
#include "config.h"

//semaphore

sem_t wrt;
pthread_mutex_t readerMutex;

int readerCnt = 0;

void write_start()
{
    LOG(DEBUG_LEVEL_D)
        std::cout << "BBFile Access: WRITE trying!" << std::endl;

    sem_wait(&wrt);

    LOG(DEBUG_LEVEL_D)
        std::cout << "BBFile Access: WRITE begin!" << std::endl;

    return;
}

void write_end()
{
    //for test
    bbfile_debug_wait(6);

    sem_post(&wrt);

    LOG(DEBUG_LEVEL_D)
        std::cout << "BBFile Access: WRITE end!" << std::endl;

    return;
}



void read_start()
{
    LOG(DEBUG_LEVEL_D)
        std::cout << "BBFile Access: READ trying!" << std::endl;

    pthread_mutex_lock(&readerMutex);
    readerCnt++;
    if(readerCnt == 1){
        sem_wait(&wrt);
    }
    pthread_mutex_unlock(&readerMutex);

    LOG(DEBUG_LEVEL_D)
        std::cout << "BBFile Access: READ begin! Set reader count["<<readerCnt<<"]" << std::endl;

    //for test
    bbfile_debug_wait(3);

    //read

    return;
}

void read_end()
{
    pthread_mutex_lock(&readerMutex);
    readerCnt--;
    if(readerCnt == 0){
        sem_post(&wrt);
    }
    pthread_mutex_unlock(&readerMutex);

    LOG(DEBUG_LEVEL_D)
        std::cout << "BBFile Access: READ end!   Set reader count["<<readerCnt<<"]" << std::endl;

    return;
}


int init_bbfile_access_semahpores()
{
    if (pthread_mutex_init(&readerMutex, NULL) != 0){
        std::cout << "init readerMutex failed!" << std::endl;
    }

    sem_init(&wrt, 0, 1);

    return 0;
}

int destroy_bbfile_access_semahpores()
{
    pthread_mutex_destroy(&readerMutex);

    LOG(DEBUG_LEVEL_D)
        std::cout << "BBfile access mutex deleted!" << std::endl;

    return 0;
}


void bbfile_debug_wait(int time)
{
    if(SysCfgCB.debug){
        sleep(time);
    }

    return;
}


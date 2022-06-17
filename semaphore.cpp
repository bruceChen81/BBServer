
#include <iostream>
#include <unistd.h>
#include <semaphore.h>
#include <string>

#include "semaphore.h"
#include "config.h"

//semaphore

sem_t wrt;
pthread_mutex_t readerMutex;

int readerCnt = 0;

void write_start()
{
    if(CONFIG.debugLevel >= DEBUG_LEVEL_D)
        std::cout << "BBFile Access: write trying!" << std::endl;

    sem_wait(&wrt);

    if(CONFIG.debugLevel >= DEBUG_LEVEL_D)
        std::cout << "BBFile Access: write begin!" << std::endl;

    return;
}

void write_end()
{
    //for test
    bbfile_debug_wait(6);

    sem_post(&wrt);

    if(CONFIG.debugLevel >= DEBUG_LEVEL_D)
        std::cout << "BBFile Access: write end!" << std::endl;

    return;
}



void read_start()
{
    if(CONFIG.debugLevel >= DEBUG_LEVEL_D)
        std::cout << "BBFile Access: read trying!" << std::endl;

    pthread_mutex_lock(&readerMutex);

    readerCnt++;

    if(readerCnt == 1)
    {
        sem_wait(&wrt);
    }

    pthread_mutex_unlock(&readerMutex);

    if(CONFIG.debugLevel >= DEBUG_LEVEL_D)
        std::cout << "BBFile Access: read begin! set reader count["<<readerCnt<<"]" << std::endl;


    //for test
    bbfile_debug_wait(3);

    //read

    return;
}

void read_end()
{


    pthread_mutex_lock(&readerMutex);

    readerCnt--;

    if(readerCnt == 0)
    {
        sem_post(&wrt);
    }

    pthread_mutex_unlock(&readerMutex);

    if(CONFIG.debugLevel >= DEBUG_LEVEL_D)
        std::cout << "BBFile Access: read end!   set reader count["<<readerCnt<<"]" << std::endl;

    return;
}


int init_bbfile_access_semahpores()
{
    if (pthread_mutex_init(&readerMutex, NULL) != 0)
    {
        std::cout << "init readerMutex failed!" << std::endl;
    }

    sem_init(&wrt, 0, 1);

    return 0;
}

void bbfile_debug_wait(int time)
{
    if(CONFIG.debug)
    {
        sleep(time);
    }

    return;
}


#include "queue.h"


STAILQ_HEAD(clientHead, _client);

struct clientHead clientQueue;

pthread_mutex_t clientQueueLock;

pthread_cond_t cond_clientQueue = PTHREAD_COND_INITIALIZER;

int create_client_queue()
{
    if (pthread_mutex_init(&clientQueueLock, NULL) != 0)
    {
        std::cout << "init clientQueueLock failed!" << std::endl;
    }

    STAILQ_INIT(&clientQueue);

    std::cout << "client connection Queue created!" << std::endl;

    return 1;
}


void enClientQueue(client *pClient)
{
    pthread_mutex_lock(&clientQueueLock);

    STAILQ_INSERT_TAIL(&clientQueue, pClient, p);

    pthread_cond_signal(&cond_clientQueue);

    pthread_mutex_unlock(&clientQueueLock);
}

client *deClientQueue()
{
    client *p = nullptr;

    pthread_mutex_lock(&clientQueueLock);

    pthread_cond_wait(&cond_clientQueue, &clientQueueLock);

    if (STAILQ_EMPTY(&clientQueue))
    {
        p = nullptr;
    }
    else
    {
        p = STAILQ_FIRST(&clientQueue);

        STAILQ_REMOVE_HEAD(&clientQueue,p);
    }

    pthread_mutex_unlock(&clientQueueLock);

    return p;
}

bool isClientQueueEmpty()
{
    bool ret = false;

    pthread_mutex_lock(&clientQueueLock);

    ret = STAILQ_EMPTY(&clientQueue);

    pthread_mutex_unlock(&clientQueueLock);

    return ret;
}

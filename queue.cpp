#include <unistd.h>
#include <semaphore.h>

#include "queue.h"
#include "config.h"

//client event queue
STAILQ_HEAD(clientEvHead, _clientEvent) clientEvQueue
        = STAILQ_HEAD_INITIALIZER(clientEvQueue);

pthread_mutex_t clientEvQueueLock;

pthread_cond_t cond_clientEvQueue = PTHREAD_COND_INITIALIZER;


int create_client_event_queue()
{
    if (pthread_mutex_init(&clientEvQueueLock, NULL) != 0)
    {
        std::cout << "init clientEvQueueLock failed!" << std::endl;
    }

    STAILQ_INIT(&clientEvQueue);

    return 1;
}


void enClientEventQueue(clientEvent *pClientEv)
{
    if(CONFIG.debugLevel >= DEBUG_LEVEL_QUEUE)
        std::cout << "enClientEvQueue type:" << pClientEv->event << " fd:" << pClientEv->fd << std::endl;

    pthread_mutex_lock(&clientEvQueueLock);

    STAILQ_INSERT_TAIL(&clientEvQueue, pClientEv, p);

    pthread_cond_signal(&cond_clientEvQueue);

    pthread_mutex_unlock(&clientEvQueueLock);

    return;
}

clientEvent *deClientEventQueue()
{
    clientEvent *p = nullptr;

    pthread_mutex_lock(&clientEvQueueLock);

    pthread_cond_wait(&cond_clientEvQueue, &clientEvQueueLock);

    if (STAILQ_EMPTY(&clientEvQueue))
    {
        p = nullptr;
    }
    else
    {
        p = STAILQ_FIRST(&clientEvQueue);

        STAILQ_REMOVE_HEAD(&clientEvQueue,p);

        if(CONFIG.debugLevel >= DEBUG_LEVEL_QUEUE)
            std::cout << "deClientEvQueue type:" << p->event << " fd:" << p->fd << std::endl;
    }

    pthread_mutex_unlock(&clientEvQueueLock);

    return p;
}

bool isClientEventQueueEmpty()
{
    bool ret = false;

    pthread_mutex_lock(&clientEvQueueLock);

    ret = STAILQ_EMPTY(&clientEvQueue);

    pthread_mutex_unlock(&clientEvQueueLock);

    return ret;
}



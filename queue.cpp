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

    if(CONFIG.debugLevel >= DEBUG_LEVEL_D)
        std::cout << "client event Queue created!" << std::endl;

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








//message save event queue

STAILQ_HEAD(msgSaveEvHead, _msgSaveEvent) msgSaveEvQueue
        = STAILQ_HEAD_INITIALIZER(msgSaveEvQueue);

pthread_mutex_t msgSaveEvQueueLock;

pthread_cond_t cond_msgSaveEvQueue = PTHREAD_COND_INITIALIZER;


int create_msg_save_event_queue()
{
    if (pthread_mutex_init(&msgSaveEvQueueLock, NULL) != 0)
    {
        std::cout << "init msgSaveEvQueueLock failed!" << std::endl;
    }

    STAILQ_INIT(&msgSaveEvQueue);

    if(CONFIG.debugLevel >= DEBUG_LEVEL_D)
        std::cout << "msg save event Queue created!" << std::endl;

    return 1;
}


void enMsgSaveEventQueue(msgSaveEvent *pMsgSaveEv)
{
    if(CONFIG.debugLevel >= DEBUG_LEVEL_APP)
        std::cout << "enMsgSaveEventQueue type:"<<pMsgSaveEv->event<<" msg:" << pMsgSaveEv->msg << std::endl;

    pthread_mutex_lock(&msgSaveEvQueueLock);

    STAILQ_INSERT_TAIL(&msgSaveEvQueue, pMsgSaveEv, p);

    pthread_cond_signal(&cond_msgSaveEvQueue);

    pthread_mutex_unlock(&msgSaveEvQueueLock);

    return;
}

msgSaveEvent *deMsgSaveEventQueue()
{
    msgSaveEvent *p = nullptr;

    pthread_mutex_lock(&msgSaveEvQueueLock);

    pthread_cond_wait(&cond_msgSaveEvQueue, &msgSaveEvQueueLock);

    if (STAILQ_EMPTY(&msgSaveEvQueue))
    {
        p = nullptr;
    }
    else
    {
        p = STAILQ_FIRST(&msgSaveEvQueue);

        STAILQ_REMOVE_HEAD(&msgSaveEvQueue,p);
    }

    pthread_mutex_unlock(&msgSaveEvQueueLock);

    if(CONFIG.debugLevel >= DEBUG_LEVEL_APP)
    {
        if(p)
        {
            std::cout << "deMsgSaveEventQueue type:"<<p->event<<" msg:" << p->msg << std::endl;
        }
    }


    return p;
}

bool isMsgSaveEventQueueEmpty()
{
    bool ret = false;

    pthread_mutex_lock(&msgSaveEvQueueLock);

    ret = STAILQ_EMPTY(&msgSaveEvQueue);

    pthread_mutex_unlock(&msgSaveEvQueueLock);

    return ret;
}


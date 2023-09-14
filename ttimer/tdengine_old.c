
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>

#define mpool_h void *
typedef void *tmr_h;

typedef struct {
    int				numOfFree;
    int 			first;
    int 			numOfBlock;
    int 			blockSize;
    int *			freeList;
    char * 			pool;
    pthread_mutex_t	mutex;
} pool_t;

#define maxNumOfTmrCtrl  	16
#define MSECONDS_PER_TICK 	5

typedef struct _tmr_obj
{
    void *param1;
    void (*fp)(void *, void *);
    tmr_h				timerId;
    short				cycle;
    struct _tmr_obj * 	prev;
    struct _tmr_obj * 	next;
    int                 index;
    struct _tmr_ctrl_t *pCtrl;
} tmr_obj_t;

typedef struct
{
    tmr_obj_t * head;
    int			count;
} tmr_list_t;

typedef struct _tmr_ctrl_t
{
    void * 			signature;
    pthread_mutex_t mutex;
    int				resolution;
    int 			numOfPeriods;
    unsigned int	periodsFromStart;
    tmr_list_t *	tmrList;
    mpool_h			poolHandle;
    char            label[12];
    int 			maxNumOfTmrs;
    int 			numOfTmrs;
    int 			ticks;
    int				maxTicks;
} tmr_ctrl_t;


tmr_ctrl_t tmrCtrl[maxNumOfTmrCtrl];


mpool_h memPoolInit(int maxNum, int blockSize);
char * memPoolMalloc(mpool_h handle);
void memPoolFree(mpool_h handle, char *p);
void memPoolCleanup(mpool_h handle);


void tmrProcessList(tmr_ctrl_t *pCtrl)
{
    int index;
    tmr_list_t * pList;
    tmr_obj_t * pObj, *header;

    pthread_mutex_lock(&pCtrl->mutex);
    index = pCtrl->periodsFromStart % pCtrl->numOfPeriods;
    pList = &pCtrl->tmrList[index];
    while(1)
    {
        header = pList->head;
        if(header == NULL) break;

        if(header->cycle > 0)
        {
            pObj = header;
            while(pObj)
            {
                pObj->cycle--;
                pObj = pObj->next;
            }
            break;
        }

        pCtrl->numOfTmrs--;
        printf("%s %p, timer expired, fp:%p, tmr_h:%p, index:%d, total:%d\n", pCtrl->label, header->param1, header->fp,
               header, index, pCtrl->numOfTmrs);

        pList->head = header->next;
        if(header->next) header->next->prev = NULL;
        pList->count--;
        header->timerId = NULL;

        if (header->fp)
            (*(header->fp))(header->param1, header);

        memPoolFree(pCtrl->poolHandle, (char *)header);
    }

    pCtrl->periodsFromStart++;
    pthread_mutex_unlock(&pCtrl->mutex);
    //printf("%s tmrProcessList index[%d]\n", pCtrl->label, index);
}

void * timerLoopFunc(void)
{
    tmr_ctrl_t *pCtrl;
    int i = 0;

    for(i = 0; i < maxNumOfTmrCtrl; i++)
    {
        pCtrl = tmrCtrl + i;
        if(pCtrl->signature)
        {
            pCtrl->ticks++;
            if(pCtrl->ticks >= pCtrl->maxTicks)
            {
                tmrProcessList(pCtrl);
                pCtrl->ticks = 0;
            }
        }
    }
}

void * processAlarmSignal(void *tharg)
{
    sigset_t 		sigset;
    timer_t	 		timerId;
    int signo;

    sigemptyset(&sigset);
    sigaddset(&sigset, SIGALRM);
    sigprocmask(SIG_BLOCK, &sigset, NULL);

    struct itimerval new_value, old_value;
    new_value.it_value.tv_sec = 0;
    new_value.it_value.tv_usec = 1000 * MSECONDS_PER_TICK;
    new_value.it_interval.tv_sec = 0;
    new_value.it_interval.tv_usec = 1000 * MSECONDS_PER_TICK;
    setitimer(ITIMER_REAL, &new_value, &old_value);

    while(1)
    {
        if(sigwait(&sigset, &signo))
        {
            printf("Failed to wait signal: number %d", signo);
            continue;
        }
        timerLoopFunc();
    }
    return NULL;
}

void tmrModuleInit(void)
{
    pthread_t		thread;
    pthread_attr_t	tattr;

    memset(tmrCtrl, 0, sizeof(tmrCtrl));

    pthread_attr_init(&tattr);
    pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_DETACHED);
    if(pthread_create(&thread, &tattr, processAlarmSignal, NULL) != 0)
    {
        printf("failed to create timer thread");
        return;
    }

    pthread_attr_destroy(&tattr);
}

void * tmrInit(int maxNumOfTmrs, int resolution, int longest, char * label)
{
    tmr_ctrl_t * pCtrl;
    int tmrCtrlId = 0;
    int i = 0;

    //tmrCtrlId = taosAllocateId(tmrIdPool);
    pCtrl = tmrCtrl + tmrCtrlId;

    memset(pCtrl, 0, sizeof(tmr_ctrl_t));
    strncpy(pCtrl->label, label, sizeof(pCtrl->label));
    pCtrl->maxNumOfTmrs = maxNumOfTmrs;

    if((pCtrl->poolHandle = memPoolInit(maxNumOfTmrs + 10,sizeof(tmr_obj_t))) == NULL)
    {
        printf("%s failed to allocate mem pool", label);
        memPoolCleanup(pCtrl->poolHandle);
        return NULL;
    }

    if(resolution < MSECONDS_PER_TICK) resolution = MSECONDS_PER_TICK;
    pCtrl->resolution = resolution;
    pCtrl->maxTicks = resolution / MSECONDS_PER_TICK;
    pCtrl->ticks = rand() / pCtrl->maxTicks;
    pCtrl->numOfPeriods = longest / resolution;
    if(pCtrl->numOfPeriods < 10) pCtrl->numOfPeriods = 10;

    pCtrl->tmrList = (tmr_list_t *)malloc(sizeof(tmr_list_t) * pCtrl->numOfPeriods);
    for(i = 0; i < pCtrl->numOfPeriods; i++)
    {
        pCtrl->tmrList[i].head = NULL;
        pCtrl->tmrList[i].count = 0;
    }

    pCtrl->signature = pCtrl;
    printf("%s timer ctrl is initialized, tmrCtrlId:%d\n", label, tmrCtrlId);
    return pCtrl;
}

void tmrReset(void (*fp)(void *, void*), int mseconds, void * param1, void * handle, tmr_h *pTmrId)
{
    tmr_obj_t *pObj, *cNode, *pNode;
    tmr_list_t * pList = NULL;
    int index, period;
    tmr_ctrl_t *pCtrl = (tmr_ctrl_t *)handle;

    if(handle == NULL || pTmrId == NULL) return;

    period = mseconds / pCtrl->resolution;
    if(pthread_mutex_lock(&pCtrl->mutex) != 0)
        printf("%s mutex lock failed, reason:%s", pCtrl->label, strerror(errno));

    pObj = (tmr_obj_t *)(*pTmrId);

    if(pObj && pObj->timerId == *pTmrId)
    {
        pList = &(pCtrl->tmrList[pObj->index]);
        if(pObj->prev)
            pObj->prev->next = pObj->next;
        else
            pList->head = pObj->next;

        if(pObj->next)
            pObj->next->prev = pObj->prev;

        pList->count--;
        pObj->timerId = NULL;
        pCtrl->numOfTmrs--;
        printf("reset pObj:%p\n", pObj);
    }
    else
    {
        pObj = (tmr_obj_t *)memPoolMalloc(pCtrl->poolHandle);
        *pTmrId = pObj;
        if(pObj == NULL)
        {
            printf("%s failed to allocate timer, max:%d allocated:%d", pCtrl->label, pCtrl->maxNumOfTmrs, pCtrl->numOfTmrs);
            pthread_mutex_unlock(&pCtrl->mutex);
            return;
        }
        printf("malloc pObj:%p\n", pObj);
    }

    pObj->cycle = period / pCtrl->numOfPeriods;
    pObj->param1 = param1;
    pObj->fp = fp;
    pObj->timerId = pObj;
    pObj->pCtrl = pCtrl;

    index = (period + pCtrl->periodsFromStart) % pCtrl->numOfPeriods;
    pList = &(pCtrl->tmrList[index]);
    pObj->index = index;
    cNode = pList->head;
    pNode = NULL;

    while(cNode != NULL)
    {
        if(cNode->cycle < pObj->cycle)
        {
            pNode = cNode;
            cNode = cNode->next;
        }
        else
            break;
    }

    pObj->next = cNode;
    pObj->prev = pNode;

    if(cNode != NULL)
        cNode->prev = pObj;

    if(pNode != NULL)
        pNode->next = pObj;
    else
        pList->head = pObj;

    pList->count++;
    pCtrl->numOfTmrs++;

    if (pthread_mutex_unlock(&pCtrl->mutex) != 0)
        printf("%s mutex unlock failed, reason:%s", pCtrl->label, strerror(errno));

    printf("%s %p, timer is reset, fp:%p, tmr_h:%p, cycle:%d, index:%d, total:%d numOfFree:%d\n", pCtrl->label, param1, fp, pObj,
           pObj->cycle, index, pCtrl->numOfTmrs, ((pool_t *)pCtrl->poolHandle)->numOfFree);
    return;
}



mpool_h memPoolInit(int numOfBlock, int blockSize)
{
    int i = 0;
    pool_t * pool_p = NULL;

    if(numOfBlock <= 1 || blockSize <= 1)
    {
        printf("invalid parameter in memPoolInit\n");
        return NULL;
    }

    pool_p = (pool_t *)malloc(sizeof(pool_t));
    if(pool_p == NULL)
    {
        printf("mempool malloc failed\n");
        return NULL;
    }

    memset(pool_p, 0, sizeof(pool_t));

    pool_p->blockSize = blockSize;
    pool_p->numOfBlock = numOfBlock;
    pool_p->pool = (char *)malloc((size_t)(blockSize * numOfBlock));
    pool_p->freeList = (int *)malloc(sizeof(int) * (size_t)numOfBlock);

    if(pool_p->pool == NULL || pool_p->freeList == NULL)
    {
        printf("failed to allocate memory\n");
        free(pool_p->freeList);
        free(pool_p->pool);
        free(pool_p);
    }

    pthread_mutex_init(&(pool_p->mutex), NULL);

    for(i = 0; i < pool_p->numOfBlock; i++)
        pool_p->freeList[i] = i;

    pool_p->first = 0;
    pool_p->numOfFree= pool_p->numOfBlock;

    return (mpool_h)pool_p;
}

char * memPoolMalloc(mpool_h handle)
{
    char * pos = NULL;
    pool_t * pool_p = (pool_t *)handle;

    pthread_mutex_lock(&pool_p->mutex);

    if(pool_p->numOfFree <= 0)
    {
        printf("mempool: out of memory");
    }
    else
    {
        pos = pool_p->pool + pool_p->blockSize * (pool_p->freeList[pool_p->first]);
        pool_p->first = (pool_p->first + 1) % pool_p->numOfBlock;
        pool_p->numOfFree--;
    }

    pthread_mutex_unlock(&pool_p->mutex);
    if(pos != NULL) memset(pos, 0, (size_t)pool_p->blockSize);
    return pos;
}

void memPoolFree(mpool_h handle, char * pMem)
{
    int index = 0;
    pool_t * pool_p = (pool_t *)handle;

    if(pool_p == NULL || pMem == NULL) return;

    pthread_mutex_lock(&pool_p->mutex);

    index = (int)(pMem - pool_p->pool) % pool_p->blockSize;
    if(index != 0)
    {
        printf("invalid free address:%p\n", pMem);
    }
    else
    {
        index = (int)((pMem - pool_p->pool) / pool_p->blockSize);
        if(index < 0 || index >= pool_p->numOfBlock)
        {
            printf("mempool: error, invalid address:%p\n", pMem);
        }
        else
        {
            pool_p->freeList[(pool_p->first + pool_p->numOfFree) % pool_p->numOfBlock] = index;
            pool_p->numOfFree++;
            memset(pMem, 0, (size_t)pool_p->blockSize);
        }
    }

    pthread_mutex_unlock(&pool_p->mutex);
}

void memPoolCleanup(mpool_h handle)
{
    pool_t *pool_p = (pool_t *)handle;

    pthread_mutex_destroy(&pool_p->mutex);
    if(pool_p->pool) free(pool_p->pool);
    if(pool_p->freeList) free(pool_p->freeList);
}

void timerFunc1(void *param, void *tmrId)
{
    int id = *(int *)param;
    printf("%s id[%d]\n", __func__, id);
}

void timerFunc2(void *param, void *tmrId)
{
    int id = *(int *)param;
    printf("%s id[%d]\n", __func__, id);
}

void test()
{
    void *timerHandle = tmrInit(5, 100, 1000, "http");
    void *timer1 = NULL, *timer2 = NULL, *timer3 = NULL;
    int id1 = 1, id2 = 2, id3 = 3;

    int five_seconds = 1000 * 5;
    int ten_seconds = 1000 * 10;
    int sixty_seconds = 1000 * 60;
    tmrReset(timerFunc1, five_seconds, (void *)&id1, timerHandle, &timer1);

    tmrReset(timerFunc2, ten_seconds, (void *)&id2, timerHandle, &timer2);

    tmrReset(timerFunc2, sixty_seconds, (void *)&id3, timerHandle, &timer3);
}

int main()
{
    sigset_t 		sigset;

    sigemptyset(&sigset);
    sigaddset(&sigset, SIGALRM);
    sigprocmask(SIG_BLOCK, &sigset, NULL);

    tmrModuleInit();
    test();
    while(1)
    {
        sleep(1);
    }
}

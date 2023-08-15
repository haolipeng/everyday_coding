#include <stdint.h>
#include <pthread.h>
#include <stdatomic.h>

#include "timer.h"
#define atomic_add_fetch_ptr(ptr, val) __atomic_add_fetch((ptr), (val), __ATOMIC_SEQ_CST)

typedef union _tmr_ctrl_t{
    char label[16];
    struct {
        char padding[16 - sizeof(union _tmr_ctrl_t*)];
        union _tmr_ctrl_t* next;
    };
}tmr_ctrl_t;

typedef struct tmr_obj_t{
    uintptr_t           id;
    tmr_ctrl_t*         ctrl;
    struct tmr_obj_t*   mnext;
    struct tmr_obj_t*   prev;
    struct tmr_obj_t*   next;

    uint16_t            slot;
    uint8_t             wheel;
    uint8_t             state;
    uint8_t             refCount;
    uint8_t             reserved1;
    uint16_t             reserved2;

    union {
        int64_t expireAt;
        int64_t executeBy;
    };

    TAOS_TMR_CALLBACK fp;
    void*   param;
}tmr_obj_t;

typedef struct timer_list_t{
    int64_t     lockedBy;
    tmr_obj_t*  timers;
}timer_list_t;

typedef struct timer_map_t{
    uint32_t        size;
    uint32_t        count;
    timer_list_t*   slots;
}timer_map_t;

typedef struct time_wheel_t{
    pthread_mutex_t mutex;
    int64_t         nextScanAt;
    uint32_t        resolution;
    uint16_t        size;
    uint16_t        index;
    tmr_obj_t**     slots;
}time_wheel_t;

//TODO:not finished

int taosTmrThreads = 1;

static uintptr_t nextTimerId = 0;

static time_wheel_t wheels[] = {
        {.resolution = MSECONDS_PER_TICK, .size = 4096},
        {.resolution = 1000, .size = 1024},
        {.resolution = 60000, .size = 1024},
};
static timer_map_t  timerMap;

static uintptr_t  getNextTimerId(){
    uintptr_t id;
    do{
        id = atomic_add_fetch_ptr(&nextTimerId, 1);
    } while (id == 0);
    return id;
}


static void timerAddRef(tmr_obj_t* timer){
    atomic_add_fetch_8(&timer->refCount, 1);
}












#include <stdint.h>
#include "timer.h"

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
};
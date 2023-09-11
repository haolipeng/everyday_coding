#ifndef _OS_H
#define _OS_H

#define atomic_val_compare_exchange_16(ptr, oldval, newval) _InterlockedCompareExchange16((short volatile*)(ptr), (short)(newval), (short)(oldval))
#define atomic_val_compare_exchange_32(ptr, oldval, newval) _InterlockedCompareExchange((long volatile*)(ptr), (long)(newval), (long)(oldval))
#define atomic_val_compare_exchange_64(ptr, oldval, newval) _InterlockedCompareExchange64((__int64 volatile*)(ptr), (__int64)(newval), (__int64)(oldval))

#endif //_OS_H

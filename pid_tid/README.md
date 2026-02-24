# Linux PID / TID / TGID 关系

## 概念说明

| 内核字段 | 含义 | 用户态获取方式 |
|---|---|---|
| `task->pid` | 每个线程独有的内核级 ID | `gettid()` / `syscall(__NR_gettid)` |
| `task->tgid` | 线程组 ID（即主线程的 pid） | `getpid()` |

- **tgid** = Thread Group ID，同一进程内所有线程共享同一个 tgid
- **tid** = 线程自己的唯一 ID（即内核级的 pid）
- 对于主线程：`tid == tgid`
- 对于子线程：`tid != tgid`

## ASCII 图解

```
Linux 进程/线程 ID 关系图
========================

┌─────────────────────────────────────────────────────────┐
│                    Process (进程)                         │
│                    tgid = 1000                           │
│                                                         │
│  ┌───────────────┐  ┌───────────────┐  ┌──────────────┐ │
│  │  Main Thread   │  │   Thread 1    │  │   Thread 2   │ │
│  │  (主线程)      │  │   (子线程)     │  │   (子线程)    │ │
│  │               │  │               │  │              │ │
│  │  tid  = 1000  │  │  tid  = 1001  │  │  tid  = 1002 │ │
│  │  tgid = 1000  │  │  tgid = 1000  │  │  tgid = 1000 │ │
│  └───────────────┘  └───────────────┘  └──────────────┘ │
│        ▲                                                │
│        │                                                │
│   tid == tgid                                           │
│   (主线程特征)                                            │
└─────────────────────────────────────────────────────────┘

内核 task_struct 视角:
═══════════════════

  task_struct          task_struct          task_struct
  ┌──────────┐        ┌──────────┐        ┌──────────┐
  │ pid: 1000│        │ pid: 1001│        │ pid: 1002│
  │tgid: 1000│        │tgid: 1000│        │tgid: 1000│
  └────┬─────┘        └────┬─────┘        └────┬─────┘
       │                   │                   │
       └───────────────────┴───────────────────┘
                           │
                    同一个线程组
                  (thread group)

用户态 API 映射:
═══════════════

  ┌──────────────┐       ┌──────────────────────┐
  │   getpid()   │──────▶│  返回 task->tgid      │
  │              │       │  所有线程返回值相同     │
  └──────────────┘       └──────────────────────┘

  ┌──────────────┐       ┌──────────────────────┐
  │   gettid()   │──────▶│  返回 task->pid       │
  │              │       │  每个线程返回值不同     │
  └──────────────┘       └──────────────────────┘

  ┌──────────────┐       ┌──────────────────────┐
  │pthread_self()│──────▶│  返回用户态 pthread_t  │
  │              │       │  与内核 ID 无直接关系   │
  └──────────────┘       └──────────────────────┘
```

## 内核源码实现（kernel/sys.c）

```c
/**
 * getpid - 返回的是 tgid（线程组ID），不是内核级 pid
 *
 * 同一进程内所有线程调用 getpid() 都返回相同的值
 */
SYSCALL_DEFINE0(getpid)
{
        return task_tgid_vnr(current);  // 返回 current->tgid
}

/**
 * gettid - 返回的是内核级 pid，即每个线程独有的 ID
 *
 * 每个线程调用 gettid() 返回各自不同的值
 */
SYSCALL_DEFINE0(gettid)
{
        return task_pid_vnr(current);   // 返回 current->pid
}
```

关键辅助函数展开：

```c
// include/linux/sched.h
static inline pid_t task_tgid_vnr(struct task_struct *tsk)
{
        return __task_pid_nr_ns(tsk, PIDTYPE_TGID, NULL);
}

static inline pid_t task_pid_vnr(struct task_struct *tsk)
{
        return __task_pid_nr_ns(tsk, PIDTYPE_PID, NULL);
}
```

> 源码一目了然：`getpid()` 取的是 `PIDTYPE_TGID`，`gettid()` 取的是 `PIDTYPE_PID`。
> 这就是内核命名和用户态命名"反直觉"的根源。

## 重点和关键点

Linux内核里的命名和用户态容易混淆——内核 `task_struct.pid` 其实对应用户态的 **tid**，

而 `task_struct.tgid` 对应用户态的 **pid**（即 `getpid()` 的返回值）。

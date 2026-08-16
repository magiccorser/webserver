## Linux信号量

```
int sem_init(sem_t *sem, int pshared, unsigned int value);
//pshared	0代表当前进程内线程共享，1代表进程共享
//value		信号量的值
成功返回0，失败返回-1
```

## Linux条件变量

​	cond是用的linux内核里面的futex机制，内核里面有256个hash桶，把 futex 地址 hash 进去。同一个 cond 的等待者确实会聚到同一个桶的链表上，但多个不同的 futex（不同的锁）也可能撞到同一个桶，它们在同一个链表上串着。然后wait的就放在队列尾，signal就会唤醒队列头，如果队列里面没有东西，signal就会空唤醒，内核不会保存这个唤醒信号，如果多次唤醒，那唤醒的就会去抢互斥锁，抢到的执行。

#### futex机制详解：


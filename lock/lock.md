## Linux信号量

```
int sem_init(sem_t *sem, int pshared, unsigned int value);
//pshared	0代表当前进程内线程共享，1代表进程共享
//value		信号量的值
成功返回0，失败返回-1
```

## Linux条件变量

​	cond是用的linux内核里面的futex机制，内核里面有256个hash桶，把 futex 地址 hash 进去。同一个 cond 的等待者确实会聚到同一个桶的链表上，但多个不同的 futex（不同的锁）也可能撞到同一个桶，它们在同一个链表上串着。然后wait的就放在队列尾，signal就会唤醒队列头，如果队列里面没有东西，signal就会空唤醒，内核不会保存这个唤醒信号，如果多次唤醒，那唤醒的就会去抢互斥锁，抢到的执行。

## pthread_cond_wait()执行流程：

    上锁 -> 判断条件 -> 解锁(wait在解锁前执行) ->阻塞 -> 上锁 -> signal开始执行唤醒wait -> 解锁 -> 上锁(wait结束) -> 判断条件(while) ->解锁
3个重要问题
1.为什么判断条件前要上锁？
判断条件前上锁是怕signal在进入wait之前就改了判断条件，然后导致wait没有收到signal的信号
2.为什么wait阻塞睡觉前要上锁？
首先是signal也要锁，如果进入wait前不解锁，那signal就没有锁了，会发生死锁；其次是如果wait里面的解锁条件需要signal来修改，那wait不解锁，signal就永远无法唤醒wait
3.为什么最后出wait之前也要上锁？
怕别的线程把条件修改了，然后signal不能唤醒wait；如果唤醒之前有其他语句需要执行那也可以放在signal之前

#### futex机制详解：


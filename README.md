国科大OS实验
====
实现一个简易的操作系统，从系统引导到内部实现，进程的管理，中断的处理，内存的管理，文件系统等等。

注意：限于本人以及课程以外的人使用，同期课程成员作为实验使用可能被判为抄袭。

项目介绍
---

核心功能如下：

1.系统引导（Bootloader）

核心文件：arch\mips\boot\bootblock.S

2.利用PCB进行进程启动和调度

核心文件：kernel\sched\sched.c kernel\sched\queue.c

3.线程锁的实现

核心文件：kernel\sched\sched.c kernel\locking\lock.c

4.时钟中断的处理、优先级调度

核心文件：arch\mips\kernel\entry.S kernel\irq\irq.c

5.系统调用（功能在后来又添加很多，详细见具体文件）

核心文件：arch\mips\kernel\syscall.S arch\mips\kernel\entry.S kernel\syscall\syscall.c (具体调用关系见该文件，因为系统调用涉及被调用的文件过多，不再列出)

6.终端的实现，实现用户命令输入的读取、解析、显示，根据用户输入调用相关系统调用。

核心文件：test\test_shell.c

7.实现对进程的管理操作

核心文件：test\test_shell.c kernel\shell\shell.c

8.实现进程间同步

kernel\locking\barrier.c kernel\locking\cond.c kernel\locking\sem.c

9.实现进程间通信

libs\mailbox.c


更新说明
---

2019年1月6日

更新了文件系统的内容，做了课程的大BONUS，应该是满分了。

更新说明懒得写了，等考完试之后再补充吧。

2018年12月16日

更新了网卡的驱动程序，实现网卡驱动的编写，通过DMA进行数据的传送。收发包的实现，以及有网卡中断的多线程收发包。

2018年12月6日

更新了内存管理的部分，实现了页表和TLB内容。每个进程的页表大小目前为256项，可访问物理空间1MB。开发板可用物理内存一共为32MB。

用户进程的栈被放置在通过tlb访问的地址空间，在试图访问超过自身地址空间的内容时会报错。没有初始化页表，实现了缺页的处理机制和按需调页的功能。

存在的问题
---
还未成功实现和SD卡上swap区的内存交换，读写函数似乎出了一些问题

打印上之前留存的问题还需要再解决，打印刷新的机制需要调整。

开发环境
---
gcc-4.3-ls232 交叉编译工具

串口相关驱动已经附加在linux内核中，无需额外的驱动

安装minicom串口调试软件

硬件环境：采用国产龙芯1C处理器的开发板（MIPS处理器）



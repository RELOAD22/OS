#include "lock.h"
#include "sched.h"
#include "common.h"
#include "screen.h"
#include "syscall.h"

void system_call_helper(int fn, int arg1, int arg2, int arg3)
{
    //syscall[fn](arg1, arg2, arg3);
	current_running->user_context.regs[2] = syscall[fn](arg1, arg2, arg3);
}

void sys_sleep(int time)
{   
    invoke_syscall(SYSCALL_SLEEP, time, IGNORE, IGNORE);
}

void sys_block(queue_t *queue)
{
    invoke_syscall(SYSCALL_BLOCK, (int)queue, IGNORE, IGNORE);
}

void sys_unblock_one(queue_t *queue)
{
    invoke_syscall(SYSCALL_UNBLOCK_ONE, (int)queue, IGNORE, IGNORE);
}

void sys_unblock_all(queue_t *queue)
{
    invoke_syscall(SYSCALL_UNBLOCK_ALL, (int)queue, IGNORE, IGNORE);
}

void sys_write(char *buff)
{
    invoke_syscall(SYSCALL_WRITE, (int)buff, IGNORE, IGNORE);
}

void sys_reflush()
{
    invoke_syscall(SYSCALL_REFLUSH, IGNORE, IGNORE, IGNORE);
}

void sys_move_cursor(int x, int y)
{
    invoke_syscall(SYSCALL_CURSOR, x, y, IGNORE);
}

void mutex_lock_init(mutex_lock_t *lock)
{
    invoke_syscall(SYSCALL_MUTEX_LOCK_INIT, (int)lock, IGNORE, IGNORE);
}

void mutex_lock_acquire(mutex_lock_t *lock)
{
    do{
        invoke_syscall(SYSCALL_MUTEX_LOCK_ACQUIRE, (int)lock, IGNORE, IGNORE);
    }while(find_in_lockarray(lock) != lock);
}

void mutex_lock_release(mutex_lock_t *lock)
{
    invoke_syscall(SYSCALL_MUTEX_LOCK_RELEASE, (int)lock, IGNORE, IGNORE);
}

void sys_ps()
{
    invoke_syscall(SYSCALL_PS, IGNORE, IGNORE, IGNORE);    
}

void sys_exit()
{
    invoke_syscall(SYSCALL_EXIT, IGNORE, IGNORE, IGNORE);    
}

void sys_waitpid(int pid)
{
    invoke_syscall(SYSCALL_WAIT, pid, IGNORE, IGNORE);    
}

void sys_spawn(task_info_t *task)
{
    invoke_syscall(SYSCALL_SPAWN, task, IGNORE, IGNORE);    
}

void sys_kill(int pid)
{
    invoke_syscall(SYSCALL_KILL, pid, IGNORE, IGNORE);    
}

int sys_getpid()
{
    invoke_syscall(SYSCALL_GETPID, IGNORE, IGNORE, IGNORE);    
}

void semaphore_init(semaphore_t *s, int val)
{
    invoke_syscall(SYSCALL_SEMAPHORE_INIT, s, val, IGNORE);
}

void semaphore_up(semaphore_t *s)
{
    invoke_syscall(SYSCALL_SEMAPHORE_UP, s, IGNORE, IGNORE);
}

void semaphore_down(semaphore_t *s)
{
    invoke_syscall(SYSCALL_SEMAPHORE_DOWN, s, IGNORE, IGNORE);
}

void condition_init(condition_t *condition)
{
    invoke_syscall(SYSCALL_CONDITION_INIT, condition, IGNORE, IGNORE);
}

void condition_wait(mutex_lock_t *lock, condition_t *condition)
{
    invoke_syscall(SYSCALL_CONDITION_WAIT, lock, condition, IGNORE);
    mutex_lock_acquire(lock);
}

void condition_signal(condition_t *condition)
{
    invoke_syscall(SYSCALL_CONDITION_SIGNAL, condition, IGNORE, IGNORE);
}

void condition_broadcast(condition_t *condition)
{
    invoke_syscall(SYSCALL_CONDITION_BROADCAST, condition, IGNORE, IGNORE);
}

void barrier_init(barrier_t *barrier, int goal)
{
    invoke_syscall(SYSCALL_BARRIER_INIT, barrier, goal, IGNORE);
}

void barrier_wait(barrier_t *barrier)
{
    invoke_syscall(SYSCALL_BARRIER_WAIT, barrier, IGNORE, IGNORE);
}

void sys_init_mac()
{
    invoke_syscall(SYSCALL_INIT_MAC, IGNORE, IGNORE, IGNORE); 
}

uint32_t sys_net_recv(uint32_t rd, uint32_t rd_phy, uint32_t daddr)
{
    invoke_syscall(SYSCALL_NET_RECV, rd, rd_phy, daddr); 
}

void sys_net_send(uint32_t td, uint32_t td_phy)
{
    invoke_syscall(SYSCALL_NET_SEND, td, td_phy, IGNORE); 
}

void sys_wait_recv_package()
{
    invoke_syscall(SYSCALL_WAIT_RECV_PACKAGE, IGNORE, IGNORE, IGNORE); 
}

void sys_mkfs()
{
    invoke_syscall(SYSCALL_MKFS, IGNORE, IGNORE, IGNORE); 
}

void sys_statfs()
{
    invoke_syscall(SYSCALL_STATFS, IGNORE, IGNORE, IGNORE); 
}

void sys_ls()
{
    invoke_syscall(SYSCALL_LS, IGNORE, IGNORE, IGNORE); 
}

void sys_mkdir()
{
    invoke_syscall(SYSCALL_MKDIR, IGNORE, IGNORE, IGNORE); 
}

void sys_cd()
{
    invoke_syscall(SYSCALL_CD, IGNORE, IGNORE, IGNORE); 
}

void sys_rmdir()
{
    invoke_syscall(SYSCALL_RMDIR, IGNORE, IGNORE, IGNORE); 
}

void sys_touch()
{
    invoke_syscall(SYSCALL_TOUCH, IGNORE, IGNORE, IGNORE); 
}

void sys_cat()
{
    invoke_syscall(SYSCALL_CAT, IGNORE, IGNORE, IGNORE); 
}

int sys_fopen(char *name, int access)
{
    invoke_syscall(SYSCALL_FOPEN, name, access, IGNORE); 
}

int sys_fwrite(int fd, char *buff, int size)
{
    invoke_syscall(SYSCALL_FWRITE, fd, buff, size); 
}

int sys_fread(int fd, char *buff, int size)
{
    invoke_syscall(SYSCALL_FREAD, fd, buff, size); 
}

void sys_fclose(int fd)
{
    invoke_syscall(SYSCALL_FCLOSE, fd, IGNORE, IGNORE); 
}
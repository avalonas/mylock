#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<errno.h>
#include<linux/futex.h>
#include<sys/syscall.h>

#define futex(addr1, op, val, rel, addr2, val3)	\
syscall(SYS_futex, addr1, op, val, rel, addr2, val3)

typedef struct lclock
{
	int status;		// 当前状态
}loadlock;

typedef struct share_use
{
	int S,W,T;
	int buffer[10000];
}share;


share* sdata;			//共享数据

void init_lock(loadlock* lock)
{
	lock->status = 1;
	int shmid = shmget((key_t)ftok(".",2) , sizeof(share), 0666|IPC_CREAT);
	if(shmid == -1)
	{
		fprintf(stderr,"共享内存创建失败\n");
		printf("%s\n",strerror(errno));
		exit(1);
	}

	sdata = shmat(shmid,NULL,0);
	if(sdata == (void*)-1)
	{
		fprintf(stderr,"启用共享内存失败\n");
		exit(2);
	}
}


static inline int try_lock(loadlock* lock)
{
	/*
	char oldval; 
  __asm__ __volatile__( 
    "xchgb %b0,%1"
    :"=q" (oldval), "=m" (lock->status) 
    :"0" (0) : "memory"); 
  return oldval > 0;
  */
	return __sync_bool_compare_and_swap(&(lock->status),1,0);	//
}

void acquire_lock(loadlock* lock)
{
	while(!try_lock(lock))
	{
		if((sdata->S - sdata->W) >= sdata->T)
			continue;
		else
		{
			int tempS = sdata->S;
			if(__sync_bool_compare_and_swap(&(sdata->S),tempS,tempS+1))
			{
				tempS = tempS + 1;
				sdata->buffer[tempS % 10000] = 1;
				struct timespec tmp;
				tmp.tv_nsec = 10000000;
				if(try_lock(lock))
				{
					sdata->buffer[tempS %10000] = 0;
					__sync_fetch_and_add(&(sdata->W),1);
					return;
				}
				if(sdata->buffer[tempS % 10000] == 1)
					futex(sdata->buffer + tempS % 10000, FUTEX_WAIT, 1, &tmp, 0, 0);
				__sync_fetch_and_add(&(sdata->W),1);
				
			}
			else
				continue;
		}	
	}
	return;			//获得
}

void release_lock(loadlock* lock)
{
	__sync_lock_test_and_set(&(lock->status),1);
}

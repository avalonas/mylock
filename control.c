#include<stdio.h>
#include<stdlib.h>
#include<sys/time.h>
#include<unistd.h>
#include<sched.h>
#include<string.h>
#include<pthread.h>
#include<sys/shm.h>
#include<sys/ipc.h>
#include<sys/types.h>

#include<linux/futex.h>
#include<sys/syscall.h>
#define NCPU 8


#define futex(addr1, op, val, rel, addr2, val3)	\
syscall(SYS_futex, addr1, op, val, rel, addr2, val3)

typedef struct share_use
{
	int S,W,T;
	int buffer[10000];
}share;

share* sdata;
int tempT;
FILE* file;

void read_usage()
{
	char buf[200];
	char running[22];

	file = fopen("/proc/stat","r");
	if(file == NULL)
	{
		fprintf(stderr,"打开stat文件失败\n");
	}

	while(fgets(buf,200,file))
	{
		if(strstr(buf,"procs_running"))
		{
			sscanf(buf,"%s %d",running, &tempT);
		}
	}
	fclose(file);
}
int main()
{
	struct timespec slptm;
	struct timeval start,end;

	slptm.tv_sec = 0;
	slptm.tv_nsec = 10000;

	sdata = NULL;
	int shmid = shmget((key_t)ftok(".",2), sizeof(share), 0666|IPC_CREAT);
	if(shmid == -1)
	{
		fprintf(stderr,"共享内存创建失败\n");
		exit(1);
	}

	sdata = shmat(shmid,NULL,0);
	if(sdata == (void*)-1)
	{
		fprintf(stderr,"启用共享内存失败\n");
		exit(2);
	}


	{
		sdata->T = 0;
		sdata->S = 0;
		sdata->W = 0;
	}

	int temp;
	int lastknow = 1;	//slot 从1开始放
	for(;;)
	{
		printf("S:%d  W:%d  T:%d  \n", sdata->S,sdata->W,sdata->T);
		read_usage();
		if(tempT > NCPU)
		{
			temp = sdata->T;
			sdata->T = tempT - NCPU;
			if(temp > sdata->T)
			{
				/*
				int i,j;
				int tt = sdata->T;
				int ss = sdata->S;
				for(i = lastknow, j = 0; j < (temp - tt) && i <= ss; i++)
				{
					if(sdata->buffer[i % 10000] == 1)
					{
						j++;
						sdata->buffer[i % 10000] = 0;
						futex(sdata->buffer + i % 10000, FUTEX_WAKE, 1, 0, 0, 0);
					}
				}
				lastknow = i;
				*/
				int i;
				for(i = 0; i < 10000; ++i)
				{
					if(sdata->buffer[i] == 1)
					{
						sdata->buffer[i] = 0;
						futex(sdata->buffer + i, FUTEX_WAKE, 1, 0, 0, 0);
					}
				}
			}
		}
		nanosleep(&slptm,NULL);
	}
	
	return 0;
}

#include "lclock.h"
#include <time.h>

#define PASS 1000000
//#define NOT_USE 1

#ifndef NOT_USE
pthread_mutex_t spinlock;
#else
loadlock tmplock;
#endif

int counter = 0;

void* consumer(void* arg)
{
//	int counter = 0;

	int tmp = 1;	
	while(1)
	{
#ifdef NOT_USE
		acquire_lock(&tmplock);
#else
		pthread_mutex_lock(&spinlock);
#endif
		if(tmp > PASS)
		{
#ifdef NOT_USE
			release_lock(&tmplock);
#else
			pthread_mutex_unlock(&spinlock);
#endif
			return;
		}
		tmp++;
		counter = counter + 1;
#ifdef NOT_USE
		release_lock(&tmplock);
#else
		pthread_mutex_unlock(&spinlock);
#endif
	}
	return;
}

int main()
{
	double timesum = 0;
	pthread_t *thread_array;
	

	int pass;
	for(pass = 1; pass <= 32; ++pass)
	{
#ifdef NOT_USE
		init_lock(&tmplock);
#else
		pthread_mutex_init(&spinlock,NULL);
#endif
		struct timespec time1,time2;
		memset(&time1,0,sizeof(time1));
		memset(&time2,0,sizeof(time2));

		counter = 0;
		thread_array = malloc(32 * sizeof(pthread_t));
		memset(thread_array,0,sizeof(thread_array));
		int j;
		clock_gettime(CLOCK_MONOTONIC, &time1);
		for(j = 0; j <pass; ++j)
		{
			if(pthread_create(&thread_array[j], NULL, (void*)consumer, NULL))
			{
				printf("error create!\n  %d", j);
			}
		}


		int t;
		for(t = 0; t <pass; ++t)
		{
			pthread_join(thread_array[t], NULL);
		}
		clock_gettime(CLOCK_MONOTONIC, &time2);
		
	
		
		printf("pass:%d time:%.6lf \n",pass,  (time2.tv_sec - time1.tv_sec) * 1000000000.0 + (time2.tv_nsec - time1.tv_nsec));
		timesum += (time2.tv_sec - time1.tv_sec) * 1000000000.0 + (time2.tv_nsec - time1.tv_nsec);
	
		if(counter == pass* PASS)
			printf("right\n");
		else
			printf("wrong\n");
		free(thread_array);
	}

		return 0;
}

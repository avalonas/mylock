#include "lclock.h"
#include <time.h>

//#define NOT_USE 1

#ifndef NOT_USE
pthread_mutex_t spinlock;
#else
loadlock tmplock;
#endif


void* consumer(void* arg)
{
	int counter = 0;

	
	while(1)
	{
#ifdef NOT_USE
		acquire_lock(&tmplock);
#else
		pthread_mutex_lock(&spinlock);
#endif
		if(counter > 1000000)
		{
#ifdef NOT_USE
			release_lock(&tmplock);
#else
			pthread_mutex_unlock(&spinlock);
#endif
			return;
		}
		counter++;
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
		//pthread_mutex_init(&spinlock);
#endif
		struct timespec time1,time2;
		memset(&time1,0,sizeof(time1));
		memset(&time2,0,sizeof(time2));

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

		free(thread_array);
	}

		return 0;
}

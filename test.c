#include<stdlib.h>
#include<stdio.h>
#include"lclock.h"
int main()
{
	loadlock tmp;
	init_lock(&tmp);
	acquire_lock(&tmp);
	{
		printf("%d\n",sdata->W);
	}
	release_lock(&tmp);

	acquire_lock(&tmp);
	printf("111\n");
	release_lock(&tmp);
}

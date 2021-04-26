#include "libminiomp.h"
#include <stdlib.h>

// Called when encountering taskwait and taskgroup constructs
miniomp_taskqueue_t * miniomp_taskgroupqueue;

void
GOMP_taskwait (void)
{
    //printf("TBI: Entered in taskwait, there should be no pending tasks, so I proceed\n");
		
		runtasks(miniomp_taskqueue);
		// TODO aixo nomes assgura que totes les tasques hauran comencat, no que hagin acabat
		// IDEA en miniomp_taskqueue_t camp working initialized a -1, quan s'exeuta un taska, abans es llegeix el valor. 
		// s'escriu thread id, s'executa la task, i un cop acabada l'execucio es torna a escriure el primer valor llegit.
		// si no hi ha tasks al queue i el camp working == -1 esk s'han executat totes les tasks
}

void
GOMP_taskgroup_start (void)
{
    //printf("TBI: Starting a taskgroup region, at the end of which I should wait for tasks created here\n");
		// init tasgroup queue
		miniomp_taskgroupqueue = TQinit(MAXELEMENTS_TQ, 1);

		pthread_mutex_lock(&miniomp_taskqueue->lock_queue);
		miniomp_taskqueue->in_group = 1;
		pthread_mutex_unlock(&miniomp_taskqueue->lock_queue);
}

void
GOMP_taskgroup_end (void)
{
		pthread_mutex_lock(&miniomp_taskqueue->lock_queue);
		miniomp_taskqueue->in_group = 0;
		pthread_mutex_unlock(&miniomp_taskqueue->lock_queue);

		runtasks(miniomp_taskgroupqueue);
		TQfree(miniomp_taskgroupqueue);
    printf("TBI: Finished a taskgroup region, there should be no pending tasks, so I proceed\n");
}

#include "libminiomp.h"
#include <stdlib.h>

// Called when encountering taskwait and taskgroup constructs
miniomp_taskqueue_t * miniomp_taskgroupqueue;

void
GOMP_taskwait (void)
{
		pthread_cond_wait(&miniomp_taskqueue->cond_tsync, &miniomp_taskqueue->lock_tsync);
}

void
GOMP_taskgroup_start (void)
{
		// init tasgroup queue
		miniomp_taskgroupqueue = TQinit(MAXELEMENTS_TQ, 1);
		miniomp_taskgroupqueue->still_pushing = omp_get_thread_num();
		pthread_mutex_lock(&miniomp_taskqueue->lock_queue);
		miniomp_taskqueue->in_group = 1;
		pthread_mutex_unlock(&miniomp_taskqueue->lock_queue);
}

void
GOMP_taskgroup_end (void)
{
		miniomp_taskgroupqueue->still_pushing = -1;
		pthread_mutex_lock(&miniomp_taskqueue->lock_queue);
		miniomp_taskqueue->in_group = 0;
		pthread_mutex_unlock(&miniomp_taskqueue->lock_queue);
		pthread_cond_wait(&miniomp_taskgroupqueue->cond_tsync, &miniomp_taskgroupqueue->lock_tsync);

}

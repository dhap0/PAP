#include "libminiomp.h"

// Default lock for unnamed critical sections
pthread_mutex_t miniomp_default_lock;
pthread_mutex_t miniomp_named_lock;

void 
GOMP_critical_start (void) {
	pthread_mutex_lock(&miniomp_default_lock);
//  printf("TBI: Entering an unnamed critical, don't know if anyone else is alrady in. I proceed\n");
}

void 
GOMP_critical_end (void) {
	pthread_mutex_unlock(&miniomp_default_lock);
 // printf("TBI: Exiting an unnamed critical section. I can not inform anyone else, bye!\n");
}

void 
GOMP_critical_name_start (void **pptr) {
  //pthread_mutex_t * plock = *pptr;
  // if plock is NULL it means that the lock associated to the name has not yet been allocated and initialized
	pthread_mutex_lock(&miniomp_named_lock);
	if (*pptr == NULL) {
		*pptr = malloc(sizeof(pthread_mutex_t)); 		
		pthread_mutex_init(*pptr, NULL);
	}
	pthread_mutex_unlock(&miniomp_named_lock);
	pthread_mutex_lock(*pptr);
  //printf("TBI: Entering a named critical %p (%p), don't know if anyone else is alrady in. I proceed %d\n", pptr, *pptr, omp_get_thread_num());
}

void 
GOMP_critical_name_end (void **pptr) {
  pthread_mutex_t * plock = *pptr;
  // if plock is still NULL something went wrong
  //printf("TBI: Exiting a named critical %p (%p), I can not inform anyone else, bye! %d\n", pptr, plock, omp_get_thread_num());
	pthread_mutex_unlock(plock);
	
}

// Default barrier within a parallel region
pthread_barrier_t miniomp_barrier;

void 
GOMP_barrier() {
  //printf("TBI: Entering in barrier, but do not know how to wait for the rest. I proceed\n");
  printf("estic aki\n");
	pthread_barrier_wait(&miniomp_barrier);
	runtasks();
}

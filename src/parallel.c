#include "libminiomp.h"

// This file implements the PARALLEL construct as part of your 
// miniomp runtime library. parallel.h contains definitions of 
// data types used here

// Declaration of array for storing pthread identifier from 
// pthread_create function
pthread_t *miniomp_threads;

// Global variable for parallel descriptor
miniomp_parallel_t *miniomp_parallel;

// Declaration of per-thread specific key
pthread_key_t miniomp_specifickey;

// This is the prototype for the Pthreads starting function
void *
worker(void *args) {
  // insert all necessary code here for:
	miniomp_parallel_t *info = (miniomp_parallel_t *)args;
  //   1) save thread-specific data
	pthread_setspecific(miniomp_specifickey, (void *) (intptr_t)info->id);
  //   2) invoke the per-threads instance of function encapsulating the parallel region
	info->fn(info->fn_data);

  //   3) exit the function
	runtasks();	
  pthread_exit(NULL);
}

void
GOMP_parallel (void (*fn) (void *), void *data, unsigned num_threads, unsigned int flags) {
  if(!num_threads) num_threads = omp_get_num_threads();
  printf("Starting a parallel region using %d threads\n", num_threads);
	int nthreads = omp_get_num_threads();
	omp_set_num_threads(num_threads);
	pthread_barrier_init(&miniomp_barrier, NULL, num_threads);
	pthread_mutex_init(&miniomp_single.lock, NULL);	
	pthread_barrier_init(&miniomp_single.barrier, NULL, num_threads);	

	miniomp_parallel = malloc(sizeof(miniomp_parallel_t)*num_threads-1);				
	miniomp_threads = malloc(sizeof(pthread_t)*num_threads-1);				

  for (int i=0; i<num_threads-1; i++) {
		miniomp_parallel[i].fn = fn; 	
		miniomp_parallel[i].fn_data = data;
		miniomp_parallel[i].id = i+1;
		pthread_create(&miniomp_threads[i], NULL, worker, (void *)&miniomp_parallel[i]);	
	}
	
	fn(data);

	runtasks();	
	for (int i=0; i<num_threads-1; i++) {
		pthread_join(miniomp_threads[i], NULL);
	}
	omp_set_num_threads(nthreads);
}

#include "libminiomp.h"

// Declaratiuon of global variable for single work descriptor
miniomp_single_t miniomp_single;

// This routine is called when first encountering a SINGLE construct. 
// Returns true if this is the thread that should execute the clause. 

bool
GOMP_single_start (void) {
  //printf("TBI: Entering into single, don't know if anyone else arrived before, I proceed\n");
	bool val;
	pthread_mutex_lock(&miniomp_single.lock);
	if (miniomp_single.count == 0){
		val = true;
		miniomp_taskqueue->still_pushing = omp_get_thread_num();
	}
	else
		val = false;
	miniomp_single.count++;
	pthread_mutex_unlock(&miniomp_single.lock);
	pthread_barrier_wait(&miniomp_single.barrier);	
	miniomp_single.count = 0;
	if(!val)
		runtasks();
	
  return(val);
}

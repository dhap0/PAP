#include "libminiomp.h"

miniomp_taskqueue_t * miniomp_taskqueue;

// Initializes the task queue
miniomp_taskqueue_t *TQinit(int max_elements, int in_group) {
		miniomp_taskqueue_t *taskqueue = malloc(sizeof(miniomp_taskqueue_t));
		taskqueue->max_elements = (max_elements > 0)? max_elements: MAXELEMENTS_TQ;
		taskqueue->count = -1;
		taskqueue->head = -1;
		taskqueue->in_group = in_group;
		pthread_cond_init(&taskqueue->cond, NULL);
		pthread_mutex_init(&taskqueue->lock_cond, NULL);
		pthread_mutex_init(&taskqueue->lock_queue, NULL);
		taskqueue->queue = calloc(max_elements, sizeof(miniomp_task_t *));
		return taskqueue;
		
}


void TQfree(miniomp_taskqueue_t * taskqueue) {
		pthread_cond_destroy(&taskqueue->cond);
		pthread_mutex_destroy(&taskqueue->lock_cond);
		pthread_mutex_destroy(&taskqueue->lock_queue);
		free(taskqueue->queue);
		free(taskqueue);

}

// Checks if the task queue is empty
bool TQis_empty(miniomp_taskqueue_t *task_queue) {
    return task_queue->count == -1;
}

// Checks if the task queue is full
bool TQis_full(miniomp_taskqueue_t *task_queue) {
    return task_queue->count == task_queue->max_elements - 1;
}

// Enqueues the task descriptor at the tail of the task queue
bool TQenqueue(miniomp_taskqueue_t *task_queue, miniomp_task_t *task_descriptor) {
		
		if (TQis_full(task_queue))
		{
			// block untill task_queue not full
			pthread_cond_wait(&task_queue->cond, &task_queue->lock_cond);
		}
		pthread_mutex_lock(&task_queue->lock_queue);
		(task_queue->head)++;	
		(task_queue->count)++;	
		task_queue->queue[task_queue->head] = task_descriptor;
		pthread_mutex_unlock(&task_queue->lock_queue);
    return true;
}

// Dequeue the task descriptor at the head of the task queue
// MUST be called from TQfirst or using mutex_lock, otherwise prone to double-free or corruption
bool TQdequeue(miniomp_taskqueue_t *task_queue) { 
		if(TQis_empty(task_queue) )
		{
			return false;
		} else
		{
			free(task_queue->queue[task_queue->head]);
			(task_queue->head)--;	
			(task_queue->count)--;	
			
		}
    return true;
}

// Returns false if TQ empty, otherwise fill first with the head task of the task queue
bool TQfirst(miniomp_taskqueue_t *task_queue, miniomp_task_t *first) {
			bool was_full;
		// if queue es full must send signal to unlock pusher
		pthread_mutex_lock(&task_queue->lock_queue);
		if(TQis_empty(task_queue)){
			pthread_mutex_unlock(&task_queue->lock_queue);
			return false;
		} else {

			was_full = TQis_full(task_queue);
			*first = *(task_queue->queue[task_queue->head]);
			TQdequeue(task_queue);
			if (was_full) {
				// unlock pusher
				pthread_cond_broadcast(&task_queue->cond);
			}	
			pthread_mutex_unlock(&task_queue->lock_queue);
			return true;
		}
}

void runtasks(miniomp_taskqueue_t * taskqueue) {
	miniomp_task_t task;
	while (TQfirst(taskqueue, &task)) {
			task.fn(task.data);
	}
}

#define GOMP_TASK_FLAG_UNTIED           (1 << 0)
#define GOMP_TASK_FLAG_FINAL            (1 << 1)
#define GOMP_TASK_FLAG_MERGEABLE        (1 << 2)
#define GOMP_TASK_FLAG_DEPEND           (1 << 3)
#define GOMP_TASK_FLAG_PRIORITY         (1 << 4)
#define GOMP_TASK_FLAG_UP               (1 << 8)
#define GOMP_TASK_FLAG_GRAINSIZE        (1 << 9)
#define GOMP_TASK_FLAG_IF               (1 << 10)
#define GOMP_TASK_FLAG_NOGROUP          (1 << 11)
#define GOMP_TASK_FLAG_REDUCTION        (1 << 12)

// Called when encountering an explicit task directive. Arguments are:
//      1. void (*fn) (void *): the generated outlined function for the task body
//      2. void *data: the parameters for the outlined function
//      3. void (*cpyfn) (void *, void *): copy function to replace the default memcpy() from 
//                                         function data to each task's data
//      4. long arg_size: specify the size of data
//      5. long arg_align: alignment of the data
//      6. bool if_clause: the value of if_clause. true --> 1, false -->0; default is set to 1 by compiler
//      7. unsigned flags: see list of the above

void
GOMP_task (void (*fn) (void *), void *data, void (*cpyfn) (void *, void *),
           long arg_size, long arg_align, bool if_clause, unsigned flags,
           void **depend, int priority)
{
   // printf("TBI: a task has been encountered, I am executing it immediately\n");

    // This part of the code appropriately copies data to be passed to task function,
    // either using a compiler cpyfn function or just memcopy otherwise; no need to
    // fully understand it for the purposes of this assignment
    char *arg;
    if (__builtin_expect (cpyfn != NULL, 0)) {
	  char *buf =  malloc(sizeof(char) * (arg_size + arg_align - 1));
          arg       = (char *) (((uintptr_t) buf + arg_align - 1)
                               & ~(uintptr_t) (arg_align - 1));
          cpyfn (arg, data);
    } else {
          arg       =  malloc(sizeof(char) * (arg_size + arg_align - 1));
          memcpy (arg, data, arg_size);
    }

    // Function invocation should be replaced with the appropriate task instatiation

		miniomp_task_t * new_task = malloc(sizeof(miniomp_task_t));
		new_task->fn = fn;
		new_task->data = arg;
		pthread_mutex_lock(&miniomp_taskqueue->lock_queue);
		int in_group = miniomp_taskqueue->in_group;
		pthread_mutex_unlock(&miniomp_taskqueue->lock_queue);
		if(in_group)
			TQenqueue(miniomp_taskgroupqueue, new_task);
		else
			TQenqueue(miniomp_taskqueue, new_task);
		
	
}

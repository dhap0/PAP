/* This structure describes a "task" to be run by a thread.  */
typedef struct {
    void (*fn)(void *);
    void (*data);
    // complete with additional field if needed
} miniomp_task_t;

typedef struct {
    int max_elements;
    int count;
    int head;
		int in_group; // 1 in taskgroup region
		int still_pushing; // -1 if no more tasks to be pushed
		int busy_count; // threads executind tasks
		pthread_cond_t	cond_full; 
		pthread_cond_t	cond_tsync; 
		pthread_mutex_t lock_full;
		pthread_mutex_t lock_tsync;
    pthread_mutex_t lock_queue;
    miniomp_task_t **queue;
    // complete with additional field if needed
} miniomp_taskqueue_t;

extern miniomp_taskqueue_t * miniomp_taskqueue;
extern miniomp_taskqueue_t * miniomp_taskgroupqueue;
#define MAXELEMENTS_TQ 128

// funtions to implement basic management operations on taskqueue
miniomp_taskqueue_t *TQinit(int max_elements, int in_group);
void TQfree(miniomp_taskqueue_t * taskqueue);
bool TQis_empty(miniomp_taskqueue_t *task_queue);
bool TQis_full(miniomp_taskqueue_t *task_queue) ;
bool TQenqueue(miniomp_taskqueue_t *task_queue, miniomp_task_t *task_descriptor); 
bool TQfirst(miniomp_taskqueue_t *task_queue, miniomp_task_t *first); 
void runtasks();

// Functions implemented in task* modules
void GOMP_task (void (*fn) (void *), void *data, void (*cpyfn) (void *, void *),
           long arg_size, long arg_align, bool if_clause, unsigned flags,
           void **depend, int priority);
void GOMP_taskloop (void (*fn) (void *), void *data, void (*cpyfn) (void *, void *),
               long arg_size, long arg_align, unsigned flags,
               unsigned long num_tasks, int priority,
               long start, long end, long step);
void GOMP_taskwait (void);
void GOMP_taskgroup_start (void);
void GOMP_taskgroup_end (void);
void GOMP_taskgroup_reduction_register (uintptr_t *data);
void GOMP_taskgroup_reduction_unregister (uintptr_t *data);
void GOMP_task_reduction_remap (size_t cnt, size_t cntorig, void **ptrs);


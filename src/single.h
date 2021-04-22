// Type declaration for single work sharing descriptor
typedef struct {
		pthread_mutex_t lock;
		pthread_barrier_t barrier;
		int 						count;
    // complete the definition of single descriptor
} miniomp_single_t;

// Declaration of global variable for single work descriptor
extern miniomp_single_t miniomp_single;

// Functions implemented in this module
bool GOMP_single_start (void);

#ifndef __RECURSIVE_MUTEX__
#define __RECURSIVE_MUTEX__

#include <pthread.h>

typedef struct recursive_mutex {
    /* # of self duplicate locks */
    int self_locked_count;
    /* # of other waiting threads */
    int waiting_threads_count;
    /* the holder thread of the lock */
    pthread_t locking_thread;
    pthread_mutex_t mutex;
    pthread_cond_t cv;
} recursive_mutex;

recursive_mutex *recursive_mutex_init(void);
void recursive_mutex_lock(recursive_mutex *rec_mutex);
void recursive_mutex_unlock(recursive_mutex *rec_mutex);
void recursive_mutex_destroy(recursive_mutex *rec_mutex);

#endif

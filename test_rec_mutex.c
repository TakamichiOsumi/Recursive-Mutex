#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include "rec_mutex.h"

/*
 * <FIRST TEST>
 */
static void
delete_index(recursive_mutex *rm, int **array, int index){
    recursive_mutex_lock(rm);
    array[index] = NULL;
    recursive_mutex_unlock(rm);
}

static void
delete_array(recursive_mutex *rm, int **array, int size){
    int i;

    recursive_mutex_lock(rm);
    for (i = 0; i < size; i++){
	delete_index(rm, array, i);
    }
    recursive_mutex_unlock(rm);
}

static void
check_array(recursive_mutex *rm, int **array, int size){
    int i;

    recursive_mutex_lock(rm);
    for (i = 0; i < size; i++){
	if (array[i] != NULL){
	    fprintf(stderr,
		    "array[%d]=%lu has not been deleted yet\n",
		    i, (uintptr_t) array[i]);
	    exit(-1);
	}
    }
    recursive_mutex_unlock(rm);
}

static void
self_duplicate_lock_test(void){
    recursive_mutex *rm;
    int *array[] = { (void *) 1, (void *) 2, (void *) 3,
		     (void *) 4, (void *) 5 };

    rm = recursive_mutex_init();
    delete_array(rm, array, 5);
    check_array(rm, array, 5);
    recursive_mutex_destroy(rm);
}

/*
 * <SECOND TEST> (which utilizes the <FIRST TEST>)
 */
typedef struct thread_unique {
    /* Data hold by each thread */
    int thread_id;
    /* Data shared among all the threads */
    recursive_mutex *rec_mutex;
} thread_unique;

static void *
lock_first_thread_cb(void *arg){
    thread_unique *unique = (thread_unique *) arg;
    int *array[] = { (void *) 1, (void *) 2, (void *) 3,
		     (void *) 4, (void *) 5 };

    recursive_mutex_lock(unique->rec_mutex);

    /*
     * Calling sleep is the simple way to let other sub threads go first.
     * See wait_thread_cb. They will get blocked since the mutex lock is
     * taken by this thread and will increment the waiting counts.
     * They resume its execution after the test of this thread.
     */
    sleep(5);

    /* <FIRST TEST> */
    delete_array(unique->rec_mutex, array, 5);
    check_array(unique->rec_mutex, array, 5);

    recursive_mutex_unlock(unique->rec_mutex);

    free(arg);

    return NULL;
}

static void *
wait_thread_cb(void *arg){
    thread_unique *unique = (thread_unique *) arg;

    sleep(1);

    recursive_mutex_lock(unique->rec_mutex);
    /* Do nothing. Expect those threads to wait */
    recursive_mutex_unlock(unique->rec_mutex);

    free(arg);

    return NULL;
}

static void
waiting_multiple_threads_test(void){
#define THREADS_TOTAL_NO 8

    pthread_t handlers[THREADS_TOTAL_NO];
    recursive_mutex* shared;
    int i;

    if ((shared = malloc(sizeof(recursive_mutex))) == NULL){
	perror("malloc");
	exit(-1);
    }

    /* Prepare shared data */
    shared = recursive_mutex_init();

    for (i = 0; i < THREADS_TOTAL_NO; i++){
	thread_unique *unique;

	if ((unique = malloc(sizeof(thread_unique))) == NULL){
	    perror("malloc");
	    exit(-1);
	}

	unique->thread_id = i;
	unique->rec_mutex = shared;
	
	if (i == 1){
	    if (pthread_create(&handlers[i], NULL,
			       lock_first_thread_cb, (void *) unique) != 0){
		perror("pthread_create");
		exit(-1);
	    }
	}else{
	    if (pthread_create(&handlers[i], NULL,
			       wait_thread_cb, (void *) unique) != 0){
		perror("pthread_create");
		exit(-1);
	    }
	}
    }

    for (i = 0; i < THREADS_TOTAL_NO; i++)
	pthread_join(handlers[i], NULL);

    recursive_mutex_destroy(shared);
}

int
main(int argc, char **argv){

    printf("<Tests for self duplicate lock>\n");
    self_duplicate_lock_test();

    printf("<Tests for waiting multiple threads>\n");
    waiting_multiple_threads_test();

    return 0;
}

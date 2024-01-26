#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "rec_mutex.h"

recursive_mutex *
recursive_mutex_init(void){
    recursive_mutex *rec_mutex;

    if ((rec_mutex = (recursive_mutex *)
	 malloc(sizeof(recursive_mutex))) == NULL){
	perror("malloc");
	exit(-1);
    }

    rec_mutex->self_locked_count = 0;
    rec_mutex->waiting_threads_count = 0;

    if (pthread_cond_init(&rec_mutex->cv, NULL) != 0){
	perror("pthread_cond_init");
	exit(-1);
    }
    if (pthread_mutex_init(&rec_mutex->mutex, NULL) != 0){
	perror("pthread_mutex_init");
	exit(-1);
    }

    return rec_mutex;
}

void
recursive_mutex_lock(recursive_mutex *rec_mutex){
    pthread_mutex_lock(&rec_mutex->mutex);
    if (rec_mutex->locking_thread == pthread_self()){
	rec_mutex->self_locked_count++;
	printf("[%s] Incremented self lock count. Count = %d\n",
	       __FUNCTION__, rec_mutex->self_locked_count);
    }else{
	printf("[%s] thread (id=%p) has started to wait\n",
	       __FUNCTION__, pthread_self());
	while(rec_mutex->locking_thread){
	    rec_mutex->waiting_threads_count++;
	    printf("[%s] thread (id=%p) has become in wait state and made waiting threads count = %d\n",
		   __FUNCTION__, pthread_self(), rec_mutex->waiting_threads_count);
	    if (pthread_cond_wait(&rec_mutex->cv,
				  &rec_mutex->mutex) != 0){
		perror("pthread_cond_wait");
		exit(-1);
	    }
	    rec_mutex->waiting_threads_count--;
	    printf("[%s] thread (id=%p) has become in exec state and made waiting threads count = %d\n",
		   __FUNCTION__, pthread_self(), rec_mutex->waiting_threads_count);
	}
	printf("[%s] Waiting thread (id=%p) gets the initiative\n",
	       __FUNCTION__, pthread_self());

	assert(rec_mutex->self_locked_count == 0);
	assert(rec_mutex->locking_thread == NULL);

	/*
	 * The count will become zero in
	 * the corresponding recursive_mutex_unlock().
	 */
	rec_mutex->self_locked_count = 1;
	printf("[%s] %p set self_locked_count = 1\n",
	       __FUNCTION__, pthread_self());
	rec_mutex->locking_thread = pthread_self();
	printf("[%s] %p set locking_thread = self\n",
	       __FUNCTION__, pthread_self());
    }
    pthread_mutex_unlock(&rec_mutex->mutex);
}

void
recursive_mutex_unlock(recursive_mutex *rec_mutex){
    pthread_mutex_lock(&rec_mutex->mutex);
    assert(rec_mutex->locking_thread == pthread_self());
    assert(rec_mutex->self_locked_count >= 1);

    rec_mutex->self_locked_count--;
    printf("[%s] Decremented self lock count. Count = %d\n",
	   __FUNCTION__, rec_mutex->self_locked_count);

    if (rec_mutex->self_locked_count == 0){
	rec_mutex->locking_thread = NULL;
	if (rec_mutex->waiting_threads_count > 0){
	    if (pthread_cond_signal(&rec_mutex->cv) != 0){
		perror("pthread_cond_signal");
		exit(-1);
	    }
	}
    }
    pthread_mutex_unlock(&rec_mutex->mutex);
}

void
recursive_mutex_destroy(recursive_mutex *rec_mutex){
    assert(rec_mutex->self_locked_count == 0 &&
	   rec_mutex->waiting_threads_count == 0 &&
	   rec_mutex->locking_thread == NULL);
    pthread_mutex_destroy(&rec_mutex->mutex);
    pthread_cond_destroy(&rec_mutex->cv);
}

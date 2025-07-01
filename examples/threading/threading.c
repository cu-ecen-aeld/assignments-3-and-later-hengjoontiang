#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <errno.h>


// Optional: use these functions to add debug or error prints to your application
#define DEBUG_LOG(msg,...)
//#define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("threading ERROR: " msg "\n" , ##__VA_ARGS__)

//use global for the mutex
//pthread_mutex_t mutex = PThREAD_MUTEXT_INITIALIZER;
/*
Understand the thread lifecycle -- IMPORTANT
https://www.linkedin.com/pulse/basic-understanding-threads-practical-guide-posix-c-mohit-mishra-sghcc

*/
/*
type of mutext will determine the blocking behavio0ur
https://pubs.opengroup.org/onlinepubs/009604499/functions/pthread_mutex_lock.html
*/
void* threadfunc(void* thread_param)
{

    // TODO: wait, obtain mutex, wait, release mutex as described by thread_data structure
    // hint: use a cast like the one below to obtain thread arguments from your parameter
    struct thread_data* thread_func_args = NULL;
    
    
    //retrieve wait time 1 and wait time 2
    int wait_to_obtain_ms = 0;
    
    int wait_to_release_ms = 0;
    
    pthread_mutex_t* mutex = NULL;
    
    
    
    if (thread_param){
      thread_func_args = (struct thread_data *) thread_param;
      wait_to_obtain_ms = thread_func_args->wait_to_obtain_ms;
      wait_to_release_ms = thread_func_args->wait_to_release_ms;
      mutex = thread_func_args->mutex ;
      thread_func_args->currentThread = pthread_self(); 
    }
    printf("In threadfunc\n");
    printf("wait_to_obtain_ms=%d\n",wait_to_obtain_ms);
    printf("wait_to_release_ms=%d\n",wait_to_release_ms);
    
    
    //wait for wait time1
    usleep(1000*wait_to_obtain_ms);
    printf("after wait_to_obtain_ms\n");
    //lock mutext
    //do nothing if > 0 ; correct behaviour
    int rc;
    //rc = pthread_mutex_trylock(mutex);
    rc = pthread_mutex_lock(mutex);
    /*
     EBUSY

    The mutex could not be acquired because the mutex pointed to by mutex was already locked.


EAGAIN

    The mutex could not be acquired because the maximum number of recursive locks for mutex has been exceeded.

If the symbol _POSIX_THREAD_PRIO_INHERIT is defined, the mutex is initialized with the protocol attribute value PTHREAD_PRIO_INHERIT, and the robustness argument of pthread_mutexattr_setrobust_np() is PTHREAD_MUTEX_ROBUST_NP the function fails and returns:


EOWNERDEAD

    The last owner of this mutex died while holding the mutex. This mutex is now owned by the caller. The caller must attempt to make the state protected by the mutex consistent.

    If the caller is able to make the state consistent, call pthread_mutex_consistent_np() for the mutex and unlock the mutex. Subsequent calls to pthread_mutex_lock() will behave normally.

    If the caller is unable to make the state consistent, do not call pthread_mutex_init() for the mutex, but unlock the mutex. Subsequent calls to pthread_mutex_trylock() fail to acquire the mutex and return an ENOTRECOVERABLE error code.

    If the owner that acquired the lock with EOWNERDEAD dies, the next owner acquires the lock with EOWNERDEAD.


ENOTRECOVERABLE

    The mutex you are trying to acquire is protecting state left irrecoverable by the mutex's previous owner that died while holding the lock. The mutex has not been acquired. This condition can occur when the lock was previously acquired with EOWNERDEAD and the owner was unable to cleanup the state and had unlocked the mutex without making the mutex state consistent.

ENOMEM

    The limit on the number of simultaneously held mutexes has been exceeded.

    */
    /*
      pthread_mutexattr_t Attr;

      pthread_mutexattr_init(&Attr);
      pthread_mutexattr_settype(&Attr, PTHREAD_MUTEX_RECURSIVE);
      pthread_mutex_init(mutex,&Attr);
      rc = pthread_mutex_trylock(mutex);*/
    //rc = pthread_mutex_lock(mutex);
    printf("mutext trylock rc=%d\n",rc);
    /*
    if (rc == EINVAL) { //invalid mutex, not initialized
      printf("rc == EINVAL\n");
      //create re-entry mutex
      //pthread_mutex_t Mutex;
      
      thread_func_args->thread_complete_success = false;
      pthread_exit(thread_func_args); 
      //return thread_param;
    }*/
    if (rc == 0 ) {
      //do nothing
    }
    //The mutex could not be acquired because it was already locked.
    /*
    if (rc == EBUSY) {
      printf("rc == EBUSY\n");
      //blocking , return true
      thread_func_args->thread_complete_success = true;
      //pthread_exit(thread_func_args); 
      return thread_func_args;  
      
    }*/
   
    if (rc < 0 ) {
      thread_func_args->thread_complete_success = false;
      //pthread_exit(thread_func_args); 
      return thread_func_args;
    }
    
    usleep(1000*wait_to_release_ms);
    printf("after wait_to_release_ms\n");
    //if > 0 normal
    rc = pthread_mutex_unlock(mutex);
    printf("mutext unlock rc=%d\n",rc);
    
    if (rc == 0) {
      printf("thread mutext unlock successful \n");
    }
    if (rc != 0 ) {
      thread_func_args->thread_complete_success = false;
      //pthread_exit(thread_func_args); 
      return thread_func_args;
    }
   
    thread_func_args->thread_complete_success = true;
    
    
    
    
    printf("thread success \n");
    //dun stop the thread
    //pthread_exit(thread_func_args); 
    return thread_func_args;
}


bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex,int wait_to_obtain_ms, int wait_to_release_ms)
{
    /**
     * TODO: allocate memory for thread_data, setup mutex and wait arguments, pass thread_data to created thread
     * using threadfunc() as entry point.
     *
     * return true if successful.
     *
     * See implementation details in threading.h file comment block
     */
     
   
    
    bool retStatus = false;
    struct thread_data* thread_func_args = malloc(sizeof(struct thread_data));
    thread_func_args->wait_to_obtain_ms=wait_to_obtain_ms;
    thread_func_args->wait_to_release_ms=wait_to_release_ms;
    thread_func_args->mutex = mutex;
    
   
    int rc = pthread_create(thread, NULL, threadfunc, thread_func_args);
    printf("thread created \n");
    printf("rc=%d\n",rc);
    if (rc != 0 ) {
      perror("pthread_create");
      
      //free(thread_func_args);
     
      return false;
    }
    if (rc == 0) {
      retStatus = true;
    }
    
    
    //thread created
    //free(thread_func_args);
    //not owner, dont destroy the mutex
    //pthread_mutex_destroy(mutex);
    //thread_func_args = NULL;
    return retStatus;
}


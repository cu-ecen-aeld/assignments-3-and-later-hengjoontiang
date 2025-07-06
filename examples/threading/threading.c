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
    
    printf("mutext trylock rc=%d\n",rc);
    
    if (rc == 0 ) {
      //do nothing
    }
    
   
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


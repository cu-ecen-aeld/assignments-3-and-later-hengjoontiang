#include <pthread.h>
#include <stdlib.h>
#include "ThreadSafeArrayList.h"
//function to initialize arryalist
void init_array_list(ThreadSafeArrayList* list,int initial_capacity) {
  list -> elements = (void**)malloc(sizeof(void*) * initial_capacity);
  list -> size = 0;
  list->capacity = initial_capacity;
  pthread_mutex_init(&list->mutex, NULL);
}

//function to add an element
void add_element(ThreadSafeArrayList* list, void* element){
  pthread_mutex_lock(&list->mutex);
  
  //do stuff
  list->elements[list->size]=element;
  list->size++;
  pthread_mutex_unlock(&list->mutex);
}

//function to destroy array list
void destroy_array_list(ThreadSafeArrayList* list) {
  free(list->elements);
  pthread_mutex_destroy(&list->mutex);
}

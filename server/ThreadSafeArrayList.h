
typedef struct {
        void **elements; // Array of void pointers to store elements
        size_t size;     // Current number of elements
        size_t capacity; // Maximum capacity of the array
        pthread_mutex_t mutex; // Mutex for thread-safe access
} ThreadSafeArrayList;
//function to initialize arryalist
void init_array_list(ThreadSafeArrayList* list,int initial_capacity) ;
//function to add an element
void add_element(ThreadSafeArrayList* list, void* element);
void destroy_array_list(ThreadSafeArrayList* list) ;

/**
 * @file aesd-circular-buffer.c
 * @brief Functions and data related to a circular buffer imlementation
 *
 * @author Dan Walkes
 * @date 2020-03-01
 * @copyright Copyright (c) 2020
 *
 */

#ifdef __KERNEL__
#include <linux/string.h>
#else
#include <string.h>
#endif

#include "aesd-circular-buffer.h"
#include <stdlib.h>
#include <stdio.h>

/**
 * @param buffer the buffer to search for corresponding offset.  Any necessary locking must be performed by caller.
 * @param char_offset the position to search for in the buffer list, describing the zero referenced
 *      character index if all buffer strings were concatenated end to end
 * @param entry_offset_byte_rtn is a pointer specifying a location to store the byte of the returned aesd_buffer_entry
 *      buffptr member corresponding to char_offset.  This value is only set when a matching char_offset is found
 *      in aesd_buffer.
 * @return the struct aesd_buffer_entry structure representing the position described by char_offset, or
 * NULL if this position is not available in the buffer (not enough data is written).
 */
struct aesd_buffer_entry *aesd_circular_buffer_find_entry_offset_for_fpos(struct aesd_circular_buffer *cir_buffer,
            size_t char_offset, size_t *entry_offset_byte_rtn )
{

    bool IsFound = false;
    /**
    * TODO: implement per description
    */
    size_t off_based_onLeft_boundary_index = 0;
    //int off_based_onRight_boundary_index = 0;
    //based on char_offset, search thorugh from out pointer to in pointer
    //AESD_CIRCULAR_BUFFER_FOREACH(entryptr,buffer,index) 
    //{
    
    //check out_offs 
    size_t index_to_search=0;
    /*
    for (index_to_search = 0 ; index_to_search < (cir_buffer->out_offs); index_to_search++) {
        off_based_onLeft_boundary_index += cir_buffer->entry[index_to_search].size;
      
    }
    */
    
    //printf("passed in cir_buffer->out_offs=%d\n",cir_buffer->out_offs);
    //printf("passed in cir_buffer->in_offs=%d\n",cir_buffer->in_offs);
    //printf("passed in cir_buffer->count=%d\n",cir_buffer->count);
    
    
    
    size_t start_index = cir_buffer->out_offs;
    //size_t end_index = cir_buffer->in_offs;
    //size_t numItems = 0;
    index_to_search = start_index;
    uint8_t numItemsProcessed = 0;
    //while (index_to_search != end_index) {
    //while (index_to_search < end_index + AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED) {
    while (true) {
        if (numItemsProcessed >= cir_buffer-> count) break;
    //while (index_to_search != end_index + AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED  ) {
        index_to_search = index_to_search % AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED;
        //printf("entry item=%s\n",cir_buffer->entry[index_to_search ].buffptr);
        //index_to_search++;
  
    //for (index_to_search = cir_buffer->out_offs; index_to_search < (cir_buffer->in_offs + AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED); index_to_search++) {
      //index_to_search = index_to_search % AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED;
      //if found, break out of search loop
      //printf("index_to_search=%lu\n",index_to_search);
      //printf("entry buffer size=%lu\n",cir_buffer->entry[index_to_search].size);
      
      //array_index=0, len=7,word_index=0-6
      //printf("array_index=%lu\n",index_to_search);
      //printf("len=%lu\n",cir_buffer->entry[index_to_search].size);
      //printf("entry string=%s\n",cir_buffer->entry[index_to_search].buffptr);
      //printf("word_index=%lu-%lu\n",off_based_onLeft_boundary_index,off_based_onLeft_boundary_index + cir_buffer->entry[index_to_search].size-1);
      if ( off_based_onLeft_boundary_index <= char_offset && char_offset <= (off_based_onLeft_boundary_index + cir_buffer->entry[index_to_search].size) - 1){
          //printf("In if \n");
          *entry_offset_byte_rtn = (char_offset - off_based_onLeft_boundary_index) ;
          //printf("entry_offset_byte_rtn=%lu\n",*entry_offset_byte_rtn);
          
          //printf("updated out_offs=%d\n",cir_buffer->out_offs);
          //printf("returning\n");
          //return &cir_buffer->entry[index_to_search];
          IsFound = true;
      }//if ..
      //update the boundary
      //++cir_buffer->out_offs;
      /*
      if ((cir_buffer->out_offs)==  (AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED)) 
      { 
		cir_buffer->out_offs= 0;
      }
      */
      //cir_buffer->full = (cir_buffer->out_offs == cir_buffer->in_offs);
      if (IsFound) break;
      
      off_based_onLeft_boundary_index += (cir_buffer->entry[index_to_search].size);
      ++index_to_search;
      //++numItems;
      ++numItemsProcessed;
      
      
    }
    if (IsFound)
      return &cir_buffer->entry[index_to_search];
    
    return NULL;
}

/**
* Adds entry @param add_entry to @param buffer in the location specified in buffer->in_offs.
* If the buffer was already full, overwrites the oldest entry and advances buffer->out_offs to the
* new start location.
* Any necessary locking must be handled by the caller
* Any memory referenced in @param add_entry must be allocated by and/or must have a lifetime managed by the caller.
*/
//by all rights out_off, in_off, in_off is always on the right
void aesd_circular_buffer_add_entry(struct aesd_circular_buffer *cir_buffer, const struct aesd_buffer_entry *add_entry)
{
    /**
    * TODO: implement per description
    */
    //check if overflow;loopback if it indeed overflow
    //uint8_t in_offs=0;//head
    //uint8_t out_offs=0;//tail
    
    //MAX=AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED
    //check isFull flag, wrap index
    cir_buffer->full = cir_buffer->count == AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED;//(cir_buffer->out_offs == cir_buffer->in_offs);
    if(cir_buffer->full)
    {
	  //if ((cir_buffer->in_offs) == (AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED )) 
          { 
			cir_buffer->in_offs = 0; //cir_buffer->in_offs % AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED;
			cir_buffer->out_offs = cir_buffer->in_offs + 1;
			cir_buffer->count = AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED; //do not change the count
	  }
    }
   

    
    /*
    if (++(cir_buffer->out_offs)==  (AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED)) 
    { 
		cir_buffer->out_offs= 0;
    }*/
    //printf("b4 setting up buffer->entry[buffer->in_offs]=add_entry\n");
    //printf("cir_buffer->in_offs = %d\n",(int)cir_buffer->in_offs);
    //printf("add_entry->size=%d\n",(int)(add_entry->size));
    //printf("add_entry->buffptr=%s\n",add_entry->buffptr);
    //int str_len = add_entry->size;
    //printf("buffer->in_offs=%d\n",cir_buffer->in_offs);
    cir_buffer->entry[cir_buffer->in_offs].size = add_entry->size;
    
    cir_buffer->entry[cir_buffer->in_offs].buffptr = add_entry->buffptr;
    
    //printf("done setting up buffer->entry[buffer->in_offs]=add_entry\n");
    
    
    
    
     //assert(add_entry);
     //update indecx to next free slot,note that we dont wrap around, as the next slot is not being utilized
    if(!cir_buffer->full) { //update when it is not full, else forego updating, just keep overwriting
      ++(cir_buffer->in_offs) ; //update in_offs to point to next empty slot
      ++cir_buffer->count;
    }
    
}
/**
* Initializes the circular buffer described by @param buffer to an empty struct
*/
void aesd_circular_buffer_init(struct aesd_circular_buffer *cir_buffer)
{
    memset(cir_buffer,0,sizeof(struct aesd_circular_buffer));
    /*
    //int buff_size = sizeof(struct aesd_circular_buffer);
    if (!cir_buffer)
      cir_buffer = (struct aesd_circular_buffer *)malloc(sizeof(struct aesd_circular_buffer));
    //printf("buff_size=%d\n",buff_size);
    //ir_buffer = malloc(buff_size);
    
    cir_buffer->in_offs = 0;
    cir_buffer->out_offs = 0;
    cir_buffer->full = false;
    
    //cir_buffer = &(struct aesd_circular_buffer){.in_offs=0,.out_offs=0,.full=false};
    printf("inside init cir_buffer->in_offs=%d\n",cir_buffer->in_offs);*/
}



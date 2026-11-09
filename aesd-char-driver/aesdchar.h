/*
 * aesdchar.h
 *
 *  Created on: Oct 23, 2019
 *      Author: Dan Walkes
 */

#ifndef AESD_CHAR_DRIVER_AESDCHAR_H_
#define AESD_CHAR_DRIVER_AESDCHAR_H_

#define AESD_DEBUG 1  //Remove comment on this line to enable debug

#undef PDEBUG             /* undef it, just in case */
#ifdef AESD_DEBUG
#  ifdef __KERNEL__
     /* This one if debugging is on, and kernel space */
#    define PDEBUG(fmt, args...) printk( KERN_DEBUG "aesdchar: " fmt, ## args)
#  else
     /* This one for user space */
#    define PDEBUG(fmt, args...) fprintf(stderr, fmt, ## args)
#  endif
#else
#  define PDEBUG(fmt, args...) /* not debugging: nothing */
#endif

#include "aesd-circular-buffer.h"
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
//#include <linux/spinlock.h>
struct aesd_dev
{
    /**
     * TODO: Add structure(s) and locks needed to complete assignment requirements
     */
        
    
    
        //struct list_head list;
	dev_t dev; //device id of a specific device instance !!
	
	struct cdev cdev;     /* Char device structure  */
	//struct aesd_circular_buffer *circular_buffer; //custom circular buffer -- no direct access, within the device driver, effectively at user space, we interact with user data only
	//size_t	esize;
	//spinlock_t my_lock;
	//struct mutex fifo_lock;         // Shields queue modifications
	
	/* Access Control Flags */
        //atomic_t reader_available;     // Tracking reader slots 
        //atomic_t writer_available;     // Tracking writer slots
    
	/* Blocking queues for empty/full thresholds */
        //wait_queue_head_t read_queue;
        //wait_queue_head_t write_queue;
        
        struct class *my_class;
        struct device *my_device;
};



#endif /* AESD_CHAR_DRIVER_AESDCHAR_H_ */

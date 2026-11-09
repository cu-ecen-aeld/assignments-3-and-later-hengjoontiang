/**
 * @file aesdchar.c
 * @brief Functions and data related to the AESD char driver implementation
 *
 * Based on the implementation of the "scull" device driver, found in
 * Linux Device Drivers example code.
 *
 * @author Dan Walkes
 * @date 2019-10-22
 * @copyright Copyright (c) 2019
 *
 */
//#include <linux/mutex.h>
#include <linux/rwsem.h>      // Required for rw_semaphore APIs
//#include <linux/spinlock.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/printk.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/kernel.h>
#include <linux/fs.h> // file_operations

#include <linux/slab.h>
#include <linux/string.h>
#include "aesdchar.h"
#include "aesd-circular-buffer.h"
//#include <linux/rwlock.h>
int aesd_major =   0; // use dynamic major
int aesd_minor =   0;

MODULE_AUTHOR("Heng Joon Tiang"); /** TODO: fill in your name **/
MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("A circular buffer char device driver");
MODULE_VERSION("1.0");

#define DEVICE_NAME "aesdchar"
#define CLASS_NAME "char_class"
//#define MY_MAJOR 240
//#define MY_MINOR 0
// Read-Write Lock
//rwlock_t rwlock;
//static rwlock_t my_rwlock;
//rwlock_t my_rwlock = __RW_LOCK_UNLOCKED(my_rwlock);
//unsigned long flags;
//struct mutex dynamic_lock;
// 1. Declare and initialize the read-write semaphore statically
static DECLARE_RWSEM(my_rwsem);

//static DEFINE_SPINLOCK(rwlock);

static dev_t dev_num;

//define a temporary buffer to store the write characters till we encounter \n, before we write to the circular buffer
#define TEMP_BUF_SIZE 1024
char temp_buf[TEMP_BUF_SIZE];
size_t temp_len;
static int read_offset;


struct aesd_dev aesd_device;
struct aesd_circular_buffer circular_buffer; //note that the circular buffer is maintained separate from the device driver, it is not embedded within the device driver
//the notion is that the device driver requires resources, and then invoke it
/*
set up the device driver link between user space
*/

static size_t dev_size = 0; // Tracks the current length of the string
loff_t aesd_llseek(struct file *file, loff_t offset, int whence) {
    loff_t new_pos = 0;
    down_read(&my_rwsem);
    dev_size = getTotalSizeFromFIFO(&circular_buffer);
    up_read(&my_rwsem);
    switch (whence) {
        case SEEK_SET: // Seek from the beginning of the string
            new_pos = offset;
            break;

        case SEEK_CUR: // Seek from the current pointer position
            new_pos = file->f_pos + offset;
            break;

        case SEEK_END: // Seek from the end of the string (its length)
            new_pos = dev_size + offset;
            break;

        default:
            return -EINVAL; // Invalid argument
    }

    // Prevent seeking before the start or past the actual string length boundary
    if (new_pos < 0 || new_pos > dev_size) {
        return -EINVAL;
    }

    file->f_pos = new_pos; // Update the kernel's file position pointer
    return new_pos;        // Return the new position to user space
}

int aesd_open(struct inode *inode, struct file *filp)
{
    PDEBUG("open");
    /**
     * TODO: handle open
     */
    //rwlock_init(&my_rwlock);
    struct aesd_dev *dev; //device information

    dev = container_of(inode->i_cdev, struct aesd_dev,cdev);
    filp->private_data = dev;//save context pointer
    printk("Driver : open device\n");


    return 0;          /* success */

}
/*
In Linux character device drivers, release handles closing an individual file descriptor from user space, while cleanup (the module exit function) unloads the entire driver from the kernel.
*/
int aesd_release(struct inode *inode, struct file *filp)
{
    PDEBUG("release");
    /**
     * TODO: handle release
     */
     //release the memory used by the custom fifo
     struct aesd_dev *dev = (struct aesd_dev *)filp->private_data;


    return 0;
}
/*
Follow many reader but only 1 writer strategy

READING

start reading:
1. check writer_count == 0, start reading
                   else  wait till writer_count==0
2.  reader_count++,writer_count=0
end reading:
1.  reader_count--,writer_count=0

WRITING
start writing:
1. check read_count, if reader_count > 0 , skip
                     else    check writer_count > 0 , skip
                                          else        write
2. writer_count++, reader_count=0

end writing:
1. writer_count--,reader_count=0;

*/
//from system back to user; ie read from char device driver, and place the data into a read buffer *buf
//struct file *filp, char __user *buf, size_t count, loff_t *ppos
ssize_t aesd_read(struct file *filp, char __user *buf, size_t len,loff_t *offset)
{

    printk("aesd_read started");
    printk("AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED=%d",AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED);
    printk("*offset=%lld",*offset);
    //important so that cat will return once we have finished reading all -- EOF ie return 0
    
    if (*offset > AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED){
      return 0;
    }
    /*
    if (len > AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED -  *offset) {
      len = AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED -  *offset;
    }*/
    ssize_t retval = 0;
    //printk("read %zu bytes with offset %lld",len,*offset);
    //struct aesd_dev *dev = (struct aesd_dev *)filp->private_data;
    //size_t available_data;
    size_t bytes_to_read;
    size_t bytes_to_write;
    
   
    // Request shared access to read
    //read_lock_irqsave(&xxx_lock, flags);
    //if (read_lock_irqsave(&my_rwlock,flags))
    //    return -ERESTARTSYS;
    // Acquire the lock for reading (multiple readers can enter concurrently)
    printk("Just before down_read");
    down_read(&my_rwsem);
    printk("Just after down_read");
    //kernel space
    /*
    static void verify_find_entry(struct aesd_circular_buffer *buffer, size_t entry_offset_byte, const char *expectstring)
{
    size_t offset_rtn=0;
    char message[250];
    struct aesd_buffer_entry *rtnentry = aesd_circular_buffer_find_entry_offset_for_fpos(buffer,
                                                entry_offset_byte,
                                                &offset_rtn);
    
    */
    size_t offset_rtn=0;
    //read_offset = circular_buffer.in_offs;
    
    bytes_to_read = 0;
    bytes_to_write = 0;
    //keep reading till we have exhausted all of the data in the circular buffer
    int i=0;
    int sizeof_string;
    //int buffer_offset = 0;
    printk("Just before for loop");
    //char* buf_head_current_str = buf;
    
    for (i=0;i<circular_buffer.count;i++) {
      printk("i in for loop=%d",i);
      struct aesd_buffer_entry * system_data_ptr = aesd_circular_buffer_find_entry_offset_for_fpos(&circular_buffer,
            bytes_to_read, &offset_rtn);
      if (system_data_ptr == NULL) {
        sizeof_string = 0;
        printk("system_data_ptr == NULL");
        //buf[bytes_to_read++]='\0';
        goto out;
      }
      if (system_data_ptr->buffptr == NULL) {
        sizeof_string = 0;
        //buf[bytes_to_read++]='\0';
        printk("system_data_ptr->buffptr == NULL");
        goto out;
      }
      printk("Just after aesd_circular_buffer_find_entry_offset_for_fpos bytes_to_read=%zu,offset_rtn=%zu",bytes_to_read,offset_rtn);
      printk("system_data_ptr->buffptr=%s",system_data_ptr->buffptr);
      
      //unsigned long copy_to_user(void __user *to, const void *from, unsigned long n);
      //AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED = 10
      
      //char * intermed_buffer = kmalloc(strlen(system_data_ptr->buffptr),GFP_KERNEL);
      //sizeof_string = strlen(system_data_ptr->buffptr);
      sizeof_string = system_data_ptr->size;
      //sizeof_string--; //take out \0
      printk("sizeof_string system_data_ptr->buffptr=%d",sizeof_string);
      //if (copy_to_user(buf + bytes_to_read, system_data_ptr->buffptr, sizeof_string) ) { //copy from kernel space into user space
      
      //sizeof_string +1 , include \0 behind for proper string termination
      //+1 for \0
      //less \0 behind
      if (copy_to_user(buf+ bytes_to_write, system_data_ptr->buffptr, sizeof_string-1) ) { //copy from kernel space into user space
      //if (copy_to_user(intermed_buffer, system_data_ptr->buffptr,strlen(system_data_ptr->buffptr)) ) { //copy from kernel space into user space
            //read_unlock_irqrestore(&my_rwlock);
            up_read(&my_rwsem);
            return -EFAULT;
      }
      //read_offset = (read_offset+1) % AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED;
      //buffer_offset += sizeof(system_data_ptr->buffptr); 
      //buffer_offset += bytes_to_read;
      
      bytes_to_read += (sizeof_string); //extra 1 char for \n
      //bytes_to_read++; // \0
      
      //bytes_to_read++;
      bytes_to_write += (sizeof_string-1); //no need for ++, as \n already  present in the string
      //bytes_to_write++;
      printk("sizeof_string system_data_ptr->buffptr=%d",sizeof_string);
      //printk("system_data_ptr->buffptr=%s",system_data_ptr->buffptr);
      //printk("sizeof(system_data_ptr->buffptr)=%d",sizeof_string);
      //printk("intermed_buffer=%s",intermed_buffer);
      //buf[bytes_to_read+sizeof_string]='\n';
      //sizeof_string++; //for \n
      
      //buf[bytes_to_read]='\r';//add +1 for extra \nes_
      //bytes_to_write++;
      //sizeof_string++; //for \n
      //buf[bytes_to_write]='\n';//add +1 for extra \nes_
      //bytes_to_write++;
      
      
      
      //buf_head_current_str += sizeof_string;
      
      printk("i value=%d",i);
      
      //*offset=*offset+1;
       // Advance the offset so the next iteration of cat knows where to continue
      *offset += bytes_to_write;
    }
    //read_offset += offset_rtn;
    //buf = 0; //reset back to beginning
    //if (i==AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED){
       
    
        //bytes_to_read++;
        //goto out;
    //}

    // Release the shared lock
    //read_unlock_irqrestore(&my_rwlock,flags);
    // Release the read lock
     //buf[bytes_to_write]='\n';
    //bytes_to_write++;
    //bytes_to_write++;
    //*offset = *offset  + 1; //add \0
    buf[bytes_to_write]='\0';
    
out:
   
    
    
    //*offset = 0; //reset
    up_read(&my_rwsem);
    
    printk("buf= %s",buf);
    
    printk("aesd_read ended");
    //buf[bytes_to_read+sizeof_string]='\n';
    
    sanity_check(&circular_buffer);
    printk("bytes_to_write=%zu",bytes_to_write);
    return bytes_to_write;
}


//from user into system; ie read from write buffer *buf, and write into char device driver
ssize_t aesd_write(struct file *filp, const char __user *buf, size_t lenk,
                loff_t *f_pos)
{
    ssize_t retval = -ENOMEM;
    printk("1 - write %zu bytes with offset %lld",lenk,*f_pos);
    /**
     * TODO: handle write
     */

    
    struct aesd_dev *dev = (struct aesd_dev *)filp->private_data;
    
   
    down_write(&my_rwsem);

   
    //PDEBUG("string k_buf passed in %s\n",k_buf);
    //from assigment 7 part 1 -- Test_circular_buffer.c
    printk("lenk at begin = %zu\n",lenk);
    size_t bytes_written = 0;
    
   
    char c;
    int i;
    for (i = 0; i < lenk; i++) {
        // Get one byte from user space
        if (get_user(c, buf + i)) {
            bytes_written = -EFAULT;
            printk("bytes_written = -EFAULT");
            goto out;
        }

        // Prevent temporary buffer overflow
        if (temp_len >= TEMP_BUF_SIZE) {
            // Handle error: clear buffer or force flush
            temp_len = 0; 
            goto out;
        }

        // Accumulate in the temporary buffer
        //temp_len++;
        temp_buf[temp_len] = c;
        

        
        
        
        printk("current character %c",c);
        if (c=='\n') {
          printk("current character is nextline c= \\n");
          
        }
        printk("temp_len %zu",temp_len);
        
        temp_len++;
        bytes_written++;
        *f_pos = *f_pos + 1;
        
        if (c=='\n' ){
        
    
          
          printk("Just about to assign \\0 to temp_buf[temp_len]");
          temp_buf[temp_len] = '\0';//add null
          printk("Just after assign \\0 to temp_buf[temp_len]");
          
          
          //temp_len++;
        //bytes_written++;
        //*f_pos = *f_pos + 1;
        
        //add_entry->buffptr = (const char*)&temp_buf;
        struct aesd_buffer_entry* add_entry = kmalloc(sizeof(struct aesd_buffer_entry), GFP_KERNEL);
        printk("Just after add_entry kmalloc\n");
        add_entry->buffptr =kmalloc(temp_len+1, GFP_KERNEL); 
        //add_entry->buffptr =kmalloc(temp_len, GFP_KERNEL); 
        printk("Just after add_entry->buffptr kmalloc\n");
        printk("Just before buyes_copied=strscpy \n");
        int bytes_copied = strscpy((char *) add_entry->buffptr, temp_buf, temp_len+1);//count wil need to include \0, length copied over will be count-1
        //int bytes_copied = strscpy((char *) add_entry->buffptr, temp_buf, temp_len);//count wil need to include \0, length copied over will be count-1
        printk("Just after buyes_copied=strscpy \n");
        //do a copy from temp_buf into add_entry->buffptr
        printk("bytes_copied %d",bytes_copied);
        printk("2 - write %zu bytes with offset %lld",temp_len+1,*f_pos);
        //printk("2 - write %zu bytes with offset %lld",temp_len,*f_pos);
            
        add_entry->size= temp_len+1;
        //add_entry->size= temp_len;
            
         printk("writing to circular buffer %s",temp_buf);
            
            
        aesd_circular_buffer_add_entry(&circular_buffer, add_entry);
        printk("aesd_circular_buffer_add_entry(&circular_buffer, add_entry)");
        kfree(add_entry);
        printk("kfree(add_entry)");
        
        printk("temp_buf[temp_len] set to 0 \n");
        printk("temp_len %zu",temp_len);
        temp_len = 0;
        read_offset = 0; //reset
      //reset the buffer
      // Reset temporary buffer length for the next string
      
        //*f_pos = 0;//dun reset fpos
        goto out;
      }//if (temp_buf[temp_len]=='\n' ){
      
      
     
    }//for (i = 0; i < len; i++) {
    printk("len after loop=%zu\n",lenk);
    
      
      
      
        
      
      
    
            
          

    // Release the exclusive lock
    //write_unlock_irqrestore(&my_rwlock,flags);
    // Release the write lock
  out:
    up_write(&my_rwsem);
    
    sanity_check(&circular_buffer);
    printk("bytes_written=%zu",bytes_written);
    printk("lenk=%zu\n",lenk);
    printk("*f_pos=%lld\n",*f_pos);
    
    
    //dev_size = bytes_written;
    return lenk;

    //return retval;
    //return retval;
}
struct file_operations aesd_fops = {
    .owner =    THIS_MODULE,
    .llseek  = aesd_llseek,
    .open =     aesd_open,
    .read =     aesd_read,
    .write =    aesd_write,
    .release =  aesd_release,
};



static int aesd_setup_cdev(struct aesd_dev *dev)
{

    //dev_t dev_num;
    unsigned int count = 1; // Number of minor numbers to request

    int result;

  // Request dynamic major number and a range of minor numbers starting at 0
  result = alloc_chrdev_region(&dev_num, 0, count, DEVICE_NAME);
  

  // Extract major and minor numbers
  aesd_major= MAJOR(dev_num);
  aesd_minor = MINOR(dev_num);
    if (result < 0) {
        printk(KERN_WARNING "Can't get major %d\n", aesd_major);
        return result;
    }
    //int err, devno = MKDEV(aesd_major, aesd_minor);

    cdev_init(&dev->cdev, &aesd_fops);
    //dev->cdev.owner = THIS_MODULE;
    //dev->cdev.ops = &aesd_fops;
    int err = cdev_add(&dev->cdev, dev_num, 1);
    if (err) {
        printk(KERN_ERR "Error %d adding aesd cdev", err);
        //unregister_chrdev_region(devno, 1);
        goto r_class;
    }
    
    dev->my_class = class_create(THIS_MODULE,CLASS_NAME);
    if (IS_ERR(dev->my_class)){
      printk("Failed to create class\n");
      goto r_cdev;
    }

    dev->my_device = device_create(dev->my_class,NULL,dev_num,NULL,DEVICE_NAME);
    if (IS_ERR(dev->my_device)){
      printk("Failed to create device\n");
      goto r_device;
    }
    
    return 0;

    
    r_device:
      class_destroy(dev->my_class);
    r_cdev:
      cdev_del(&dev->cdev);
      
    r_class:
      unregister_chrdev_region(dev_num,1);
      return -1;
}


/*
set up the device driver
*/
int aesd_init_module(void)
{
    int result;
    memset(&aesd_device,0,sizeof(struct aesd_dev));
    //mutex_init(&dynamic_lock);
    /**
     * TODO: initialize the AESD specific portion of the device
     */
    

    //ased_device.circular_buffer
    result = aesd_setup_cdev(&aesd_device);
    //init_waitqueue_head(&aesd_device.read_queue);
    //init_waitqueue_head(&aesd_device.write_queue);

    aesd_circular_buffer_init(&circular_buffer);
    printk("aesd init");

    return result;

}
/*
In Linux character device drivers, release handles closing an individual file descriptor from user space, while cleanup (the module exit function) unloads the entire driver from the kernel.
*/
void aesd_cleanup_module(void)
{
    //dev_t devno = MKDEV(aesd_major, aesd_minor);

    
    /**
     * TODO: cleanup AESD specific poritions here as necessary
     */

    //unregister_chrdev_region(devno, 1);
    int i;
    for (i=0;i<AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED;i++) {
      struct  aesd_buffer_entry aesd_entry = circular_buffer.entry[i];
      if (aesd_entry.buffptr) kfree(aesd_entry.buffptr);
    }
    
    circular_buffer.in_offs= 0;
    circular_buffer.out_offs= 0;
    circular_buffer.full = false;
    circular_buffer.count = 0;
    
    
    
    device_destroy(aesd_device.my_class,dev_num);
    
    class_destroy(aesd_device.my_class);
    
    cdev_del(&aesd_device.cdev);
    unregister_chrdev_region(dev_num,1);
    
    
    
    printk("aesd clean");
}

module_init(aesd_init_module);
module_exit(aesd_cleanup_module);

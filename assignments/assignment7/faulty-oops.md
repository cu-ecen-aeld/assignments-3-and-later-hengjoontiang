Analysis of error when we echo hello to /dev/faulty

# echo "hello world" > /dev/faulty
snapshot of the error
>>
[   38.479672] Unable to handle kernel NULL pointer dereference at virtual address 0000000000000000
..
>>

The key error highlighted is 
38.479672] Unable to handle kernel NULL pointer dereference at virtual address 0000000000000000

It came from this piece of code found in faulty.c
>>
ssize_t faulty_write (struct file *filp, const char __user *buf, size_t count,
		loff_t *pos)
{
	/* make a simple fault by dereferencing a NULL pointer */
	*(int *)0 = 0;
	return 0;
}
>>



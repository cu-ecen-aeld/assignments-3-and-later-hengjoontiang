/*
Write a shell script finder-app/writer.sh as described below

    Accepts the following arguments: the first argument is a full path to a file (including filename) on the filesystem, referred to below as writefile; the second argument is a text string which will be written within this file, referred to below as writestr

    Exits with value 1 error and print statements if any of the arguments above were not specified

    Creates a new file with name and path writefile with content writestr, overwriting any existing file and creating the path if it doesn’t exist. Exits with value 1 and error print statement if the file could not be created.


*Take note destination always created, so there is no need to create a new folder
Example:

       writer.sh /tmp/aesd/assignment1/sample.txt ios

Creates file:

    /tmp/aesd/assignment1/sample.txt

            With content:

            ios
  */

#include <syslog.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h> 
int main(int argc, char* argv[]) {
  //check the number of arguments, MUST BE 2, highlight error that there must be 2 input parameters
  //after checking that number of input parameters = 2,
  //              check if the destination file exists, create if not exists
  //                    write to file stated

  openlog("Syslog",0,LOG_USER);


  //check input parameters
  printf("Number of arguments=%d\n",argc);
  if (argc != 3){
    printf("Incorrect number of arguments passed\n");
    printf("The first argument should be file name\n");
    printf("The second argument should be the text to write to the file in argument 1\n");
    return 1;
  }

  FILE *fptr;
  fptr = fopen(argv[1],"w");
  //char *log_msg_str;
  //log_msg_str = malloc(sizeof(argv[2])+100);
  //strcpy(log_msg_str,argv[2]);
  //sprintf(log_msg_str,"File opened=%s\n",argv[2]);
  if (errno > 0){
    syslog(LOG_ERR,"%s\n",strerror(errno));
    return 1;
  }
  //syslog(LOG_DEBUG,"File opened=%s\n",argv[1]);
  //free(log_msg_str);

  //char data[50] = "GeeksforGeeks-A Computer ";
  fputs(argv[2],fptr);
  syslog(LOG_DEBUG,"Writing %s to %s\n",argv[2],argv[1]);
  //fputs(data,fptr);
  //simple test
  //tested ok -- checked /var/log/syslog
  //syslog(LOG_DEBUG,"writer debug msg");
  //syslog(LOG_ERR,"writer err msg");

  fclose(fptr);
  closelog();

  return 0;
}

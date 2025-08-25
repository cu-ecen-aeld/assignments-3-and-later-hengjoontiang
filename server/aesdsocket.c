/*
Opens a stream socket bound to port 9000, failing and returning -1 if any of the socket connection steps fail.

     c. Listens for and accepts a connection

     d. Logs message to the syslog “Accepted connection from xxx” where XXXX is the IP address of the connected client. 

     e. Receives data over the connection and appends to file /var/tmp/aesdsocketdata, creating this file if it doesn’t exist.

    Your implementation should use a newline to separate data packets received.  In other words a packet is considered complete when a newline character is found in the input receive stream, and each newline should result in an append to the /var/tmp/aesdsocketdata file.

    You may assume the data stream does not include null characters (therefore can be processed using string handling functions).

    You may assume the length of the packet will be shorter than the available heap size.  In other words, as long as you handle malloc() associated failures with error messages you may discard associated over-length packets.

     f. Returns the full content of /var/tmp/aesdsocketdata to the client as soon as the received data packet completes.

    You may assume the total size of all packets sent (and therefore size of /var/tmp/aesdsocketdata) will be less than the size of the root filesystem, however you may not assume this total size of all packets sent will be less than the size of the available RAM for the process heap.

     g. Logs message to the syslog “Closed connection from XXX” where XXX is the IP address of the connected client.

     h. Restarts accepting connections from new clients forever in a loop until SIGINT or SIGTERM is received (see below).

     i. Gracefully exits when SIGINT or SIGTERM is received, completing any open connection operations, closing any open sockets, and deleting the file /var/tmp/aesdsocketdata.

    Logs message to the syslog “Caught signal, exiting” when SIGINT or SIGTERM is received.
*/
/*
** server.c -- a stream socket server demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <signal.h>
#include <syslog.h>
#include <fcntl.h>
#include <time.h>
#include <pthread.h>
#include "ThreadSafeArrayList.h"
#include "threading.h"

#define PORT "9000"  // the port users will be connecting to
#define AESD_DATA_FILE "/var/tmp/aesdsocketdata"
#define BACKLOG 10   // how many pending connections queue will hold
volatile sig_atomic_t stop_flag = 0;
pthread_mutex_t aesd_file_mutex;

volatile sig_atomic_t IsMsgComplete = 0;
ThreadSafeArrayList threadsafearraylist;
// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void init_file(char * filename){
          FILE *fptr;//for client data store
          fptr = fopen(filename,"w");
          /*if (truncate(filename, 0) == -1) { // Truncates to 0 bytes
            perror("Error truncating file with truncate()");
            return ;
          }*/

          //fprintf(fptr, "%s", stream_buffer);
          fclose(fptr);
          fptr = NULL; 
}
void write_file(char * stream_buffer, int numchar_to_write, char * filename){

          //check mutex
          pthread_mutex_lock(&aesd_file_mutex);
          
          FILE *fptr;//for client data store
          fptr = fopen(filename,"a+");
          stream_buffer[numchar_to_write] = '\0';
          fprintf(fptr, "%s", stream_buffer);
          
          //printf("write_file raw_bytes\n");
          //for (int u=0;u<strlen(stream_buffer);u++){
          //  printf("%d\n",stream_buffer[u]);
          //}
          //fputs(stream_buffer,fptr);
          //fflush(fptr);
          fclose(fptr);
          fptr = NULL; 
          
          //let go of mutex
          pthread_mutex_unlock(&aesd_file_mutex);
}
char * read_file(ssize_t* numbytes, char * filename) {
            char* stream_buffer;
            FILE *fptr;//for client data store
            //ssize_t numbytes;
            
            
            pthread_mutex_lock(&aesd_file_mutex);
            
            fptr = fopen(filename,"r");
            if(fptr == NULL){
                perror("Missing file to send back to client");
                exit(1);
            }
            /* Get the number of bytes */
            fseek(fptr, 0L, SEEK_END);
            *numbytes = ftell(fptr);
            /* reset the file position indicator to 
            the beginning of the file */
            //fseek(fptr, 0L, SEEK_SET);	
            /* grab sufficient memory for the 
            buffer to hold the text */
            rewind(fptr);
            //printf("size of file =%d\n",(int)(*numbytes));
            stream_buffer = (char*)malloc((*numbytes)*sizeof(char));
            //buffer = (char*) malloc(sizeof(char) * (string_size + 1) );
            /* memory error */
            if(stream_buffer == NULL){
                perror("Error in allocating memory for stream buffer");
                exit(1);
            }
            /* copy all the text into the buffer */
            int read_size = fread(stream_buffer, sizeof(char), *numbytes, fptr);
            //read_size = fread(buffer, sizeof(char), string_size, handler);
            //stream_buffer[*numbytes]='\0';
            //printf("readfile stream_buffer=%s\n",stream_buffer);
            if (*numbytes != read_size)
            {
             // Something went wrong, throw away the memory and set
             // the buffer to NULL
             free(stream_buffer);
             stream_buffer = NULL;
            }
            fclose(fptr);
            //fptr = NULL; 
            
            pthread_mutex_unlock(&aesd_file_mutex);
            
            return stream_buffer;
}
//to handle SIGINT and SIGTERM
void sigint_handler(int signum) {
        if (signum == SIGINT || signum == SIGTERM) {
            printf("SIGINT or SIGTERM fired \n");
            syslog(LOG_INFO,"Caught signal, exiting");
            stop_flag = 1;
            //printf("stop_flag=%d\n",stop_flag);
        }
}
void timer_handler (int signum){
  printf("Timer expired!\n");
  /* Append a timestamp in the form “timestamp:time” where time is specified by the RFC 2822 compliant strftime format
, followed by newline.*/ 
  //"Tue, 01 Jun 2024 14:30:00 -0700"
  
  time_t t = time(NULL);
  struct tm tm = *localtime(&t);
  // 2025-08-17 19:26:01
  //01234567890123456789
  char buffer[50];
  sprintf(buffer,"timestamp: %d-%02d-%02d %02d:%02d:%02d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
  printf("%s\n",buffer);
  
  write_file(buffer, sizeof(buffer),AESD_DATA_FILE);
}
//read file contents into a buffer
void* threadfunc(void* thread_param)
{

    // TODO: wait, obtain mutex, wait, release mutex as described by thread_data structure
    // hint: use a cast like the one below to obtain thread arguments from your parameter
    struct thread_data* thread_func_args = NULL;
    
    
   
    int clientfd;
    
    if (thread_param){
      thread_func_args = (struct thread_data *) thread_param;
      thread_func_args->currentThread = pthread_self(); 
      clientfd = thread_func_args->clientfd;
    }
    printf("In threadfunc\n");
    
    //do thread stuff
    //do read from socket
    
    //check if rec \n
    //need to pass in clientfd
    char buffer[10000]; //use a fixed size buffer for receiving
    ssize_t bytes_received = recv(clientfd,buffer,1024,0);//fixed max size 1024 so that it does not hang
    
    
    //add_element(&threadsafearraylist,buffer);
    
    if (buffer[bytes_received-1]=='\n')
        IsMsgComplete = 1;
    if (bytes_received <= 0 ) 
        IsMsgComplete = 1;
    if (bytes_received > 0 ) {
    //transfer to linkedlist
      printf("buffer=%s\n",buffer);
      char *bytes_rec_buffer=malloc(bytes_received);
      strcpy(bytes_rec_buffer,buffer);
      add_element(&threadsafearraylist, bytes_rec_buffer);
    }
   
    
    thread_func_args->thread_complete_success = true;
    
    
    
    
    printf("thread success \n");
    //dun stop the thread
    //pthread_exit(thread_func_args); 
    return thread_func_args;
}


int aesd_main(void)
{

    //signal(SIGINT, signal_handler); // Register the signal handler
    //#CTRL-C in aesd
    if (signal(SIGINT, sigint_handler) == SIG_ERR) {
        perror("Error setting SIGINT handler");
        exit(1);
    }
    //signal(SIGTERM, signal_handler); // Register the signal handler
    //ps -eaf 
    //#look for aesd
    //kill <pid_aesd>
    if (signal(SIGTERM, sigint_handler) == SIG_ERR) {
        perror("Error setting SIGTERM handler");
        exit(1);
    }
    if (signal(SIGALRM, timer_handler) == SIG_ERR) { // Install signal handler
        perror("Error setting SIGALRM handler");
        exit(1);
    }
    //alarm(10); // Schedule SIGALRM in 5 seconds
    
    
    struct itimerval new_value;
    memset(&new_value, 0, sizeof(new_value));

    new_value.it_interval.tv_sec = 10;   
    new_value.it_interval.tv_usec = 0;
    new_value.it_value.tv_sec = 10;
    new_value.it_value.tv_usec = 0;

    int ret = setitimer(ITIMER_REAL, &new_value, NULL);
    if (ret != 0)
    {
         perror("setitimer");
         exit(1);
    }
    
    
    init_array_list(&threadsafearraylist, 10000);
    
    pthread_mutex_init(&aesd_file_mutex, NULL); // Initialize the mutex
    
    FILE *fptr;//for client data store
    
    //init_file(AESD_DATA_FILE);
    fptr = fopen(AESD_DATA_FILE,"w");
    //fprintf(fp, "Hello, world!\n");
    fclose(fptr); // Close the file
    
    openlog("Syslog",0,LOG_USER);
    
    // listen on sock_fd, new connection on new_fd
    int sockfd, clientfd;
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address info
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    //printf("after getaddrinfo\n");
    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }
       /*
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }*/
        int enable = 1;
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR , &yes, sizeof(int)  ) ) {
            perror("setsockopt");
            exit(1);
        }
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo); // all done with this structure

    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        exit(-1);
    }

   if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(-1);
    }
    //printf("after listen\n");
    
    //printf("server: waiting for connections...\n");

    
    while (!stop_flag)  {  // main accept() loop
    
        int flags = fcntl(sockfd, F_GETFL, 0); // Get current flags
        if (flags == -1) {
          perror("fcntl F_GETFL");
          // Handle error
          exit(-1);
        }

        if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) == -1) { // Set O_NONBLOCK flag
            perror("fcntl F_SETFL O_NONBLOCK");
            // Handle error
            exit(-1);
        }
        
        sin_size = sizeof their_addr;
        clientfd = accept(sockfd, (struct sockaddr *)&their_addr,&sin_size);
        if (clientfd == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // No pending connections, continue or perform other tasks
                // You might introduce a small delay (e.g., sleep) to avoid busy-waiting
                continue;//in the while loop
            } 
        } 
        //printf("after accept\n");
        inet_ntop(their_addr.ss_family,
            get_in_addr((struct sockaddr *)&their_addr),
            s, sizeof s);
        //printf("after inet_ntop\n");
        
        
       
        /*
        Logs message to the syslog “Accepted connection from xxx” where XXXX is the IP address of the connected client
        */
        //printf("server: got connection from %s\n", s);
        syslog(LOG_INFO,"Accepted connection from %s\n",s);
        /*
        https://www.reddit.com/r/learnpython/comments/avl5f5/using_new_line_in_sockets/
        https://www.how2lab.com/programming/c/socket-programming-7#google_vignette
        */
        //printf("fork()\n");
        if (!fork()) { 
           //printf("child process\n");
        //sub process invoked by server to handle comms with client; THOIS IS NOT the CLIENT
        //note that this client handler need to handle sigint / sigterm
        
        // this is the child process
            //sockfd -- server
            //new_fd -- client
            //close(sockfd); // child doesn't need the listener
            char buffer[100000]; // buffer to store text data rec from client
            char *stream_buffer;
            ssize_t numbytes;
            //rec non blocking -- aloows sigint/sigterm to interrupt while loop -- server get from client
            
            
            int flags = fcntl(clientfd,F_GETFL,0);
            fcntl(clientfd,F_SETFL,flags | O_NONBLOCK);
            
            
            //int receiver_len=-1;
            int total_received = 0;
            
            
            IsMsgComplete = 0;
            pthread_t thread_array[100];
            int pthread_count = 0;//reset threadcount
            while ( !stop_flag) { //this while loop will continue to create new threads to rec till we have recieved end of line,which will break out of loop
              //printf("rec loop stop_flag=%d\n",stop_flag);
              //if (stop_flag) break; //to handle SIGINT/SIGTERM
              //non blocking read till client has stopped sending
              fd_set read_fds;
              FD_ZERO(&read_fds);
              FD_SET(clientfd,&read_fds);
              
              struct timeval timeout;
              timeout.tv_sec = 0; //check every 1 sec
              timeout.tv_usec = 100;//this will control how much delay
              
              
              
              
              int retval = select(clientfd + 1, &read_fds,NULL,NULL,&timeout);
              int received_len=0;
              if (retval == -1) {
                if (errno = EINTR) {
                  //interrupted by signal
                  continue;
                }else {
                  perror("select");
                  break;
                }
              }else if ( retval == 0) {
                //timeout , no data,continue
                break;
              }else {
                //data available, recv data
                /*
                if (receiver_len == -1){
                  recv(clientfd, &received_len, sizeof(int),0);
                  printf("received_len=%d\n",received_len);
                }*/
                
                //int received_len = -1;
                //recv(clientfd,&received_len, sizeof(int),0);
                //printf("received_len=%d\n",received_len);
                
                //ssize_t bytes_received = recv(clientfd,buffer+total_received,1024,0);//fixed max size 1024 so that it does not hang
                
                
                //bool retStatus = false;
                struct thread_data* thread_func_args = malloc(sizeof(struct thread_data));
                thread_func_args->clientfd = clientfd;
                int rc = pthread_create(&thread_array[pthread_count], NULL, threadfunc, thread_func_args);
                pthread_count++; 
                
              }
              /*
              if (total_received == received_len) {
                //buffer[total_received]='\0';//snull terminate for string
                //write_file(buffer, AESD_DATA_FILE);
                break;
              }*/
              
               if (IsMsgComplete==1) 
                  break;
              
            }//while ( !stop_flag) {
            
            //join threads 
            void * thread_rtn = NULL;
            for (int local_thread_count = 0; local_thread_count < pthread_count;local_thread_count++) {
              int rc = pthread_join(thread_array[local_thread_count],&thread_rtn);
              if( thread_rtn ) {
                struct thread_data* thread_func_data = (struct thread_data *) thread_rtn;
                free(thread_rtn);
              }//if( thread_rtn ) {
            }//for (int local_thread_count
            
            
            //reconstruct the buffer from the threadsafearraylist
            int list_size = threadsafearraylist.size;
            for (int array_index = 0 ; array_index < list_size; array_index++) {
              //copy from threadsafearraylist to the buffers");
              
              //int mini_buffer_size=sizeof(threadsafearraylist.elements[array_index])/sizeof(threadsafearraylist.elements[array_index][0]);
              int mini_buffer_size=strlen(threadsafearraylist.elements[array_index]);
              /*
              printf("mini_buffer_size=%d\n",mini_buffer_size);
              char *mini_buffer=malloc(mini_buffer_size);
              strcpy(mini_buffer,(char*)threadsafearraylist.elements[array_index]);
              printf("min_buffer=%s\n",mini_buffer);
              printf("threadsafearraylist.elements[%d]=%s\n",array_index,mini_buffer);
              strcat(buffer,mini_buffer);
              */
              strcat(buffer,(char*)threadsafearraylist.elements[array_index]);
              total_received+=mini_buffer_size;
            }
            
            printf("check buffer=%s\n",buffer);
            printf("total_received=%d\n",total_received);
            
            //send(clientfd, "abcd", 4, 0);
            //buffer[total_received]='\0';//snull terminate for string
            //printf("adjusted length of string with null terminator =%d\n",total_received);
            //printf("buffer=%s",buffer);
            write_file(buffer, total_received,AESD_DATA_FILE);
            //fptr = fopen(AESD_DATA_FILE,"a");//truncate any existing and append
            /*
            fptr = fopen(AESD_DATA_FILE,"a+");//truncate any existing and append
            fprintf(fptr, "%s", buffer);
            fclose(fptr);
            */
            //fclose(fptr);
            
            
            //read file copntents into a buffer
            ssize_t numbytes_file;
            stream_buffer = read_file(&numbytes_file,AESD_DATA_FILE) ;
            //printf("numbytes_file=%d\n",(int)numbytes_file);
            //printf("stream_buffer=%s",stream_buffer);
            //send non blocking -- aloows sigint/sigterm to interrupt while loop -- server send to client
            ssize_t total_sent = 0;
            ssize_t bytes_to_send = numbytes_file; // Total bytes to send
            
           // send(clientfd, "abcd", 4, 0);
            while (!stop_flag) {
              //printf("send loop stop_flag=%d\n",stop_flag);
              //if (stop_flag) break; //to handle SIGINT/SIGTERM
              //non blocking send to client till the whole file has been sent
              
              
              ssize_t sent_this_call = send(clientfd, stream_buffer + total_sent, bytes_to_send - total_sent, 0);
              //printf("sent_this_call=%d\n",(int)sent_this_call);
              if (sent_this_call == -1) {
                  if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    // Send buffer full, wait and try again (e.g., using select/poll)
                    // For simplicity, this example omits the waiting mechanism.
                    // In a real application, you'd use select/poll to wait for writeability.
                    continue; // Try sending again in the next iteration
                  } else {
                    // Handle other errors
                    break;
                  }
              } else if (sent_this_call == 0) {
                  // Connection closed by peer
                  //free(buffer);
                  //buffer = NULL;
                  break;
              } else {
                  total_sent += sent_this_call;
              }
              if (total_sent == bytes_to_send) {
                //printf("total_sent == bytes_to_send");
                //free(buffer);
                //buffer = NULL;
                break; //finished sending, break out of loop
              }
            }
            close(clientfd);
            syslog(LOG_INFO,"Closed connection from %s\n",s);
            //close(fptr);
            exit(0);
        }//if (!fork()) { 
        //printf("main process\n");
        //close(clientfd);  // parent doesn't need this
    }//while (!stop_flag) 

    if (fptr != NULL) {
        //cleann  up the file for next run
        //printf("cleaning up fptr\n");
        //fclose(fptr);
        //fptr = NULL; // Optional: set to NULL after closing to avoid dangling pointer
        
        //delete the file
        // /var/tmp/aesdsocketdata
    }
    //try to close any still opened socket
    remove(AESD_DATA_FILE);
    close(clientfd); 
    close(sockfd); 
    alarm(0);
    // Destroy the mutex
    pthread_mutex_destroy(&aesd_file_mutex);
    destroy_array_list(&threadsafearraylist);
    closelog();
    return 0;
}

void daemonize() {
    pid_t pid;

    // 1. Fork off the parent process
    pid = fork();
    if (pid < 0) {
        exit(EXIT_FAILURE);
    }
    if (pid > 0) {
        exit(EXIT_SUCCESS); // Parent exits
    }

    // 2. Create a new session
    if (setsid() < 0) {
        exit(EXIT_FAILURE);
    }

    

    // 4. Reset the file mode mask
    umask(0);

    // 5. Close all open file descriptors and redirect stdin/out/err to /dev/null
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    open("/dev/null", O_RDWR); // stdin
    dup(0); // stdout
    dup(0); // stderr

    
}
int main(int argc,char *argv[]) {
  if (argc > 2) {
    printf("Usage: %s <optional -D>\n",argv[0]);
    return 1;
  }
  int IsDaemon = 0;//default; w/o argunent
  if (argc == 2) {
    //check the passed in argument
    if (strcmp(argv[1],"-d")==0){
      printf("argument passed in = %s\n", argv[1]);
      IsDaemon = 1;
    }else {
      printf("Usage: %s <optional -D>\n",argv[0]);
      return 1;
    }
  }
  if (IsDaemon)
    daemonize();
  
  aesd_main();
  
  return 0;
}

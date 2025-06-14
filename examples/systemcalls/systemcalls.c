#include "systemcalls.h"
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
/**
 * @param cmd the command to execute with system()
 * @return true if the command in @param cmd was executed
 *   successfully using the system() call, false if an error occurred,
 *   either in invocation of the system() call, or if a non-zero return
 *   value was returned by the command issued in @param cmd.
*/


/*
//https://www.geeksforgeeks.org/system-call-in-c/
system() in C/C++
Last Updated : 11 Oct, 2024

The system() function is used to invoke an operating system command from a C/C++ program. For example, we can call system("dir") on Windows and system("ls") in a Unix-like environment to list the contents of a directory.

It is a standard library function defined in <stdlib.h> header in C and <cstdlib> in C++.
Syntax

The syntax of system() function is:

int system(const char *command);

Parameters

    command: A pointer to a null-terminated string that contains the command we want to execute.

Return Value

    It returns 0 if the command is successfully executed.
    It returns a non-zero value if command execution is not completed.
*/
bool do_system(const char *cmd)
{

/*
 * TODO  add your code here
 *  Call the system() function with the command set in the cmd
 *   and return a boolean true if the system() call completed with success
 *   or false() if it returned a failure
*/
    int retCode = system(cmd);
    if (retCode)
      return false;
    return true;
}

/**
* @param count -The numbers of variables passed to the function. The variables are command to execute.
*   followed by arguments to pass to the command
*   Since exec() does not perform path expansion, the command to execute needs
*   to be an absolute path.
* @param ... - A list of 1 or more arguments after the @param count argument.
*   The first is always the full path to the command to execute with execv()
*   The remaining arguments are a list of arguments to pass to the command in execv()
* @return true if the command @param ... with arguments @param arguments were executed successfully
*   using the execv() call, false if an error occurred, either in invocation of the
*   fork, waitpid, or execv() command, or if a non-zero return value was returned
*   by the command issued in @param arguments with the specified arguments.
*/

bool do_exec(int count, ...)
{
    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;
    // this line is to avoid a compile warning before your implementation is complete
    // and may be removed
    command[count] = command[count];

/*
 * TODO:
 *   Execute a system command by calling fork, execv(),
 *   and wait instead of system (see LSP page 161).
 *   Use the command[0] as the full path to the command to execute
 *   (first argument to execv), and use the remaining arguments
 *   as second argument to the execv() command.
 *
*/
//excec() as a family of functions -- https://stackoverflow.com/questions/20823371/what-is-the-difference-between-the-functions-of-the-exec-family-of-system-calls
//https://percona.community/blog/2021/01/04/fork-exec-wait-and-exit/
//https://linux.die.net/man/3/execv
//https://medium.com/@CyberGee/overview-of-the-use-of-fork-exit-wait-and-execve-the-linux-programming-interface-8d7d6509236f
//int execv(const char *path, char *const argv[]);
    va_end(args);
    int status;
    //flush to avoid double print
    fflush(stdout);
    pid_t pid = fork();
    if(pid<0){
      perror("fork fail");//in fork
      exit(1);
    }
    if (pid==0) { //child after fork
      //perform execv ,,execv will replace its current process
      status = execv(command[0],&command[1]);
      exit(status);
    }
    if (pid>0) { //parent after fork
      //wait for child to return and check for status
      pid = waitpid(pid,&status,0);
      if (status>0){
        exit(status);//execv failed
      }
    }
    return true;
}

/**
* @param outputfile - The full path to the file to write with command output.
*   This file will be closed at completion of the function call.
* All other parameters, see do_exec above
*/
bool do_exec_redirect(const char *outputfile, int count, ...)
{
    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
    }
    //insert > redirect command into file utputfile
    sprintf(command[count]," > %s " ,outputfile);
    command[count+1] = NULL;
    // this line is to avoid a compile warning before your implementation is complete
    // and may be removed
    command[count+1] = command[count];


/*
 * TODO
 *   Call execv, but first using https://stackoverflow.com/a/13784315/1446624 as a refernce,
 *   redirect standard out to a file specified by outputfile.
 *   The rest of the behaviour is same as do_exec()
 *
*/

    va_end(args);
    int status;
    //flush to avoid double print
    fflush(stdout);
    pid_t pid = fork();
    if(pid<0){
      perror("fork fail");//in fork
      exit(1);
    }
    if (pid==0) { //child after fork
      //perform execv ,,execv will replace its current process
      status = execv(command[0],&command[1]);
      exit(status);
    }
    if (pid>0) { //parent after fork
      //wait for child to return and check for status
      pid = waitpid(pid,&status,0);
      if (status>0){
        exit(status);//execv failed
      }
    }
    return true;
}

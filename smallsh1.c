#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/timeb.h>


// declare constants
// Spec: support command lines with a max length of 2048 characters
#define MAX_LENGTH 2048

// variable to store current process ID. This pid will be used to also kill child processes
//  before exiting
int pidNum;

// prototypes
void prompt();
void killProcesses();

void main(){

  pidNum = getpid();
  printf("THe pid number is %d\n", pidNum);
  prompt();
  int i;
  /*for(i = 0; i < 3; i++){
   pid_t pid = fork();
   printf("THe child process is:%d\n", pid);
  } */
}

void prompt(){
 bool runPrompt = true;
 while(runPrompt){
  /* 
 * Display ": " prompt to user
 * Set up string variable to hold user's entered command
 * The length of this string is MAX_LENGTH (2048) + 1 for the null terminator
 *
 * */
  printf(": ");
  char enteredCommand[MAX_LENGTH +1]; 
  fgets(enteredCommand, MAX_LENGTH, stdin);
  printf("You entered: %s\n", enteredCommand); 

  //  if user types in 'exit', break out of while loop
  // if user tries to exit &
  //  fgets adds a newline to entered text eg.  exit\n\0
  //  To get rid of this newline change \n to \0
  enteredCommand[strcspn(enteredCommand, "\n")] = '\0';
  if(strcmp(enteredCommand, "exit") ==0){
   printf("exiting loop\n");
   break;
  }

  if(strcmp(enteredCommand, "cd") == 0){
   char cwd[1024];
   getcwd(cwd, sizeof(cwd));
   printf("Current dir:%s\n", cwd);
   // chdir
   chdir("cd");
   getcwd(cwd, sizeof(cwd));
   printf("current dir:%s\n", cwd);
  }
 }  // end of while loop

} // end of prompt() 



/*
 * This method is called by prompt() when user types in exit
 * Purpose of this method: to kill all child jobs and processes
 */
void killProcesses(){
}

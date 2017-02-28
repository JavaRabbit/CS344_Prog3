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

 // variable to hold exit status. Used by built-in command status
 int exitStatus = 0;   // initialize to 0

 while(runPrompt){

  printf(": ");
  char enteredCommand[MAX_LENGTH +1]; 
  fgets(enteredCommand, MAX_LENGTH, stdin);
  //printf("You entered: %s\n", enteredCommand); 
  //printf("The length is %lu\n", strlen(enteredCommand)); 

  // fgets adds a new line to the entered text eg. exit\n\0
  // get rid of this newline. change \n to \0
  enteredCommand[strcspn(enteredCommand, "\n")] = '\0';

  // initialize array to store tokenized string from enteredCommand
  // NULL is appended to the end of the array
  // size of this array is 512 arguments
  char * words[513];    // update to explain why
  if(enteredCommand != NULL){
   int i = 0; // variable to tell where to put tokenized string
   char *p = strtok(enteredCommand, " ");
   while ( p != NULL){
    // add string to words array and increment i
    words[i++] = p;   
    p = strtok(NULL, " ");
   }

   // set words[i]  to NULL since we need to add NULL to the end
   // because execvp reads until NULL is found
   // -1 because the \n is added to the array
   // we want to replace the \n with NULL
   words[i] == NULL;
   
   int j;
   for(j = 0; j < i+1; j++){
    if(words[j] == NULL){
      printf("thisi is null\n");
    } else {
     printf("%s\n", words[j]);
    }
   } 
     
  }

  

  //  if user types in 'exit', break out of while loop
  // if user tries to exit &
  //  fgets adds a newline to entered text eg.  exit\n\0
  //  To get rid of this newline change \n to \0
  // enteredCommand[strcspn(enteredCommand, "\n")] = '\0';
  if(strcmp(enteredCommand, "exit") ==0){
   printf("exiting loop\n");
   break;
  }

  // if user types in "status", print out exit status
  if(strcmp(enteredCommand, "status") == 0){
   printf("%d\n", exitStatus);
   continue;  // continue to reshow prompt
  }

  // Check for comment starting with #
  // remember that the # sign might not be at enteredCommand[0]
  // example, user entered "   # this is stil a comment"
  // therefore we use a loop to loop over all chars of enteredCommand
  // if we detect a '#', use continue and do nothing since
  // we detect that this line is a comment
  char charVar = enteredCommand[0];
  int i;
  for(i = 0; i < strlen(enteredCommand) - 1; i++){
   charVar = enteredCommand[i];
   if(charVar == '#'){
    printf("comment detected\n"); // delete this line later
    continue;
   }
  } 


  // check if user entered a blank line. If so, shell should
  // do nothing. Use continue
  // NEEDS TO BE FIXED, bug here
  char firstChar = enteredCommand[0];
  if('\n' == firstChar){
   continue;
  }

  // needs to be updated to take in arguments to path location 
  if(strcmp(enteredCommand, "cd") == 0){
   char cwd[1024];
   getcwd(cwd, sizeof(cwd));
   printf("Current dir:%s\n", cwd);
   // chdir
   chdir(getenv("HOME"));
   getcwd(cwd, sizeof(cwd));
   printf("current dir:%s\n", cwd);
  }

  // Now that built ins have been checked, use fork and exit
  // to create processes 

 }  // end of while loop

} // end of prompt() 



/*
 * This method is called by prompt() when user types in exit
 * Purpose of this method: to kill all child jobs and processes
 */
void killProcesses(){
}

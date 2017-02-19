#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

// declare constants
// Spec: support command lines with a max length of 2048 characters
#define MAX_LENGTH 2048



// prototypes
void prompt();

void main(){

 printf("hello, more comments to test vim\n");

 /*
 bool f = false;
  if(f == false){
   printf("booleans work\n");
  }
  */
  prompt();
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
  printf("The len is %lu\n", strlen(enteredCommand));

  //  if user types in 'exit', break out of while loop
  // if user tries to exit &
  //  fgets adds a newline to entered text eg.  exit\n\0
  //  To get rid of this newline change \n to \0
  enteredCommand[strcspn(enteredCommand, "\n")] = '\0';
  if(strcmp(enteredCommand, "exit") ==0){
   printf("exiting loop\n");
   break;
  }
 }  // end of while loop

} // end of prompt() 

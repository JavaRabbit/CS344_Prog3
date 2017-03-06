#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/timeb.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>

// declare constants
// Spec: support command lines with a max length of 2048 characters
#define MAX_LENGTH 2048

// variable to store current process ID. This pid will be used to also kill child processes
//  before exiting
int pidNum;

// boolean variable for SIGTSTP signal. For Ctrl+Z, the boolean changes each time user
// enters Ctrl+Z.   Initialize to false since we allow users to do bg processes in the beginning
bool noBGallowed = false;


// boolean to hold whether last FOREGROUND process was killed by a signal
bool wasLastFGKilledBySignal = false;  
int numOfSignalKill = -10; // some bogus number

// int array to hold bg child pids
int bgChildren[2000];
int bgChildrenSize = 0; // size of the array





// prototypes
void prompt();
void killProcesses();
void sig_handler(int signo);
void sig_handler2(int signo);
void checkCompletedChildren();

void main(){

  pidNum = (int) getpid();
  printf("In main, pid number is %d\n", pidNum);
  
  // register the handler. 
  // If you get sigint signal, call this sig_handler function
  // control+c is SIGINT
  signal(SIGINT, sig_handler);
  signal(SIGTSTP, sig_handler2); // for control + z
  prompt();
}

void prompt(){
 bool runPrompt = true;
 
 // variable to hold exit status. Used by built-in command status
 int exitStatus = 0;   // initialize to 0

 while(runPrompt){
  
  bool isBGprocess = false; // variable to hold if user wanted background process

  // before printing prompt, each time, check if any children have exited
  checkCompletedChildren();
  printf(": ");

  // flush output stream
  fflush(stdout);
  fflush(stdin);  

  // string to hold users input 
  char enteredCommand[MAX_LENGTH +1]; 
  
  // get user input string 
  fgets(enteredCommand, MAX_LENGTH, stdin);
  //printf("%s\n", enteredCommand);
  fflush(stdout);
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



   // Use a loop to iterate over words[] array to check for "&".
   // If "&" is found, set isBGprocess boolean to true IF noBGallowd == false
   int bg_iterator = 0;
   while(words[bg_iterator] != NULL){
     if(strcmp(words[bg_iterator], "&") == 0){
       
       //  if noBGallowed boolean is set to true, don't set isBGprocess to true. 
       //  just replace "&" with NULL.   
       if(noBGallowed == true){
         words[bg_iterator] = NULL;
       }
       else {   /* bg processes ARE ALLOWED */
        isBGprocess = true; // set boolean to true. 
        words[bg_iterator] = NULL;  // replace & with NULL, since we found the end of the command
       }
     }
     bg_iterator++;
   } // end while

 
     int bg = 0;
     while(words[bg] != NULL){
     // Also compare each element of word[] to see if it contains "$$"
     char *n;
     //char * pp = "$$";
     n = strstr(words[bg], "$$");
     if(n != NULL){

      // function to convert integer value to string
      char pidNumString[10];
      //int pidNum2 = (int)getpid(); 
      //printf("The pid num2 is %d\n", pidNum2);
      //printf("the pid num is %d\n", pidNum); 
      
      sprintf(pidNumString, "%d", pidNum);
      //  the length of the pid is usually 5.  it does not count terminator 
      //printf("The string is %s\n and len is %lu", pidNumString, strlen(pidNumString));
      
      // Now append this pidNumString to $$
      char newString[20] = ""; // allocate too much space
      strcpy(newString, words[bg]);  // copy the string into newString
      //printf("the new string is:%s and length is %lu\n", newString, strlen(newString));
     
      
   
      // try to memcpy the pidNumString to newString at location 
      // integer to hold length of  command without the $$.  example foo$$ is len 5. but we 
      // want to start to copy pidNumString at location 3.  thus we need to chop off the last 2 chars(the $$)
      int locStart = strlen(newString) - 2;
      // copy into newString starting from locStart, pidNumString, using the strlen of pidNumString      
      memcpy(newString + locStart, pidNumString, strlen(pidNumString)); 
      //printf("the new string is:%s and length is %lu\n", newString, strlen(newString));

      //fflush(stdout);

      //  now that newString contains our variable expansion, replace words[bg_iterator] with newString;
     // words[bg_iterator] = newString;
     strcpy(words[bg], newString);  
     //strcpy(newString, '\0'); // free the newString so it doesn't cause bugs
     }  // end if n != NULL 


     bg++;
     } // end while
   

   // set words[i]  to NULL since we need to add NULL to the end
   // because execvp reads until NULL is found
   words[i] = NULL;
  
  /* 
   int j;
   for(j = 0; j < i+1; j++){
    if(words[j] == NULL){
      printf("thisi is null\n");
    } else {
     printf("%s\n", words[j]);
    }
   }
   */ 
     
  }  // end if enteredCommand != NULL

  

  //  if user types in 'exit', break out of while loop
  // if user tries to exit &
  //  fgets adds a newline to entered text eg.  exit\n\0
  //  To get rid of this newline change \n to \0
  // enteredCommand[strcspn(enteredCommand, "\n")] = '\0';
  if(strcmp(enteredCommand, "exit") == 0){
   //printf("exiting loop\n"); WRITE METHOD TO KILL CHILDREN
   killProcesses();
   exit(0);
   //break;
  }

  // if user types in "status", print out exit status
  else if(strcmp(enteredCommand, "status") == 0){
   // check if the last FG process was killed by signal or not
   if(wasLastFGKilledBySignal == false){
    printf("exit value %d\n", exitStatus);
   } else {  /* yes, terminated by singal */
    printf("terminated by signal %d\n", numOfSignalKill);
   }

   wasLastFGKilledBySignal = false;  // set to false since "status" cmd is not killed by signal
   
   // also set the exit status to 0 since status return 0
   exitStatus = 0;
 
   fflush(stdout);

   continue;  // continue to reshow prompt
  }

   
  // Comments are ignored. Comments also update status to 0
  else if(enteredCommand[0] == '#'){
   exitStatus = 0; // update exit status
   wasLastFGKilledBySignal = false; // set to false since #commands are not fg killed by signals
   continue;
  }
  

  // check if user entered a blank line. If so, shell should
  // do nothing. Use continue. Blank lines do not change exitStatus
  else if('\0' == enteredCommand[0]){
   continue;
  }
  
 
  // needs to be updated to take in arguments to path location
  // if the first word that the user typed is "cd", use this conditional to change directories 
  else if(strcmp(words[0], "cd") == 0){
   
   // if user only typed in "cd", (eg. there is not 2nd argument or path)
   if(words[1] == NULL ){
    chdir(getenv("HOME"));
   }
   // else if user typed in valid path, go to that path
   // create a variable to record if chdir was sucessful. 0 means sucessful completion
   int chdirSuccess;
   if(words[1] != NULL){
    chdirSuccess = chdir(words[1]);
   }

   if(chdirSuccess == 0){
     exitStatus = 0; // set exit status to 0 for successful dir change
   }

   // else if path is invalid, print error to user
   if(chdirSuccess != 0){
    printf("cd: %s: No such file or directory\n", words[1]);
    fflush(stdout);
    exitStatus = 1; // set exit status to 1 since unable to change dir
   }

  // set boolean to false since cd commands are fg commands not killed by sigals
   wasLastFGKilledBySignal = false;
  }

  // Now that built ins have been checked, use fork and exit
  // to create processes 
  else {
   pid_t spawnPid = -5;
   int childExitMethod = -5;
   
   spawnPid = fork();
   switch(spawnPid){
   // case 0 means the child will execute this code
   case 0: { 
    
    // 3 cases. 1 no redirection. 2. only output redirection 3. input redirection
    // 4 both input and output redirection

    // check for < and > using the words array
    // initialize another array to hold the 'command array without > <"

    char * noRedir[513];
    int i = 0;
    int noRedirPointer = 0;   // variable to hold location of noRedir array
    int lt = -5; // variable to hold < location. Initialize to negative number
    int gt = -5;  // variable to hold > location. INitialize to random neg number
    while(words[i] != NULL){
     
     // if words[i] == ">", set location of gt
     if(strcmp(words[i], ">") == 0){
      gt = i; // set location of the > sign
      i++;  // increment i but not noRedirPointer
      continue;
     } else if(strcmp(words[i], "<") ==0){
      lt = i;
      i++;
      continue;
     } else{
      noRedir[noRedirPointer] = words[i];
      i++; // increment i
      noRedirPointer++; // increment this pointer since we added values to noRedir array
     }

    } // end of while loop

    // append NULL to location end of the noRedirarray
    noRedir[noRedirPointer] = NULL;
    
    /* 
    int p = 0;
    while( noRedir[p] != NULL){
     printf("%s\n", noRedir[p]);
     p++;
    }
    */

    // CASE 1:  NO redirection. User did not use ">" or "<" 
    if(lt < 0 && gt <0 ){
     exitStatus = execvp(words[0], words);  // just run the command
     printf("%s: no such file or directory\n", words[0]);   // get here only if execvp did not complete successfully
     exitStatus = 1;   // set exit status to 1 as per spec
    } 
    else if( lt < 0 && gt > 0){   // temp set to if, reset to else if
      // CASE 2: User only used " >"   eg.   ls > somefile.txt
      int outVal; // an integer to hold value of returned value

      // output file is specified at words[gt + 1] eg. index position 1 further than > 
      // check if file words[lt+1]  is an existing output file. If yes, append to this file
      // Per the spec: output file is truncated if it exists, or created if it does not exist
      // either open and truncate, or create new file

      outVal = open(words[gt+1], O_WRONLY | O_APPEND | O_CREAT, 0664);   

      // replace standard output with output file
      dup2(outVal, 1);

      close(outVal);  // close file descriptor
     
      // create a new array to hold command up to ">"
      char * tempArr[513]; 
      int wordsPointer = 0;
      for(wordsPointer = 0; wordsPointer < gt; wordsPointer++){
       tempArr[wordsPointer] = words[wordsPointer];  // copy into tempArr the first part of the command
      }
      tempArr[wordsPointer] = NULL;   // set the end of the command to null
      int k = execvp(tempArr[0], tempArr); // run the command
     }  // end if lt < 0 && gt > 0 
     
     // CASE 3:  if user used "<" but not ">"
     else if(lt > 0 && gt < 0){   /* eg.  wc < somefile.txt   */
      // set file descriptor for input file
      int inputVal;
 
      // test if words[lt+1] is null, user di not specify file or input.  
      // redirect to "/dev/null"
      if(words[lt+1] == NULL){
        //printf("No input file found\n");
        //fflush(stdout);
        //exitStatus = 1;
        //continue;
        
        // change as per spec. if User did not specifiy input use "/dev/null"
        strcpy(words[lt+1], "/dev/null"); 
        
      }
 
      // try and read the input file located at words[lt + 1]
      inputVal = open(words[lt +1], O_RDONLY);
      // if input file is not found, send error message to user
      if(inputVal < 0){
       printf("cannot open %s for input\n", words[lt+1]);
       fflush(stdout);
       exitStatus = 1; // set exit status to 1 since invalid file user is trying to open
       continue;   // to break out of this loop and reshow : prompt to user		     
      }

      // input file is found, replace standard input with input file
      dup2(inputVal, 0);

      close(inputVal); // close file descriptor

      // make a copy of the command up until the "<"
      char * tempArr[513];
      int wordsPointer = 0;
      // use a for loop to copy the command up until the "<"  eg.  wc < someFile.txt
      for(wordsPointer = 0; wordsPointer < lt; wordsPointer++){
       tempArr[wordsPointer] = words[wordsPointer];
      }
      tempArr[wordsPointer] == NULL; // set the end of the truncated command to NULL
 
      // now execute the command
      execvp(tempArr[0], tempArr);     
      exitStatus = 1; // set to 1 if execvp did not go sucessfully
      continue; // to go to top of while loop
     }   // end if lt > 0 && gt < 0
  
     // CASE 4: user uses both < and >
     else if( lt > 0 && gt > 0){
       //printf(" used both\n");
       // set both file descriptors
       int inputVal;
       int outputVal;
 
       inputVal = open(words[lt+1], O_RDONLY); // open the input file, the one specified after the "<" less than
   
       // check if file is found, else send message to user and reshow : prompt
       if(inputVal < 0){
        printf("cannot open %s for input\n", words[lt+1]);
        fflush(stdout);
        exitStatus = 1;
        continue;
       }   // end if inputVal < 0

       outputVal = open(words[gt+1], O_WRONLY | O_APPEND | O_CREAT, 0664);
      
       // replace standard input with input file
       dup2(inputVal, 0);

      // replace standard output with output file
      dup2(outputVal, 1);

      // close file descriptors
      close(inputVal);
      close(outputVal);

      // now copy the command which is "some cmd < someInput > someOutputFile"
      // meaning we want to get the strings before the "<"
      char *tempArr[513]; 
      int wordsPointer = 0;
      
      // use for loop to copy command up to the "<"
      for(wordsPointer = 0; wordsPointer < lt; wordsPointer++){
        tempArr[wordsPointer] = words[wordsPointer];
      }
   
      tempArr[wordsPointer] = NULL; // set end of command to NULL
       
      // execute the command
      exitStatus = execvp(tempArr[0], tempArr);
  
     } // end CASE 4, user used both < and >

 
    // if command does not work, set built-in status to 1
    //exitStatus = 1;
    //printf("Command did not work. exit status: %d\n", exitStatus);
   } // end case 0
   default: {
    
   // Spec: shell will print the process id of a background process when it begins
   if(isBGprocess == true){
     printf("background pid is %d\n", spawnPid);
     fflush(stdout);

     // add to bgChildren array
     bgChildren[bgChildrenSize] = spawnPid;
     printf("again the bg pid is %d\n", bgChildren[bgChildrenSize]);
     bgChildrenSize++; 
   }

   // since the last FG process was NOT killed by a signal, set boolean to false
   wasLastFGKilledBySignal = false;


    // 3 parameters are pid of process waiting for, pointer to int to be filled with
    // exit status, then options
    // if  "is background" use WNOHANG,  else for foreground use, 0 for option
   
    if(isBGprocess == false){
     waitpid(spawnPid, &childExitMethod, 0);   // use 0 as options. 
    } 
   } // end default
   }// end of switch
  }  




 }  // end of while loop

} // end of prompt() 



/*
 * This method is called by prompt() when user types in exit
 * Purpose of this method: to kill all child jobs and processes
 */
void killProcesses(){
 //printf("kill Processes called %d\n", pidNum);
  
 // iterate over bgChildren array and kill of bgChildren
 int i = 0;
 for(i = 0; i < bgChildrenSize; i++){
  kill(bgChildren[i], SIGKILL);
  printf("killed of child %d\n", bgChildren[i]); 
 }

  exit(0);

}

// method to continually check if child processes have completed
void checkCompletedChildren(){
 int exitedChildMethod = -5; // set to bogus value
 int childPid = waitpid(-1, &exitedChildMethod, WNOHANG);
 
 // if childPid == 0, no processes have terminated, just return
 if(childPid ==0 ){
  return;
 }

 // a background child process has terminated. Print to user
 if (WIFEXITED(exitedChildMethod) != 0){  // != means exited normally
  printf("background pid %d is done: exit value %d\n", childPid , WEXITSTATUS(exitedChildMethod) );
  fflush(stdout);

  // search bgChildren[] array, and remove from the array
  int i = 0;
  for(i = 0; i < bgChildrenSize; i++){
   if(bgChildren[i] == childPid){
    printf("found in bg array\n");
    bgChildrenSize--;
    printf("size is now %d\n", bgChildrenSize);
    fflush(stdout);
    
    // Now move elements over if needed
    int j = i+1;
    for(j = i+1; j < bgChildrenSize; j++){
       bgChildren[j-1] = bgChildren[j];
    }
   }  // end if bgChildren[i] == childPid
  } // end for loop
 }  // end if(WIFEXITED...
 
} // end of void checkCompletedChildren

/* Handler function to kill child process.
 * It is the parent that will print out number of signal
 * that killed the child process
 */
void sig_handler(int signo){
 int m = getpid();  
 printf("received SIGINT %d. Killed by signal %d\n",m,signo);

 // also set the boolean to "true"
 // and set the signal number which killed this foreground process
 wasLastFGKilledBySignal = true; 
 numOfSignalKill = signo;

 fflush(stdout);
}


// This method handles signals for Cntrl + Z. It toggles the boolean "noBGallowed"
void sig_handler2(int signo){
 
 // if noBGallowed is false, set noBGallowed to true, and vice versa
 if( noBGallowed == false){
   noBGallowed = true;
   //  print message to user to bg processes are no longer allowed
   printf("\nEntering foreground-only mode (& is now ignored)\n"); // \r for carriage return
 }  else {
    // set noBGallowed to false.  Therefore allowing background processes again.
    noBGallowed = false;
    printf("\nExiting foreground-only mode\n"); 
 }
 fflush(stdout);
}


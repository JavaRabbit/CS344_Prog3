#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

void main(){

 pid_t spawnpid = -5;
 int ten = 10;

 spawnpid = fork();
 switch(spawnpid){
 case -1:
  perror("breach");
  exit(1);
  break;
 case 0:
 ten = ten+1;
 printf("I am the child. ten = %d\n", ten);
 break;
 default:
 ten = ten -1;
 printf(" I am the parent. ten = %d\n", ten);
 break;
 }
 printf(" This is executed by both\n"); 
}

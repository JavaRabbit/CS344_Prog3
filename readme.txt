To Compile:

gcc -o smallsh smallsh.c


Note: there is a bug in my program where if a child execvp fails, the exitStatus remains at 0.
This is because parent and child have their own set of variables. I'm trying to figure out how to have
a child change a parent variable.





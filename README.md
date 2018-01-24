# C-code-cleaner
The program removes all the C programming style comments and empty lines from the given file/files and saves the "clean" version to a new file with name "original_name.clean".

# BUILD INSTRUCTIONS


Run "make" to build all. If you make any changes to memory.c or memory.h you have to run "make clean" before building to build the changes. So "make clean" cleans all the files

If you want to clean up, use "make clean"


# USE INSTRUCTIONS

To remove comments from files run 

	./prog filename1 filename2 filename3 ...

So ./prog is the program to be runned and after that just put all the files separated by whitespace.

The cleaned output files can be found by they .clean postfix.

The log file of the last run ./prog can be found in a file prog.log.

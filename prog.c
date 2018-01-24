#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>

#include "loglib.h"
#include "memory.h"

#define BUFLEN 1024
static int fd_log; //for the log file
static M_tracker *mem; //for memory freeing
static char log_buf[1024]; //for writing log entries


void prepare_log()
{	
	char *write_log_buf;
	write_log_buf = calloc(strlen(log_buf) + 1, 1);
	/*sets the memory address to the memory tracker*/
	mem = set_memory(mem, write_log_buf);
	strcpy(write_log_buf, log_buf);
	write_log(write_log_buf, fd_log);
	/*frees the memory address from the memory tracker*/
	free_memory(mem, write_log_buf);
	log_buf[0] = 0;
}


void signal_handler(int signal)
{
	int p;
	switch (signal) {
		case SIGTERM:
		{	
			printf("PID %d\n", getpid());
			release_memory(mem);
			break;
		}

		case SIGINT:
		{
			sprintf(log_buf, "Caught SIGINT in %d. Sending SIGTERM to every child process \n",getpid());
            prepare_log();

			//sending SIGTERM to all the processses with the same group id
			kill(0, SIGTERM);

			//leaving time for children to exit gracefully
			sleep(1); 
			while ((p=waitpid(-1, 0, WNOHANG)) != -1) {
		    	sprintf(log_buf, "Killing zombie process with PID = %d\n", p);
		    	prepare_log();

		    	//force kill to remaining zombie process
		       	kill(p, SIGKILL);
	       	
	    	} //while

	    	strcpy(log_buf, "\nAll child processes killed. Freeing all the memory and exiting\n");
			prepare_log();

			release_memory(mem);
			close(fd_log);
		}
	} //switch

	close(fd_log);

	exit(0);
}


int main (int argc, char **argv)
{
	if (argc == 1) {
		printf("Please enter at least one file name\n");
		return -1;
	}

	pid_t pid;
	/*removing the previos log file if exists*/
	remove("prog.log");

	/*allocating memory for the memory tracker itself*/
	mem = calloc(sizeof(M_tracker), 1);

	fd_log = open("prog.log", O_RDWR | O_APPEND | O_CREAT | O_NONBLOCK, S_IRWXU);
	
	sprintf(log_buf, "This is the parent process: PID = %d\n",getpid());
	prepare_log();

	//Setting up signal handling
	struct sigaction sa;
	sigemptyset(&sa.sa_mask);
	sa.sa_handler = &signal_handler;
	sa.sa_flags=0;
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);

	for (unsigned int i = 1; i < argc; i++) {

		pid = fork();

		/*fork failed*/
		if (pid < 0) {
			strcpy(log_buf, "Fork failed\n");
			prepare_log();
			release_memory(mem);
			printf("%s\n", log_buf);
            return -1;
		}

		// child process
		if (pid == 0) {
			
			sprintf (log_buf, "\nThis is a child process: PID = %d\n",getpid());
			prepare_log();

			/*exec() doesn't return so all the memory for this child process has 
			to be freed before that*/
			release_memory(mem); 

			//arguments for cleaner main
			char *args[] = {"./cleaner", argv[i], NULL};
			execvp(args[0], args);	
		}
	}

	//waiting for all child processes to exit
	while (waitpid(-1, 0, WNOHANG) != -1) { }
	strcpy(log_buf, "\nAll child processes have returned. Freeing all the memory and exiting\n");
	prepare_log();
	close(fd_log);
	release_memory(mem);
	return 0;

} // main

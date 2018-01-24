#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>

#include "loglib.h"

#define BUFLEN 1024
static int fd_log;


void signal_handler(int signal)
{
	int p;
	char log_buf[1024] = { 0 };
	char *write_log_buf;
	if (signal == SIGTERM) {
		printf("%s\n", "term mainissa");
	}
	if (signal == SIGINT) 
	{
		printf("%d\n", getpid());

		if (!fd_log)
			printf("Can't open log file\n");

		kill(0, SIGTERM);

		sleep(1); //leaving time for children to exit gracefully

		while ((p=waitpid(-1, 0, WNOHANG)) != -1) {
	    	sprintf(log_buf, "Killing zombie process with PID = %d\n", p);
	    	printf("%s\n", log_buf);
	    	write_log_buf = calloc(strlen(log_buf) + 1, 1);
			strcpy(write_log_buf, log_buf);
			write_log(write_log_buf, fd_log);
	       	kill(p, SIGKILL);
	       	
	    } //while
	}
	/*kill(0, SIGTERM);

	sprintf(log_buf, "Killing process with PID = %d\n",getpid());
	 //leave time for processes to exit;

	write_log_buf = calloc(strlen(log_buf) + 1, 1);
	strcpy(write_log_buf, log_buf);
	write_log(write_log_buf, fd_log);

	sleep(1);*/

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
	char log_buf[1024] = { 0 };
	char *write_log_buf;
	remove("prog.log");

	fd_log = open("prog.log", O_RDWR | O_APPEND | O_CREAT | O_NONBLOCK, S_IRWXU);
	
	sprintf(log_buf, "This is the parent process: PID = %d\n",getpid());
	write_log_buf = calloc(strlen(log_buf) + 1, 1);
	strcpy(write_log_buf, log_buf);
	write_log(write_log_buf, fd_log);

	struct sigaction sa;
	sigemptyset(&sa.sa_mask);
	sa.sa_handler = &signal_handler;
	sa.sa_flags=0;
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);

	//signal(SIGINT, signal_handler);

	for (unsigned int i = 1; i < argc; i++) {

		pid = fork();

		if (pid < 0) {
			strcpy(log_buf, "Fork failed\n");
			write_log_buf = calloc(strlen(log_buf) + 1, 1);
			strcpy(write_log_buf, log_buf);
			write_log(write_log_buf, fd_log);
			printf("%s\n", log_buf);
            return -1;
		}

		// child process
		if (pid == 0) {
			
			sprintf (log_buf, "This is a child process: PID = %d\n",getpid());
			write_log_buf = calloc(strlen(log_buf) + 1, 1);
			strcpy(write_log_buf, log_buf);
			write_log(write_log_buf,fd_log);
			char *args[] = {"./cleaner", argv[i], NULL};
			execvp(args[0], args);	
		}

		//else {
           //waitpid(pid,0,0); /* wait for child to exit */
    	//}
	}

	while (waitpid(-1, 0, WNOHANG) != -1) { }
	close(fd_log);
	return 0;

} // main

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "loglib.h"
#include "memory.h"
 
 /*this program removes comments and empty lines from C code*/

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

void signal_handler(int signal) {
    switch (signal) {
        case SIGTERM:
        {
            sprintf(log_buf, "Caught SIGTERM in %d. Freeing memory and exiting\n",getpid());
            prepare_log();
            release_memory(mem);
            close(fd_log);
            exit(0);
        }
        case SIGINT:
        {
            /*Waiting for SIGTERM or some other action*/
            sleep(1);
            break;
        }
    }
}

void set_lock(int fd, int type)
{
    struct flock lock;

    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;

    while (1) {
        lock.l_type = type;
        /* if we get the lock, return immediately */
        if (!fcntl(fd, F_SETLK, &lock)) {
            sprintf(log_buf, "\tSetting lock to fd %d in PID %d\n", fd, getpid());
            prepare_log();
            return;
        }
    }
}

int remove_comments(const char *filename, const char *output)
{
    FILE *r_file = fopen(filename, "r");

    if (!r_file) {
    	sprintf(log_buf, "Cannot open file %s\n", filename);
        prepare_log();
        return -1;
    }

    mem = set_fp(mem, r_file);

    /*Setting reading lock*/
    set_lock(fileno(r_file), F_RDLCK);
    
    /* tmp = temporary file descriptor to store te uncommented version with empty lines*/
    FILE *tmp = fopen(output, "w");
    if(!tmp) {
        sprintf(log_buf, "Cannot open file %s\n", output);
        prepare_log();
        return -1;
    }
    mem = set_fp(mem, tmp);

    /*Setting writing lock*/
    set_lock(fileno(tmp), F_WRLCK);

    /*Removing comments.
    c = current char*/
    char c = fgetc(r_file);
    char next;
    while(c != EOF) {
    	next = fgetc(r_file);
    	if (c == '/' && next == '/') {
            while (c != '\n' && c != EOF) {
                c = fgetc(r_file);
            }
            c = fgetc(r_file);
        }
        else if (c == '/' && next == '*') {
            while (!(c == '*' && next == '/')) {
                c = next;
                next = fgetc(r_file);
            }
            c = fgetc(r_file);
        }
        else {
            fputc(c, tmp);
            c = next;
        }
        
    } //while
    close_fp(mem, r_file);
    close_fp(mem, tmp);

    return 0;

} //remove_comments

int remove_empty_lines(const char *orig, const char *filename)
{
    char* c_name = calloc(strlen(orig) + strlen(".clean") + 1, 1);
    mem = set_memory(mem, c_name);
    strcpy(c_name, orig);
    strcat(c_name, ".clean");

    /*final clean file*/

    FILE *c_file = fopen(c_name, "w");

    if(!c_file) {
        sprintf(log_buf, "Cannot open file %s\n", c_name);
        printf("%s\n", log_buf);
        prepare_log();
        return -1;
    }
    mem = set_fp(mem, c_file);

    set_lock(fileno(c_file), F_WRLCK);

    FILE *tmp = fopen(filename, "r");

    if(!tmp) {
        sprintf(log_buf, "Cannot open file %s\n", filename);
        printf("%s\n", log_buf);
        prepare_log();
        return -1;
    }
    mem = set_fp(mem, tmp);

    set_lock(fileno(tmp), F_RDLCK);

    char str[100];
    int flag = 0, i = 0;

    while (!feof(tmp)) {
        fgets(str, 100, tmp);
        while (str[i] != '\0') {
    		if (!isspace((unsigned char) str[i])) {
      			flag = 1;
      			break;
      		}
    		i++;
        }

        if (flag)
        	fputs(str, c_file);

        strcpy(str, "\0");
        flag = 0, i = 0;
    }

    free_memory(mem, c_name);
    close_fp(mem, tmp);
    close_fp(mem, c_file);

    remove(filename);

    return 0;
}

int main(int argc, char const *argv[])
{
	if (argc == 1) {
		printf("Please enter at least one file name\n");
		return -1;
	}
    /*allocating memory for the memory tracker itself*/
    mem = calloc(sizeof(M_tracker), 1);

    /*Setting up signal handling*/
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_handler = &signal_handler;
    sa.sa_flags=0;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);  

    fd_log = open("prog.log", O_RDWR | O_APPEND | O_CREAT | O_NONBLOCK, S_IRWXU);

	sprintf(log_buf, "\nRemoving comments and empty lines from file %s \n", argv[1]);
    printf("%s", log_buf);
    prepare_log();

	char *tmp = calloc(strlen(argv[1]) + 5, 1);
    mem = set_memory(mem, tmp);
    strcpy(tmp, argv[1]);
    strcat(tmp, ".tmp");

	if (remove_comments(argv[1], tmp) == -1) {
		free_memory(mem, tmp);
		return -1;
	}

	if (remove_empty_lines(argv[1], tmp) == -1) {
		free_memory(mem, tmp);
		return -1;
	}

	free_memory(mem, tmp);
    close(fd_log);
    release_memory(mem);

	return 0;
}
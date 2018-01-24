#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>

#include "loglib.h"
 
 /*this program removes comments and empty lines from C code*/

#define BUFLEN 1024
static int fd_log;

void signal_handler(int signal) {
    switch (signal) {
        case SIGTERM:
        {
            printf("SIGTERM in %d\n",getpid());
            break;
        }
        case SIGINT:
        {
            printf("SIGINT in %d\n",getpid());
            break;
        }
        
        
    }
}

void set_lock(int fd, int type)
{
    struct flock lock;
    char log_buf[1024] = { 0 };
    char *write_log_buf;

    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;

    while (1) {
        lock.l_type = type;
        /* if we get the lock, return immediately */
        if (!fcntl(fd, F_SETLK, &lock)) {
            sprintf(log_buf, "Setting lock to fd %d in PID %d\n", fd, getpid());
            write_log_buf = calloc(strlen(log_buf) + 1, 1);
            strcpy(write_log_buf, log_buf);
            write_log(write_log_buf, fd_log);
            return;
        }

        /* find out who holds the conflicting lock
        fcntl(fd, F_GETLK, &lock);
        if (lock.l_type != F_UNLCK) {
            /*sprintf(log_buf, "Can't set file lock, conflict with process %d", lock.l_pid);
            write_log_buf = calloc(strlen(log_buf) + 1, 1);
            strcpy(write_log_buf, log_buf);
            write_log(write_log_buf, fd_log);
            printf("%s\n", "apua");
        }*/
    }
}


int remove_comments(const char *filename, const char *output)
{
    char log_buf[1024] = { 0 };
    char *write_log_buf;

    //int r_file = open(filename, O_RDONLY | O_CREAT, S_IRWXU);
    FILE *r_file = fopen(filename, "r");
    if (!r_file) {
    	sprintf(log_buf, "Cannot open file %s\n", filename);
        printf("%s\n", log_buf);
        write_log_buf = calloc(strlen(log_buf) + 1, 1);
        strcpy(write_log_buf, log_buf);
        write_log(write_log_buf, fd_log);
        return -1;
    }

    set_lock(fileno(r_file), F_RDLCK);
    
    // tmp = temporary file descriptor to store te uncommented version with empty lines
    //int tmp = open(output, O_WRONLY | O_CREAT, S_IRWXU);
    FILE *tmp = fopen(output, "w");
    if(!tmp) {
        sprintf(log_buf, "Cannot open file %s\n", output);
        printf("%s\n", log_buf);
        write_log_buf = calloc(strlen(log_buf) + 1, 1);
        strcpy(write_log_buf, log_buf);
        write_log(write_log_buf, fd_log);
        return -1;
    }

    set_lock(fileno(tmp), F_WRLCK);

    //c = current char
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
        
    }
    fclose(r_file);
    fclose(tmp);

    return 0;

} //remove_comments

int remove_empty_lines(const char *orig, const char *filename)
{
    char* c_name = calloc(strlen(orig) + strlen(".clean") + 1, 1);
    strcpy(c_name, orig);
    strcat(c_name, ".clean");
    char log_buf[1024] = { 0 };
    char *write_log_buf;

    //final clean file

    //int c_file = open(c_name, O_WRONLY | O_CREAT, S_IRWXU);

    FILE *c_file = fopen(c_name, "w");

    if(!c_file) {
        sprintf(log_buf, "Cannot open file %s\n", c_name);
        printf("%s\n", log_buf);
        write_log_buf = calloc(strlen(log_buf) + 1, 1);
        strcpy(write_log_buf, log_buf);
        write_log(write_log_buf, fd_log);
        return -1;
    }

    set_lock(fileno(c_file), F_WRLCK);

    //int tmp = open(filename, O_RDONLY | O_CREAT, S_IRWXU);
    FILE *tmp = fopen(filename, "r");

    if(!tmp) {
        sprintf(log_buf, "Cannot open file %s\n", filename);
        printf("%s\n", log_buf);
        write_log_buf = calloc(strlen(log_buf) + 1, 1);
        strcpy(write_log_buf, log_buf);
        write_log(write_log_buf, fd_log);
        return -1;
    }

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

    free(c_name);
    fclose(tmp);
    fclose(c_file);

    remove(filename);

    return 0;
}

int main(int argc, char const *argv[])
{
	if (argc == 1) {
		printf("Please enter at least one file name clean.c\n");
		return -1;
	}
    //signal(SIGINT, signal_handler);

    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_handler = &signal_handler;
    sa.sa_flags=0;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL); 
    sleep(10);   


    char log_buf[1024] = { 0 };
    char *write_log_buf;
    fd_log = open("prog.log", O_RDWR | O_APPEND | O_CREAT | O_NONBLOCK, S_IRWXU);

	sprintf(log_buf, "Removing comments and empty lines from file %s \n", argv[1]);
    printf("%s\n", log_buf);
    write_log_buf = calloc(strlen(log_buf) + 1, 1);
    strcpy(write_log_buf, log_buf);
    write_log(write_log_buf, fd_log);

	char *tmp = calloc(strlen(argv[1]) + 5, 1);
    strcpy(tmp, argv[1]);
    strcat(tmp, ".tmp");

	if (remove_comments(argv[1], tmp) == -1) {
		free(tmp);
		return -1;
	}

	if (remove_empty_lines(argv[1], tmp) == -1) {
		free(tmp);
		return -1;
	}

	free(tmp);
    close(fd_log);
    sleep(20);

	return 0;
}
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "loglib.h"

/*Writes contents of buffer into a logfile with file descriptor fd*/
void write_log(char* buf, int fd)
{
	int ret;
   if (!fd)
            printf("Can't open log file\n");
	while ((ret = write(fd, buf, strlen(buf))) == -1) {
   		if (ret == EWOULDBLOCK || ret == EAGAIN) {
            /*Someone else is writing to a file*/
   			printf("waiting to write to log file\n");
   			continue;
   		}
   		else {
   			printf("Some error with write\n");
   			return;
   		}
   	}
}

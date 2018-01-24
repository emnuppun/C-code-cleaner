#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "loglib.h"

void write_log(char* buf, int fd)
{
	int ret;
	while ((ret = write(fd, buf, strlen(buf))) == -1) {
   		if (ret == EWOULDBLOCK || ret == EAGAIN) {
   			printf("waiting to write to log file\n");
   			continue;
   		}
   		else {
   			printf("Some error with write\n");
   			return;
   		}
   	}
   	free(buf);
}

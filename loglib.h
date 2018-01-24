
#ifndef _LOG_H_
#define _LOG_H_ 

/* Writes contents of buf to socket fd. In this program this is used to write log entries. */

void write_log(char* buf, int fd);

#endif
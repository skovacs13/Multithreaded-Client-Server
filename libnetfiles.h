#ifndef LIBNETFILES_H
#define LIBNETFILES_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

int netopen(const char *pathname, int flags);
ssize_t netread(int fildes, void *buf, size_t nbyte);
ssize_t netwrite(int fildes, const void *buf, size_t nbyte);
int netclose(int fd);
void helpSock(char c);
void error(char *msg);

#endif

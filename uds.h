#ifndef _UDS_H_
#define _UDS_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <stdio.h>

int uds_listen(const char *path);
int uds_connect(const char *path);
int uds_accept(int fd);

#endif

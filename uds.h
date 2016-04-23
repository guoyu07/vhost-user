#ifndef _UDS_H_
#define _UDS_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <stdio.h>

struct uds_ctx {
	int fd;
	struct sockaddr_un sa;
};

struct uds_ctx * uds_listen(const char *path);
struct uds_ctx * uds_dgram(const char *path);
struct uds_ctx * uds_connect(const char *path);
int uds_accept(int fd);
int uds_send_dgram(struct uds_ctx *ctx, void *buf, int len);
int uds_recv_dgram(struct uds_ctx *ctx, void *buf, int len);
void uds_close(struct uds_ctx *ctx);

#endif

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include <sys/select.h>

#include "vhost.h"
#include "vhost_msg.h"

u64 vhost_supported_features = (0);

static int n_vhost_server;
static struct vhost_ctx vhost_servers[16];
struct vhost_ctx *vhost_get_ctx(int fd)
{
	int i;
	for (i = 0; i < n_vhost_server; i++) {
		if (vhost_servers[i].fd == fd)
			return &vhost_servers[i];
	}
	if (n_vhost_server >= 16)
		return NULL;
	n_vhost_server++;
	return &vhost_servers[n_vhost_server-1];
}
static int uds_accept_handler(int fd)
{
	int connfd;

	connfd = uds_accept(fd);
	if (connfd < 0) {
		perror("accept");
		return 0;
	}
	return connfd;
}
static int msg_handler(int fd)
{
	return vhost_msg_handler(fd, vhost_get_ctx(fd));
}

void vhost_user_start(const char *path)
{
	int i;
	int rc;
	int fd;
	int nfds;
	fd_set rfds;
	struct vhost_ctx *ctx, *new_ctx;

	nfds = 0;
	FD_ZERO(&rfds);

	fd = uds_listen(path);
	ctx = vhost_get_ctx(fd);
	ctx->handler = uds_accept_handler;
	assert(ctx);
	FD_SET(ctx->fd, &rfds);
	nfds = fd + 1;

	while (1) {
		rc = select(nfds, &rfds, NULL, NULL, NULL);
		if (rc < 0) {
			perror("select");
			continue;
		}
		if (rc == 0) {
			printf("strange\n");
			continue;
		}
		for (i = 0; i < n_vhost_server; i++) {
			ctx = &vhost_servers[i];
			if (FD_ISSET(ctx->fd, &rfds)) {
				rc = ctx->handler(ctx->fd);
				if (rc > 0) {
					printf("new conn\n");
					new_ctx = vhost_get_ctx(fd);
					if (!new_ctx) {
						printf("server busy\n");
						continue;
					}
					FD_SET(rc, &rfds);
					if (new_ctx->fd >= nfds)
						nfds = new_ctx->fd + 1;
				} else if (rc < 0) {
					printf("handler failed, close conn\n");
					//close(ctx->fd);
				}
				FD_SET(ctx->fd, &rfds);
			}
		}
	}
}

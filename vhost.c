#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include <sys/select.h>

#include "vhost.h"
#include "vhost_ctx.h"
#include "vhost_msg.h"
#include "uds.h"

int vhost_debug;

/* The feature bitmap for virtio net */
#define VIRTIO_NET_F_CSUM       0       /* Host handles pkts w/ partial csum */
#define VIRTIO_NET_F_GUEST_CSUM 1       /* Guest handles pkts w/ partial csum */
#define VIRTIO_NET_F_MAC        5       /* Host has given MAC address. */
#define VIRTIO_NET_F_GUEST_TSO4 7       /* Guest can handle TSOv4 in. */
#define VIRTIO_NET_F_GUEST_TSO6 8       /* Guest can handle TSOv6 in. */
#define VIRTIO_NET_F_GUEST_ECN  9       /* Guest can handle TSO[6] w/ ECN in. */
#define VIRTIO_NET_F_GUEST_UFO  10      /* Guest can handle UFO in. */
#define VIRTIO_NET_F_HOST_TSO4  11      /* Host can handle TSOv4 in. */
#define VIRTIO_NET_F_HOST_TSO6  12      /* Host can handle TSOv6 in. */
#define VIRTIO_NET_F_HOST_ECN   13      /* Host can handle TSO[6] w/ ECN in. */
#define VIRTIO_NET_F_HOST_UFO   14      /* Host can handle UFO in. */
#define VIRTIO_NET_F_MRG_RXBUF  15      /* Host can merge receive buffers. */
#define VIRTIO_NET_F_STATUS     16      /* virtio_net_config.status available */
#define VIRTIO_NET_F_CTRL_VQ    17      /* Control channel available */
#define VIRTIO_NET_F_CTRL_RX    18      /* Control channel RX mode support */
#define VIRTIO_NET_F_CTRL_VLAN  19      /* Control channel VLAN filtering */
#define VIRTIO_NET_F_CTRL_RX_EXTRA 20   /* Extra RX mode control support */
#define VIRTIO_NET_F_GUEST_ANNOUNCE 21  /* Guest can announce device on the
					                                            * network */
#define VIRTIO_NET_F_MQ         22      /* Device supports Receive Flow
					                                            * Steering */
#define VIRTIO_NET_F_CTRL_MAC_ADDR 23   /* Set MAC address */

#define VIRTIO_F_VERSION_1 32

#define VHOST_SUPPORTS_MQ      (1ULL << VIRTIO_NET_F_MQ)

#define VHOST_SUPPORTED_FEATURES \
		((1ULL << VIRTIO_F_VERSION_1) | \
		 (1ULL << VIRTIO_NET_F_GUEST_CSUM) | \
		 (1ULL << VIRTIO_NET_F_GUEST_TSO4) | \
		 (1ULL << VIRTIO_NET_F_GUEST_TSO6))



#define VHOST_SUPPORTED_FEATURES2 ((1ULL << VIRTIO_NET_F_MRG_RXBUF) | \
		(1ULL << VIRTIO_NET_F_CTRL_VQ) | \
		(1ULL << VIRTIO_NET_F_CTRL_RX) | \
		(1ULL << VIRTIO_NET_F_GUEST_ANNOUNCE) | \
		(VHOST_SUPPORTS_MQ)            | \
		(1ULL << VIRTIO_F_VERSION_1)   | \
		(1ULL << VHOST_F_LOG_ALL)      | \
		(1ULL << VHOST_USER_F_PROTOCOL_FEATURES) | \
		(1ULL << VIRTIO_NET_F_HOST_TSO4) | \
		(1ULL << VIRTIO_NET_F_HOST_TSO6) | \
		(1ULL << VIRTIO_NET_F_CSUM)    | \
		(1ULL << VIRTIO_NET_F_GUEST_CSUM) | \
		(1ULL << VIRTIO_NET_F_GUEST_TSO4) | \
		(1ULL << VIRTIO_NET_F_GUEST_TSO6))

u64 vhost_supported_features = (VHOST_SUPPORTED_FEATURES);

int n_vhost_server;
struct vhost_ctx vhost_servers[16];
struct vhost_ctx *vhost_get_ctx(int fd)
{
	int i;
	for (i = 0; i < n_vhost_server; i++) {
		if (vhost_servers[i].fd == fd)
			return &vhost_servers[i];
	}
	if (n_vhost_server >= 16)
		return NULL;
	vhost_log("new context\n");
	n_vhost_server++;
	return &vhost_servers[n_vhost_server-1];
}
struct virtio_dev * vhost_get_first_virtio(void)
{
	int i;
	for (i = 0; i < n_vhost_server; i++) {
		if ((vhost_servers[i].fd > 0) && vhost_servers[i].dev) {
			return vhost_servers[i].dev;
		}
	}
	return NULL;
}

static int uds_accept_handler(int fd)
{
	int connfd;

	connfd = uds_accept(fd);
	if (connfd < 0) {
		perror("accept");
		return -1;
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
	struct uds_ctx *uctx;

	nfds = 0;
	FD_ZERO(&rfds);

	uctx = uds_listen(path);
	if (!uctx) {
		perror("unix");
		exit(-1);
	}
	ctx = vhost_get_ctx(uctx->fd);
	assert(ctx);
	ctx->fd = uctx->fd;
	ctx->handler = uds_accept_handler;
	FD_SET(ctx->fd, &rfds);
	nfds = uctx->fd + 1;

	/* buggy */
	while (1) {
		rc = select(nfds, &rfds, NULL, NULL, NULL);
		if (rc < 0) {
			perror("select");
			continue;
		}
		if (rc == 0) {
			vhost_log("strange\n");
			continue;
		}
		vhost_log("someone comes\n");
		for (i = 0; i < n_vhost_server; i++) {
			ctx = &vhost_servers[i];
			if (FD_ISSET(ctx->fd, &rfds)) {
				rc = ctx->handler(ctx->fd);
				if (rc > 0) {
					vhost_log("new conn\n");
					new_ctx = vhost_get_ctx(rc);
					if (!new_ctx) {
						vhost_log("server busy\n");
						continue;
					}
					new_ctx->fd = rc;
					new_ctx->handler = msg_handler;
					FD_SET(rc, &rfds);
					if (rc >= nfds)
						nfds = rc + 1;
				} else if (rc < 0) {
					vhost_log("handler failed, close conn\n");
					//close(ctx->fd);
					if (ctx->dev)
						virtio_dump_dev(ctx->dev);
					exit(-1);
				}
				FD_SET(ctx->fd, &rfds);
			}
		}
	}
}

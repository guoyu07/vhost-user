#ifndef _VHOST_CTX_H_
#define _VHOST_CTX_H_

struct vhost_ctx {
	int fd;
	int (*handler) (int);
	struct virtio_dev *dev;
};
extern int n_vhost_server;
struct vhost_ctx vhost_servers[16];
struct vhost_ctx *vhost_get_ctx(int fd);

#endif

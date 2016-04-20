#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <string.h>

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "vhost.h"
#include "vhost_msg.h"

extern int errno;

#define VHOST_USER_VRING_IDX_MASK	(0xff)
#define VHOST_USER_VRING_NOFD_MASK	(0x100) 

static int vhost_read_msg(int fd, struct vhost_msg *msg)
{
	size_t hdrsz;
	int rc;
	struct iovec iov;
	struct msghdr msgh;
	size_t fdsize = sizeof(msg->fds);
	char control[CMSG_SPACE(fdsize)];	
	struct cmsghdr *cmsg;

	hdrsz = offsetof(struct vhost_msg, num);

	memset(&msgh, 0, sizeof(msgh));
	iov.iov_base = msg;
	iov.iov_len = hdrsz;

	msgh.msg_iov = &iov;
	msgh.msg_iovlen = 1;
	msgh.msg_control = control;
	msgh.msg_controllen = sizeof(control);

	rc = recvmsg(fd, &msgh, 0);
	if (rc <= 0) {
		perror("read");
		return -1;
	}

	if (msgh.msg_flags & (MSG_TRUNC | MSG_CTRUNC)) {
		vhost_log("truncted msg\n");
		return -1;
	}

	for (cmsg = CMSG_FIRSTHDR(&msgh); cmsg != NULL;
			cmsg = CMSG_NXTHDR(&msgh, cmsg)) {
		if ((cmsg->cmsg_level == SOL_SOCKET) &&
				(cmsg->cmsg_type == SCM_RIGHTS)) {
			memcpy(msg->fds, CMSG_DATA(cmsg), fdsize);
		}
	}

	if (msg->len) {
		rc = read(fd, &msg->num, msg->len);
		if (rc == msg->len)
			return 0;
		if (rc < 0) {
			return -1;
		}
		if (rc != msg->len) {
			perror("read");
			return -1;
		}
	}
	return 0;
}

static int vhost_reply_msg(int fd, struct vhost_msg *msg)
{
	int rc;
	size_t count;

#define VHOST_USER_VERSION_MASK	0x3
#define VHOST_USER_REPLY_MASK	(0x1 << 2)
#define VHOST_USER_VERSION	0x1

	vhost_log("reply\n");

	msg->flags &= ~VHOST_USER_VERSION_MASK;
	msg->flags |= VHOST_USER_VERSION;
	msg->flags |= VHOST_USER_REPLY_MASK;

	count = offsetof(struct vhost_msg, num) + msg->len;
	rc = write(fd, msg, count);
	if (rc != count) {
		perror("write");
		vhost_log("reply failed\n");
		return -1;
	}
	return 0;
}

static int vhost_new_device(struct vhost_ctx *ctx)
{
	vhost_log("new device\n");
	ctx->dev = (struct virtio_dev *)malloc(sizeof(struct virtio_dev));
	assert(ctx->dev);
	memset(ctx->dev, 0, sizeof(struct virtio_dev));
}

/* y is 2^n */
#define ROUNDUP(x, y) (((x)+(y)-1) & (~((y)-1)))

static int vhost_user_set_mem_table(struct virtio_dev *dev, struct vhost_msg *msg)
{
	int i;
	size_t size;
	void *mmap_addr;
	struct virtio_mem_region *regions;
	struct vhost_user_mem *memory = &msg->memory;

	dev->mem = (struct virtio_mem *)malloc(sizeof(struct virtio_mem));
	assert(dev->mem);
	dev->mem->nregions = memory->nregions;
	regions = dev->mem->regions;
	for (i = 0; i < memory->nregions; i++) {
		regions[i].guest_address = memory->regions[i].guest_address;
		regions[i].user_address = memory->regions[i].user_address;
		regions[i].size = memory->regions[i].size;

		size = memory->regions[i].size + memory->regions[i].mmap_offset;
		/* size aligned at 2MB */
		size = ROUNDUP(size, 2<<20);
		vhost_log("memfd %d orig offset %lx mmapped size %lx\n",
				msg->fds[i], memory->regions[i].mmap_offset, size);
		mmap_addr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, msg->fds[i], 0);
		if (mmap_addr == MAP_FAILED) {
			perror("mmap");
			vhost_log("mmap failed\n");
			return -1;
		}
		regions[i].address_offset = (u64)mmap_addr + memory->regions[i].mmap_offset - memory->regions[i].guest_address;
	}
	return 0;
}

int vhost_msg_handler(int connfd, struct vhost_ctx *ctx)
{
	int fd;
	u16 index;
	struct virtqueue *vq;
	struct vhost_msg msg;
	struct virtio_dev *dev;

	if (vhost_read_msg(connfd, &msg)) {
		vhost_log("cannot read msg from socket\n");
		return -1;
	}

	dev = ctx->dev;

	switch (msg.request) {
		case VHOST_USER_GET_FEATURES:
			vhost_log("get features\n");
			msg.num = vhost_supported_features;
			msg.len = sizeof(msg.num);
			vhost_reply_msg(connfd, &msg);
			break;
		case VHOST_USER_SET_FEATURES:
			vhost_supported_features = msg.num;
			vhost_log("set features:%lx\n", vhost_supported_features);
			break;
		case VHOST_USER_GET_PROTOCOL_FEATURES:
			vhost_log("not implemented\n");
			break;
		case VHOST_USER_SET_PROTOCOL_FEATURES:
			vhost_log("not implemented\n");
			break;
		case VHOST_USER_SET_OWNER:
			vhost_new_device(ctx);
			vhost_log("set owner\n");
			break;
		case VHOST_USER_RESET_OWNER:
			vhost_log("reset owner\n");
			/* ignore it */
			break;
		case VHOST_USER_SET_MEM_TABLE:
			vhost_log("set mem table\n");
			vhost_user_set_mem_table(dev, &msg);
			break;
		case VHOST_USER_SET_LOG_BASE:
			vhost_log("not implemented\n");
			break;
		case VHOST_USER_SET_LOG_FD:
			vhost_log("not implemented\n");
			break;
		case VHOST_USER_SET_VRING_NUM:
			dev->vq[msg.state.index].num = msg.state.num;
			vhost_log("set vring num: %d\n", msg.state.num);
			break;
		case VHOST_USER_SET_VRING_ADDR:
			vq = &dev->vq[msg.addr.index];
			vq->desc = qva_to_va(dev, msg.addr.desc);
			vq->used = qva_to_va(dev, msg.addr.used);
			vq->avail = qva_to_va(dev, msg.addr.avail);
			/* FIXME */
			vq->log = qva_to_va(dev, msg.addr.log);
			vhost_log("vq %d orig vring addr: %lx %lx %lx %lx\n",
					msg.addr.index,
					msg.addr.desc, msg.addr.used,
					msg.addr.avail, msg.addr.log);
			vhost_log("vq %d set vring addr: %p %p %p %p\n",
					msg.addr.index,
					vq->desc, vq->used, vq->avail, vq->log);
			break;
		case VHOST_USER_SET_VRING_BASE:
			dev->vq[msg.state.index].last_used_idx = msg.state.num;
			vhost_log("set vring base: index %d base %d\n", msg.state.index, msg.state.num);
			break;
		case VHOST_USER_GET_VRING_BASE:
			/* qemu stops */
			msg.state.num = dev->vq[msg.state.index].last_used_idx;
			msg.len = sizeof(msg.state);
			vhost_reply_msg(connfd, &msg);
			vhost_log("get vring base\n");
			break;
		case VHOST_USER_SET_VRING_KICK:
			index = msg.num & VHOST_USER_VRING_IDX_MASK;
			if (msg.num & VHOST_USER_VRING_NOFD_MASK)
				fd = VIRTIO_INVALID_EVENTFD;
			else
				fd = msg.fds[0];
			if (dev->vq[index].kickfd > 0) {
				vhost_dbg("close kickfd %d\n", dev->vq[index].kickfd);
				close(dev->vq[index].kickfd);
			}
			dev->vq[index].kickfd = fd;
			vhost_log("set vring kick: index %d kick %d\n", index, fd);
			break;
		case VHOST_USER_SET_VRING_CALL:
			index = msg.num & VHOST_USER_VRING_IDX_MASK;
			if (msg.num & VHOST_USER_VRING_NOFD_MASK)
				fd = VIRTIO_INVALID_EVENTFD;
			else
				fd = msg.fds[0];
			if (dev->vq[index].callfd > 0) {
				vhost_dbg("close callfd %d\n", dev->vq[index].callfd);
				close(dev->vq[index].callfd);
			}
			dev->vq[index].callfd = fd;
			vhost_log("set vring call: index %d call %d\n", index, fd);
			break;
		case VHOST_USER_SET_VRING_ERR:
			vhost_log("set vring err\n");
			/* ignore it */
			break;
		case VHOST_USER_GET_QUEUE_NUM:
			vhost_log("get queue num\n");
			msg.len = sizeof(u64);
			msg.num = 1;
			vhost_reply_msg(connfd, &msg);
			break;
		case VHOST_USER_SET_VRING_ENABLE:
			vhost_log("set vring enable\n");
			break;
		case VHOST_USER_SEND_RARP:
			vhost_log("not implemented\n");
			break;
		default:
			vhost_log("bad request");
			return -1;
			break;
	}

	return 0;
}



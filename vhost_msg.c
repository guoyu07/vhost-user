#include "vhost.h"
#include "vhost_msg.h"

static int vhost_user_set_mem_table(struct virtio_dev *dev, struct vhost_msg *msg)
{
	int i;
	void *mmap_addr;
	struct vhost_user_mem *memory = &msg->memory;

	dev->mem = (struct virtio_mem *)malloc(sizeof(struct virtio_mem));
	assert(dev->mem);
	dev->mem->nregions = memory->nregions;
	regions = dev->mem->regions;
	for (i = 0; i < memory->nregions; i++) {
		regions[i].guest_address = memory->regions[i].guest_address;
		regions[i].user_address = memory->regions[i].user_address;
		size = memory->regions[i].size + memory->regions[i].mmap_offset;
		mmap_addr = mmap(NULL, size, PROT_READ | PROT_WRITE, MMAP_SHARED, msg->fds[i]);
		if (mmap_addr == MAP_FAILED)
			return -1;
		regions[i].addr_offset = mmap_addr - memory->regions[i].guest_address;
	}
	return 0;
}

static int vhost_msg_handler(int connfd, struct vhost_ctx *ctx)
{
	struct vhost_msg msg;
	struct virtio_dev *dev;

	if (vhost_read_msg(connfd, &msg)) {
		vhost_log("cannot read msg from socket\n");
		return -1;
	}

	dev = vhost_ctx_get_dev(ctx);

	switch (msg.request) {
		case VHOST_USER_GET_FEATURES:
			vhost_log("get features\n");
			msg.len = sizeof(u64);
			payload.num = vhost_supported_features;
			vhost_reply_msg(connfd, &msg);
			break;
		case VHOST_USER_SET_FEATURES:
			vhost_log("set features\n");
			vhost_supported_features = msg.num;
			break;
		case VHOST_USER_GET_PROTOCOL_FEATURES:
			vhost_log("not implemented\n");
			break;
		case VHOST_USER_SET_PROTOCOL_FEATURES:
			vhost_log("not implemented\n");
			break;
		case VHOST_USER_SET_OWNER:
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
			vhost_log("set vring num\n");
			dev->vq[msg.state.index].num = msg.state.num;
			break;
		case VHOST_USER_SET_VRING_ADDR:
			vhost_log("set vring addr\n");
			vq = &dev->vq[msg.addr.index];
			vq->desc = qva_to_va(msg.addr.desc);
			vq->used = qva_to_va(msg.addr.used);
			vq->avail = qva_to_va(msg.addr.avail);
			vq->log = qva_to_va(msg.addr.log);
			break;
		case VHOST_USER_SET_VRING_BASE:
			dev->vq[msg.state.index].last_used_idx = msg.state.num;
			vhost_log("set vring base\n");
			break;
		case VHOST_USER_GET_VRING_BASE:
			msg.state.num = dev->vq[msg.state.index].last_used_idx;
			vhost_log("get vring base\n");
			break;
		case VHOST_USER_SET_VRING_KICK:
			index = msg.num & VHOST_USER_VRING_IDX_MASK;
			if (msg.num & VHOST_USER_VRING_NOFD_MASK)
				fd = VIRTIO_INVALID_EVENTFD;
			else
				fd = msg.fds[0];
			dev->vq[index].kickfd = fd;
			vhost_log("set vring kick\n");
			break;
		case VHOST_USER_SET_VRING_CALL:
			index = msg.num & VHOST_USER_VRING_IDX_MASK;
			if (msg.num & VHOST_USER_VRING_NOFD_MASK)
				fd = VIRTIO_INVALID_EVENTFD;
			else
				fd = msg.fds[0];
			dev->vq[index].callfd = fd;
			vhost_log("get vring call\n");
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



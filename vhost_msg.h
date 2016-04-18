#ifndef _VHOST_MSG_H_
#define _VHOST_MSG_H_

#include "types.h"
#include "vhost.h"

enum {
	VHOST_USER_NONE = 0,
	VHOST_USER_GET_FEATURES = 1,
	VHOST_USER_SET_FEATURES = 2,
	VHOST_USER_SET_OWNER = 3,
	VHOST_USER_RESET_OWNER = 4,
	VHOST_USER_SET_MEM_TABLE = 5,
	VHOST_USER_SET_LOG_BASE = 6,
	VHOST_USER_SET_LOG_FD = 7,
	VHOST_USER_SET_VRING_NUM = 8,
	VHOST_USER_SET_VRING_ADDR = 9,
	VHOST_USER_SET_VRING_BASE = 10,
	VHOST_USER_GET_VRING_BASE = 11,
	VHOST_USER_SET_VRING_KICK = 12,
	VHOST_USER_SET_VRING_CALL = 13,
	VHOST_USER_SET_VRING_ERR = 14,
	VHOST_USER_GET_PROTOCOL_FEATURES = 15,
	VHOST_USER_SET_PROTOCOL_FEATURES = 16,
	VHOST_USER_GET_QUEUE_NUM = 17,
	VHOST_USER_SET_VRING_ENABLE = 18,
	VHOST_USER_SEND_RARP = 19,
	VHOST_USER_MAX
};

struct vhost_vring_state {
	u32 index;
	u32 num;
};
struct vhost_vring_addr {
	u32 index;
	u32 flags;
	/* all below are qemu user address */
	u64 desc;
	u64 used;
	u64 avail;
	u64 log;
};
struct vhost_user_region {
	u64 guest_address;
	u64 size;
	u64 user_address;
	u64 mmap_offset;
};
struct vhost_user_mem {
	u32 nregions;
	u32 padding;
	struct vhost_user_region regions[VIRTIO_MAX_REGION];
};
struct vhost_user_log {
	u32 size;
	u32 offset;
};
struct vhost_msg {
	u32 request;
	u32 flags;
	u32 len;
	union {
		u64 num;
		struct vhost_vring_state state;
		struct vhost_vring_addr addr;
		struct vhost_user_mem memory;
		struct vhost_user_log log;
	};
	int fds[VIRTIO_MAX_REGION];
}__attribute__((packed));


int vhost_msg_handler(int connfd, struct vhost_ctx *ctx);

#endif

#ifndef _VHOST_MSG_H_
#define _VHOST_MSG_H_

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
};

#endif

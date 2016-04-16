#ifndef _VIRTIO_H_
#define _VIRTIO_H_

#include "types.h"

#define MAX_QUEUE_SIZE	(0xffff)
#define MAX_QUEUE_PAIR	(1024)
#define VIRTIO_MAX_REGION	(8)
#define VIRTIO_INVALID_EVENTFD	(-1)

struct virtq_desc {
	/* guest physical address */
	u64 addr;
	u32 len;
	u16 flags;
	u16 next;
};

struct virtq_avail {
	u16 flags;
	u16 idx;
	u16 ring[MAX_QUEUE_SIZE];
	u16 used_event;
};

struct virtq_used_elem {
	u32 id;
	u32 len;
};

struct virtq_used {
	u16 flags;
	u16 idx;
	struct virtq_used_elem ring[MAX_QUEUE_SIZE];
	u16 used_event;
};

struct virtqueue {
	u32 num;

	struct virtq_desc *desc;
	struct virtq_avail *avail;
	struct virtq_used *used;
	u8 *log;

	u16 last_used_idx;

	int kickfd;
	int callfd;
};

struct virtio_mem_region {
	u64 guest_address;
	u64 user_address;
	u64 address_offset;
};
struct virtio_mem {
	u32 nregions;
	struct virtio_mem_region regions[VIRTIO_MAX_REGION];
};

struct virtio_dev {
	struct virtqueue vq[MAX_QUEUE_PAIR];
	struct virtio_mem *mem;
};

#endif

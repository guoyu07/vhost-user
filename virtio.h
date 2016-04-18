#ifndef _VIRTIO_H_
#define _VIRTIO_H_

#include "types.h"

/**
 * This is the first element of the scatter-gather list.  If you don't
 * specify GSO or CSUM features, you can simply ignore the header.
 */
struct virtio_net_hdr {
#define VIRTIO_NET_HDR_F_NEEDS_CSUM 1    /**< Use csum_start,csum_offset*/
	uint8_t flags;
#define VIRTIO_NET_HDR_GSO_NONE     0    /**< Not a GSO frame */
#define VIRTIO_NET_HDR_GSO_TCPV4    1    /**< GSO frame, IPv4 TCP (TSO) */
#define VIRTIO_NET_HDR_GSO_UDP      3    /**< GSO frame, IPv4 UDP (UFO) */
#define VIRTIO_NET_HDR_GSO_TCPV6    4    /**< GSO frame, IPv6 TCP */
#define VIRTIO_NET_HDR_GSO_ECN      0x80 /**< TCP has ECN set */
	uint8_t gso_type;
	uint16_t hdr_len;     /**< Ethernet + IP + tcp/udp hdrs */
	uint16_t gso_size;    /**< Bytes to append to hdr_len per frame */
	uint16_t csum_start;  /**< Position to start checksumming from */
	uint16_t csum_offset; /**< Offset after that to place checksum */
};

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
	u16 ring[0];
};

struct virtq_used_elem {
	u32 id;
	u32 len;
};

struct virtq_used {
	u16 flags;
	u16 idx;
	struct virtq_used_elem ring[0];
};

struct virtqueue {
	struct virtq_desc *desc;
	struct virtq_avail *avail;
	struct virtq_used *used;
	u8 *log;
	u64 logsz;

	u32 num;
	int kickfd;
	int callfd;

	u16 last_used_idx;
};

struct virtio_mem_region {
	u64 guest_address;
	u64 user_address;
	u64 address_offset;
	u64 size;
};
struct virtio_mem {
	u32 nregions;
	u32 padding;
	struct virtio_mem_region regions[VIRTIO_MAX_REGION];
};

struct virtio_dev {
	struct virtqueue vq[MAX_QUEUE_PAIR*2];
	struct virtio_mem *mem;
};

#endif

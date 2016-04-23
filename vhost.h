#ifndef _VHOST_H_
#define _VHOST_H_

#include "virtio.h"
#include <stdio.h>

extern int vhost_debug;

#define vhost_log(fmt, args...)	\
	do {fprintf(stderr, fmt, ##args);} while (0)
#define vhost_dbg(fmt, args...) \
	do {if (vhost_debug) fprintf(stderr, fmt, ##args);} while (0)

extern u64 vhost_supported_features;

void vhost_user_start(const char *path);

struct virtio_dev * vhost_get_first_virtio(void);

static inline void *qva_to_va(struct virtio_dev *dev, u64 qva)
{
	int i;
	struct virtio_mem_region *region;

	vhost_dbg("qva_to_va\n");

	for (i = 0; i < dev->mem->nregions; i++)
	{
		region = &dev->mem->regions[i];
		if ((qva >= region->user_address) &&
				(qva <= (region->user_address + region->size)))
			return (void*)(region->address_offset + region->guest_address + qva - region->user_address);
	}

	return 0;
}

static inline void *gpa_to_va(struct virtio_dev *dev, u64 gpa)
{
	int i;
	struct virtio_mem_region *region;

	vhost_dbg("gpa_to_va\n");

	for (i = 0; i < dev->mem->nregions; i++)
	{
		region = &dev->mem->regions[i];
		if ((gpa >= region->guest_address) &&
				(gpa <= (region->guest_address + region->size)))
			return (void*)(region->address_offset + gpa);
	}

	return 0;
}

static inline void virtio_dump_dev(struct virtio_dev *dev)
{
	int i;
	int nregion;
	struct virtio_mem_region *reg;
	struct virtqueue *vq;

	if (!dev->mem)
		return;

	nregion = dev->mem->nregions;
	vhost_log("dev %p %d regions\n", dev, nregion);
	for (i = 0; i < nregion; i++) {
		reg = &dev->mem->regions[i];
		vhost_log("\tguest %p user %p offset %p size %lu\n",
				(void*)reg->guest_address, (void*)reg->user_address,
				(void*)reg->address_offset, reg->size);
	}
	vq = &dev->vq[0];
	vhost_log("\tvq0 kickfd %d callfd %d desc %p used %p ava %p idx %hu\n",
			vq->kickfd, vq->callfd,
			vq->desc, vq->used, vq->avail,
			vq->avail->idx);
	vhost_log("last_used_idx %hu\n", vq->last_used_idx);
	vq = &dev->vq[1];
	vhost_log("\tvq1 kickfd %d callfd %d desc %p used %p ava %p idx %hu\n",
			vq->kickfd, vq->callfd,
			vq->desc, vq->used, vq->avail,
			vq->avail->idx);
	vhost_log("last_used_idx %hu\n", vq->last_used_idx);
}


#endif

#ifndef _VHOST_H_
#define _VHOST_H_

#include "virtio.h"
#include <stdio.h>

#define vhost_log(fmt, args...)	do {fprintf(stderr, fmt, ##args);} while (0)

extern u64 vhost_supported_features;

void vhost_user_start(const char *path);

static inline void *qva_to_va(struct virtio_dev *dev, u64 qva)
{
	int i;
	struct virtio_mem_region *region;

	vhost_log("qva_to_va\n");

	for (i = 0; i < dev->mem->nregions; i++)
	{
		region = &dev->mem->regions[i];
		if (qva >= region->user_address &&
				(qva <= region->user_address + region->size))
			return (void*)(region->address_offset + region->guest_address + qva - region->user_address);
	}

	return 0;
}

static inline void *gpa_to_va(struct virtio_dev *dev, u64 gpa)
{
	int i;
	struct virtio_mem_region *region;

	vhost_log("gpa_to_va\n");

	for (i = 0; i < dev->mem->nregions; i++)
	{
		region = &dev->mem->regions[i];
		if (gpa >= region->guest_address &&
				(gpa <= region->guest_address + region->size))
			return (void*)(region->address_offset + gpa);
	}

	return 0;
}

#endif

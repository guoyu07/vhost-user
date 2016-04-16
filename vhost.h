#ifndef _VHOST_H_
#define _VHOST_H_

#include "virtio.h"
#include <stdio.h>

#define vhost_log(fmt, args...)	do {printf(fmt, ##args);} while (0)

extern u64 vhost_supported_features;

void vhost_user_start(const char *path);

static inline void *qva_to_va(u64 qva)
{
	return 0;
}

static inline void *gpa_to_va(u64 gpa)
{
	return 0;
}

#endif

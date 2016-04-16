#ifndef _VHOST_H_
#define _VHOST_H_

#include "virtio.h"
#include <stdio.h>

#define vhost_log(fmt, args...)	do {fprintf(stderr, fmt, ##args);} while (0)

extern u64 vhost_supported_features;

void vhost_user_start(const char *path);

static inline void *qva_to_va(u64 qva)
{
	vhost_log("qva_to_va\n");
	return 0;
}

static inline void *gpa_to_va(u64 gpa)
{
	vhost_log("gpa_to_va\n");
	return 0;
}

#endif

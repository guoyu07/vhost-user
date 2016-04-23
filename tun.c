#include "tun.h"
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/if_tun.h>
#include <net/if.h>

int tap_open(const char *name)
{
	int rc;
	int fd;
	struct ifreq ifr;

	fd = open("/dev/net/tun", O_RDWR);
	if (fd < 0) {
		return -1;
	}

	memset(&ifr, 0, sizeof(ifr));
	ifr.ifr_flags = IFF_NO_PI | IFF_TAP;
	strcpy(ifr.ifr_name, name);
	rc = ioctl(fd, TUNSETIFF, &ifr);
	if (rc < 0) {
		return -1;
	}
	return fd;
}

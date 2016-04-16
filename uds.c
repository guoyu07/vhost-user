#include "uds.h"

int uds_listen(const char *path)
{
	int fd;
	int rc;
	struct sockaddr_un un;

	fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (fd < 0) {
		return -1;
	}
	memset(&un, 0, sizeof(struct sockaddr_un));
	un.sun_family = AF_UNIX;
	sprintf(un.sun_path, "%s", path);
	rc = bind(fd, (struct sockaddr *)&un, sizeof(struct sockaddr_un));
	if (rc < 0) {
		return -1;
	}
	rc = listen(fd, 1024);
	if (rc < 0) {
		return -1;
	}
	return fd;
}

int uds_connect(const char *path)
{
	int fd;
	int rc;
	struct sockaddr_un un;

	fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (fd < 0) {
		return -1;
	}
	memset(&un, 0, sizeof(struct sockaddr_un));
	un.sun_family = AF_UNIX;
	sprintf(un.sun_path, "%s", path);
	rc = connect(fd, (struct sockaddr *)&un, sizeof(struct sockaddr_un));
	if (rc < 0) {
		return -1;
	}
	return fd;
}

int uds_accept(int fd)
{
	int rc;

	rc = accept(fd, 0, 0);
	if (rc < 0) {
		return -1;
	}
	return rc;
}

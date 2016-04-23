#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include <unistd.h>

#include "types.h"
#include "vhost.h"
#include "mbuf.h"
#include "tun.h"

static int sendto_peer(int fd, struct mbuf *m)
{
	int rc;

	rc = write(fd, m->data, m->len);
	if (rc < 0) {
		perror("write");
		return 0;
	}
	return 1;
}

static int recvfrom_peer(int fd, struct mbuf **mbuf)
{
	int rc;
	struct mbuf *m;

	m = vhost_new_mbuf();
	if (!m) {
		vhost_log("no mbuf\n");
		exit(-1);
	}
	rc = read(fd, m->data, m->len);
	if (rc < 0) {
		perror("read");
		vhost_free_mbuf(m);
		return 0;
	}
	m->len = rc;
	*mbuf = m;
	return 1;
}

static int can_read(int fd)
{
	fd_set fs;
	struct timeval to;
	int rc;

	FD_ZERO(&fs);
	FD_SET(fd, &fs);

	to.tv_sec = to.tv_usec = 0;

	rc = select(fd+1, &fs, NULL, NULL, &to);
	if (rc > 0) {
		if (FD_ISSET(fd, &fs))
			return 1;
		vhost_log("can_read: strange");
	} else if (rc < 0) {
		perror("select");
	}
	return 0;
}

static void *worker_fn(void *data)
{
	int i;
	int np;
	int fd;
	struct virtio_dev *dev;
	struct mbuf *m;

	vhost_log("worker start...\n");

check:
	/* make sure dev is running */
	sleep(10);

	dev = NULL;
	while (!dev) {
		dev = vhost_get_first_virtio();
		sleep(1);
	}

	vhost_log("qemu comes...\n");

	fd = tap_open("tap0");
	if (fd < 0) {
		perror("tap");
		exit(-1);
	}

	/* data path is ready */

	/* check dev running ? */

	while (1) {
		np = vhost_tx(dev, 1, &m, 1);
		if (np > 0) {
			np = sendto_peer(fd, m);
			if (np > 0) {
				//vhost_dump_mbuf(m);
				vhost_log("tx to uds\n");
			} else {
				vhost_log("failed to send\n");
			}
			vhost_free_mbuf(m);
		}
		if (can_read(fd)) {
			np = recvfrom_peer(fd, &m);
			if (np > 0) {
				//vhost_dump_mbuf(m);
				vhost_log("rx from uds\n");
				vhost_rx(dev, 0, &m, 1);
				vhost_free_mbuf(m);
			}
		}
		usleep(10);
	}
}

static void vhost_user_worker_start(void)
{
	pthread_t t;
	int rc;

	rc = pthread_create(&t, NULL, worker_fn, NULL);
	if (rc) {
		vhost_log("cannot start worker\n");
		exit(-1);
	}
	rc = pthread_detach(t);
	if (rc) {
		vhost_log("cannot detach worker\n");
		exit(-1);
	}
}

int main(int argc, char **argv)
{
	assert(argc == 2);

	vhost_log("server start...\n");

	vhost_user_worker_start();

	vhost_user_start(argv[1]);

	return 0;
}

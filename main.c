#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include <unistd.h>

#include "types.h"
#include "vhost.h"
#include "mbuf.h"

static void *worker_fn(void *data)
{
	struct mbuf *pkts[1];
	int i;
	int np;

	vhost_log("worker start...\n");
	sleep(20);
	vhost_log("ctx %d\n", n_vhost_server);
	for (i = 0; i < n_vhost_server; i++) {
		if ((vhost_servers[i].fd > 0) && vhost_servers[i].dev) {
			vhost_log("ctx %d has dev %p\n", i, vhost_servers[i].dev);
			virtio_dump_dev(vhost_servers[i].dev);
		}
	}
	while (1) {
		for (i = 0; i < n_vhost_server; i++) {
			if ((vhost_servers[i].fd > 0) && vhost_servers[i].dev) {
				np = vhost_tx(vhost_servers[i].dev, 1, pkts, 1);
				if (np > 0) {
					vhost_dump_mbuf(pkts[0]);
					vhost_free_mbuf(pkts[0]);
				}
			}
		}
		usleep(100);
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

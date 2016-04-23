#include "uds.h"
#include <stdlib.h>
#include <errno.h>

struct uds_ctx * uds_listen(const char *path)
{
	int rc;
	int e;
	struct uds_ctx *ctx;

	ctx = malloc(sizeof(*ctx));
	if (!ctx) {
		return NULL;
	}
	ctx->fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (ctx->fd < 0) {
		goto err;
	}
	memset(&ctx->sa, 0, sizeof(struct sockaddr_un));
	ctx->sa.sun_family = AF_UNIX;
	sprintf(ctx->sa.sun_path, "%s", path);
	rc = bind(ctx->fd, (struct sockaddr *)&ctx->sa, sizeof(struct sockaddr_un));
	if (rc < 0) {
		goto err;
	}
	rc = listen(ctx->fd, 1024);
	if (rc < 0) {
		goto err;
	}
	return ctx;
err:
	e = errno;
	free(ctx);
	errno = e;
	return NULL;
}

struct uds_ctx * uds_dgram(const char *path)
{
	int rc;
	int e;
	struct uds_ctx *ctx;

	ctx = malloc(sizeof(*ctx));
	if (!ctx) {
		return NULL;
	}
	ctx->fd = socket(AF_UNIX, SOCK_DGRAM, 0);
	if (ctx->fd < 0) {
		goto err;
	}
	memset(&ctx->sa, 0, sizeof(struct sockaddr_un));
	ctx->sa.sun_family = AF_UNIX;
	sprintf(ctx->sa.sun_path, "%s", path);
	rc = bind(ctx->fd, (struct sockaddr *)&ctx->sa, sizeof(struct sockaddr_un));
	if (rc < 0) {
		goto err;
	}
	return ctx;
err:
	e = errno;
	free(ctx);
	errno = e;
	return NULL;
}

struct uds_ctx * uds_connect(const char *path)
{
	int rc;
	int e;
	struct uds_ctx *ctx;

	ctx = malloc(sizeof(*ctx));
	if (!ctx) {
		return NULL;
	}
	ctx->fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (ctx->fd < 0) {
		goto err;
	}
	memset(&ctx->sa, 0, sizeof(struct sockaddr_un));
	ctx->sa.sun_family = AF_UNIX;
	sprintf(ctx->sa.sun_path, "%s", path);
	rc = connect(ctx->fd, (struct sockaddr *)&ctx->sa, sizeof(struct sockaddr_un));
	if (rc < 0) {
		goto err;
	}
	return ctx;
err:
	e = errno;
	free(ctx);
	errno = e;
	return NULL;
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

int uds_send_dgram(struct uds_ctx *ctx, void *data, int len)
{
	int rc;
	struct msghdr hdr;
	struct iovec iov;

	memset(&hdr, 0, sizeof(hdr));

	iov.iov_base = data;
	iov.iov_len = len;
	hdr.msg_name = &ctx->sa;
	hdr.msg_namelen = sizeof(ctx->sa);
	hdr.msg_iov = &iov;
	hdr.msg_iovlen = 1;

	rc = sendmsg(ctx->fd, &hdr, 0);
	if (rc < 0) {
		return -1;
	}
	return 0;
}

int uds_recv_dgram(struct uds_ctx *ctx, void *data, int len)
{
	struct msghdr hdr;
	struct iovec iov;
	int rc;

	memset(&hdr, 0, sizeof(hdr));

	iov.iov_base = data;
	iov.iov_len = len;
	hdr.msg_name = &ctx->sa;
	hdr.msg_namelen = sizeof(ctx->sa);
	hdr.msg_iov = &iov;
	hdr.msg_iovlen = 1;

	rc = recvmsg(ctx->fd, &hdr, 0);
	if (rc < 0) {
		return -1;
	}

	return 0;
}

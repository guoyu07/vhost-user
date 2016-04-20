#include <string.h>

#include "vhost.h"
#include "mbuf.h"

#define MAX_PKT_BURST	(16)

#define min(x, y)	(((x) <= (y))?(x):(y))

int vhost_rx(struct virtio_dev *dev, int qidx, struct mbuf *pkts[], int npkts)
{
	return 0;
}

static int copy_desc_to_mbuf(struct virtio_dev *dev, struct virtqueue  *vq,
		struct mbuf *mbuf, u32 desc_idx)
{
	struct virtio_net_hdr *hdr;
	struct virtq_desc *desc;
	void *desc_addr;
	u32 desc_pkt_len;
	u32 hdrlen;

	hdrlen = sizeof(*hdr);
	desc = &vq->desc[desc_idx];

	vhost_log("desc %d len %d hdrlen %d\n", desc_idx, desc->len, hdrlen);

	desc_addr = gpa_to_va(dev, desc->addr);
	desc_pkt_len = desc->len - hdrlen;
	
	memcpy(&mbuf->hdr, desc_addr, hdrlen);
	if (desc_pkt_len > 0) {
		mbuf->len = desc_pkt_len;
		memcpy(mbuf->data, desc_addr + hdrlen, desc_pkt_len);
	} else if (desc_pkt_len == 0) {
		desc_idx = desc->next;
		desc = &vq->desc[desc->next];
		desc_addr = gpa_to_va(dev, desc->addr);
		desc_pkt_len = desc->len;
		vhost_log("desc-next %d len %d hdrlen %d\n", desc_idx, desc->len, hdrlen);
		if (desc_pkt_len > 0) {
			mbuf->len = desc_pkt_len;
			memcpy(mbuf->data, desc_addr, desc_pkt_len);
		}
	} else {
		vhost_log("bad pkt\n");
	}
	return 0;
}

int vhost_tx(struct virtio_dev *dev, int qidx, struct mbuf *pkts[], u16 npkts)
{
	int i;
	int err;
	u16 used_idx;
	u16 avail_idx;
	u16 navail;
	u32 desc_index[MAX_PKT_BURST];
	struct virtqueue *vq;

	if (!dev->mem)
		return 0;

	vq = &dev->vq[qidx];

	avail_idx = *((volatile u16 *)&vq->avail->idx);
	navail = avail_idx - vq->last_used_idx;
	if (navail == 0) {
		return 0;
	}

	npkts = min(npkts, MAX_PKT_BURST);
	npkts = min(npkts, navail);

	for (i = 0; i < npkts; i++) {
		desc_index[i] = vq->avail->ring[(vq->last_used_idx + i) & (vq->num - 1)];
	}

	for (i = 0; i < npkts; i++) {
		pkts[i] = vhost_new_mbuf();
		if (!pkts[i]) {
			vhost_log("no mbuf\n");
			break;
		}
		err = copy_desc_to_mbuf(dev, vq, pkts[i], desc_index[i]);
		if (err) {
			vhost_log("tx copy failed\n");
		}
		used_idx = vq->last_used_idx++ & (vq->num - 1);
		vq->used->ring[used_idx].id = desc_index[i];
		vq->used->ring[used_idx].len = 0;
	}

	vq->used->idx += i;

	return i;
}

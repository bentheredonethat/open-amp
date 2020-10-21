/*
 * Copyright (c) 2014, Mentor Graphics Corporation
 * All rights reserved.
 * Copyright (c) 2017 Xilinx, Inc.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**************************************************************************
 * FILE NAME
 *
 *       zynqmp_a53_r5_rproc.c
 *
 * DESCRIPTION
 *
 *       This file define Xilinx ZynqMP R5 to A53 platform specific 
 *       remoteproc implementation.
 *
 **************************************************************************/

#include <metal/atomic.h>
#include <metal/assert.h>
#include <metal/device.h>
#include <metal/irq.h>
#include <metal/utilities.h>
#include <openamp/rpmsg_virtio.h>
#include "platform_info.h"

static struct remoteproc * zynqmp_a53_r5_proc_init(struct remoteproc *rproc,
						   struct remoteproc_ops *ops,
						   void *arg)
{
	struct remoteproc_priv *prproc = arg;
	struct metal_device *kick_dev;
	int ret;

	if (!rproc || !prproc || !ops)
		return NULL;
	ret = metal_device_open(prproc->kick_bus_name,
				prproc->kick_dev_name,
				&kick_dev);
	if (ret) {
		xil_printf("failed to open polling device: %d.\r\n", ret);
		return NULL;
	}
	rproc->priv = prproc;
	prproc->kick_dev = kick_dev;
	prproc->kick_io = metal_device_io_region(kick_dev, 0);
	if (!prproc->kick_io)
		goto err1;
	metal_io_write32(prproc->kick_io, 0, !POLL_STOP);
	rproc->ops = ops;

	return rproc;
err1:
	metal_device_close(kick_dev);
	return NULL;
}

static void zynqmp_a53_r5_proc_remove(struct remoteproc *rproc)
{
	struct remoteproc_priv *prproc;

	if (!rproc)
		return;
	prproc = rproc->priv;

	metal_device_close(prproc->kick_dev);
}

static void *
zynqmp_a53_r5_proc_mmap(struct remoteproc *rproc, metal_phys_addr_t *pa,
			metal_phys_addr_t *da, size_t size,
			unsigned int attribute, struct metal_io_region **io)
{
	struct remoteproc_mem *mem;
	metal_phys_addr_t lpa, lda;
	struct metal_io_region *tmpio;

	lpa = *pa;
	lda = *da;

	if (lpa == METAL_BAD_PHYS && lda == METAL_BAD_PHYS)
		return NULL;
	if (lpa == METAL_BAD_PHYS)
		lpa = lda;
	if (lda == METAL_BAD_PHYS)
		lda = lpa;

	mem = metal_allocate_memory(sizeof(*mem));
	if (!mem)
		return NULL;
	tmpio = metal_allocate_memory(sizeof(*tmpio));
	if (!tmpio) {
		metal_free_memory(mem);
		return NULL;
	}
	remoteproc_init_mem(mem, NULL, lpa, lda, size, tmpio);
	/* va is the same as pa in this platform */
	metal_io_init(tmpio, (void *)lpa, &mem->pa, size,
		      sizeof(metal_phys_addr_t) << 3, attribute, NULL);
	remoteproc_add_mem(rproc, mem);
	*pa = lpa;
	*da = lda;
	if (io)
		*io = tmpio;
	return metal_io_phys_to_virt(tmpio, mem->pa);
}

static int zynqmp_a53_r5_proc_notify(struct remoteproc *rproc, uint32_t id)
{
	struct remoteproc_priv *prproc;

	(void)id;
	if (!rproc)
		return -1;
	prproc = rproc->priv;

	metal_io_write32(prproc->kick_io, 0, POLL_STOP);
	return 0;
}

/* processor operations from r5 to a53. It defines
 * notification operation and remote processor managementi operations. */
struct remoteproc_ops zynqmp_a53_r5_proc_ops = {
	.init = zynqmp_a53_r5_proc_init,
	.remove = zynqmp_a53_r5_proc_remove,
	.mmap = zynqmp_a53_r5_proc_mmap,
	.notify = zynqmp_a53_r5_proc_notify,
	.start = NULL,
	.stop = NULL,
	.shutdown = NULL,
};

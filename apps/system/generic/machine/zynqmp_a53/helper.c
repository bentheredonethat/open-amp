/*
 * Copyright (c) 2014, Mentor Graphics Corporation
 * All rights reserved.
 *
 * Copyright (c) 2015 Xilinx, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "xparameters.h"
#include "xil_exception.h"
#include "xil_printf.h"
#include "xil_cache.h"
#include <metal/sys.h>
#include <metal/irq.h>
#include "platform_info.h"


static void system_metal_logger(enum metal_log_level level,
			   const char *format, ...)
{
	(void)level;
	(void)format;
}


/* Main hw machinery initialization entry point, called from main()*/
/* return 0 on success */
int init_system(void)
{
	struct metal_init_params metal_param = {
		.log_handler = system_metal_logger,
		.log_level = METAL_LOG_INFO,
	};

	/* Low level abstraction layer for openamp initialization */
	metal_init(&metal_param);

	return 0;
}

void cleanup_system()
{
	metal_finish();

	Xil_DCacheDisable();
	Xil_ICacheDisable();
	Xil_DCacheInvalidate();
	Xil_ICacheInvalidate();
}

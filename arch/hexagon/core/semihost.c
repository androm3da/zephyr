/*
 * Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/arch/common/semihost.h>
#include <zephyr/arch/hexagon/hexagon_vm.h>

/* Semihosting message structure for HVM */
struct semihost_msg {
	unsigned int op;
	unsigned int arg;
	void *data;
};

long semihost_exec(enum semihost_instr instr, void *args)
{
	struct semihost_msg msg;
	long ret;

	/* Map semihosting instruction to HVM angel console operation */
	switch (instr) {
	case SEMIHOST_WRITEC:
		/* Write single character via HVM angel console */
		msg.op = ANGEL_CONSOLE_WRITE;
		msg.arg = 1;  /* length = 1 */
		msg.data = args;  /* args points to the character */
		ret = __vmgetinfo(VM_INFO_TYPE_ANGEL_CONSOLE, &msg);
		break;

	case SEMIHOST_WRITE0:
		/* Write null-terminated string via HVM angel console */
		if (args) {
			char *str = (char *)args;
			size_t len = 0;

			/* Calculate string length */
			while (str[len] != '\0') {
				len++;
			}

			msg.op = ANGEL_CONSOLE_WRITE;
			msg.arg = len;
			msg.data = str;
			ret = __vmgetinfo(VM_INFO_TYPE_ANGEL_CONSOLE, &msg);
		} else {
			ret = -1;
		}
		break;

	case SEMIHOST_WRITE:
		/* Write buffer via HVM angel console */
		if (args) {
			/* Args should point to semihost_write_args structure */
			struct semihost_write_args {
				long fd;
				const char *buf;
				long len;
			} *write_args = (struct semihost_write_args *)args;

			if (write_args->fd == 1 || write_args->fd == 2) { /* stdout/stderr */
				msg.op = ANGEL_CONSOLE_WRITE;
				msg.arg = write_args->len;
				msg.data = (void *)write_args->buf;
				ret = __vmgetinfo(VM_INFO_TYPE_ANGEL_CONSOLE, &msg);
			} else {
				ret = -1; /* File operations not supported */
			}
		} else {
			ret = -1;
		}
		break;

	case SEMIHOST_READC:
		/* Read character via HVM angel console (not implemented) */
		ret = -1;
		break;

	default:
		/* Fallback to original trap0 mechanism for other operations */
		{
			register unsigned long r0 __asm__("r0") = instr;
			register void *r1 __asm__("r1") = args;
			register long trap_ret __asm__("r0");

			__asm__ __volatile__(
				"r0 = %1\n\t"
				"r1 = %2\n\t"
				"trap0(#0)\n\t"
				"%0 = r0"
				: "=r"(trap_ret)
				: "r"(r0), "r"(r1)
				: "memory");
			ret = trap_ret;
		}
		break;
	}

	return ret;
}

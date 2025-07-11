/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved. */

#include <zephyr/kernel.h>
#include <zephyr/arch/hexagon/arch.h>
#include <zephyr/arch/hexagon/exception.h>
#include <zephyr/logging/log.h>
#include <zephyr/fatal.h>

LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

/* Forward declarations */
void arch_system_halt(unsigned int reason);

/* Exception cause codes */
#define HEXAGON_CAUSE_RESET          0x00
#define HEXAGON_CAUSE_PRECISE_BUS    0x01
#define HEXAGON_CAUSE_IMPRECISE_BUS  0x02
#define HEXAGON_CAUSE_ILLEGAL_INST   0x03
#define HEXAGON_CAUSE_PRIVILEGE      0x04
#define HEXAGON_CAUSE_UNALIGNED      0x05
#define HEXAGON_CAUSE_TRAP0          0x06
#define HEXAGON_CAUSE_FLOATING_POINT 0x07

/* Get current exception stack frame */
void z_hexagon_get_current_esf(struct arch_esf *esf)
{
	/* Clear the ESF structure */
	memset(esf, 0, sizeof(*esf));

	/* Get guest registers which contain exception info */
	__asm__ volatile("r0 = g0\n\t"
			 "r1 = g1\n\t"
			 "r2 = g2\n\t"
			 "r3 = g3\n\t"
			 "memw(%[event_info]+#0) = r0\n\t"
			 "memw(%[event_info]+#4) = r1\n\t"
			 "memw(%[event_info]+#8) = r2\n\t"
			 "memw(%[event_info]+#12) = r3"
			 :
			 : [event_info] "r"(&esf->event_info[0])
			 : "r0", "r1", "r2", "r3", "memory");
}

/* Fatal error handler */
void z_hexagon_fatal_error(unsigned int reason)
{
	struct arch_esf esf = {0};

	/* Save current context if possible */
	z_hexagon_get_current_esf(&esf);

	/* Call architecture-independent fatal error handler */
	z_fatal_error(reason, &esf);
}

/* Dump exception information */
void z_hexagon_exception_info(unsigned int cause, uint32_t addr)
{
	const char *cause_str = "Unknown";

	switch (cause) {
	case HEXAGON_CAUSE_RESET:
		cause_str = "Reset";
		break;
	case HEXAGON_CAUSE_PRECISE_BUS:
		cause_str = "Precise bus error";
		break;
	case HEXAGON_CAUSE_IMPRECISE_BUS:
		cause_str = "Imprecise bus error";
		break;
	case HEXAGON_CAUSE_ILLEGAL_INST:
		cause_str = "Illegal instruction";
		break;
	case HEXAGON_CAUSE_PRIVILEGE:
		cause_str = "Privilege violation";
		break;
	case HEXAGON_CAUSE_UNALIGNED:
		cause_str = "Unaligned access";
		break;
	case HEXAGON_CAUSE_TRAP0:
		cause_str = "Trap0 instruction";
		break;
	case HEXAGON_CAUSE_FLOATING_POINT:
		cause_str = "Floating point exception";
		break;
	}

	LOG_ERR("Exception: %s (0x%x) at 0x%08x", cause_str, cause, addr);
}

/* Implement arch_syscall_oops for system call errors */
FUNC_NORETURN void arch_syscall_oops(void *ssf)
{
	z_hexagon_fatal_error(K_ERR_KERNEL_OOPS);
	CODE_UNREACHABLE;
}

/* Implement z_do_kernel_oops for kernel panics */
FUNC_NORETURN void z_do_kernel_oops(const struct arch_esf *esf)
{
	LOG_ERR("Kernel OOPS at PC: 0x%08x", esf->pc);

	/* Print register state */
	LOG_ERR("Registers:");
	LOG_ERR("  g0=0x%08x g1=0x%08x g2=0x%08x g3=0x%08x", esf->event_info[0], esf->event_info[1],
		esf->event_info[2], esf->event_info[3]);
	LOG_ERR("  lr=0x%08x", esf->r31_lr);

	/* Halt system */
	arch_system_halt(0xFF);
	while (1) {
		arch_cpu_idle();
	}
}

/* Architecture-specific exception dump */
void z_arch_except(unsigned int reason)
{
	LOG_ERR("Architecture exception: reason %u", reason);

	/* Get current exception frame */
	struct arch_esf esf;
	z_hexagon_get_current_esf(&esf);

	/* Call generic exception handler */
	z_do_kernel_oops(&esf);
}

void arch_system_halt(unsigned int reason)
{
	/* Use Hexagon stop instruction to halt the system */
	__asm__ volatile("stop(r0)" : : "r"(reason) : "r0", "memory");

	/* Should never reach here, but include infinite loop as fallback */
	while (1) {
		arch_cpu_idle();
	}
}

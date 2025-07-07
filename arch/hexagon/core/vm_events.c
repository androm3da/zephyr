/*
 * Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Hexagon Virtual Machine event handlers for Zephyr
 */

#include <zephyr/kernel.h>
#include <zephyr/arch/cpu.h>
#include <zephyr/arch/hexagon/hexagon_vm.h>
#include <zephyr/arch/common/semihost.h>
#include <zephyr/irq.h>
#include <zephyr/sw_isr_table.h>
#include <zephyr/sys/printk.h>

/* Simplified pt_regs structure for Zephyr HVM events */
struct hvm_pt_regs {
	unsigned long r00, r01, r02, r03;
	unsigned long r04, r05, r06, r07;
	unsigned long r08, r09, r10, r11;
	unsigned long r12, r13, r14, r15;
	unsigned long r16, r17, r18, r19;
	unsigned long r20, r21, r22, r23;
	unsigned long r24, r25, r26, r27;
	unsigned long r28;
	unsigned long preds;
	unsigned long lc0, lc1;
	unsigned long sa0, sa1;
	unsigned long gp, ugp;
	unsigned long cs0, cs1;
	unsigned long elr;
	unsigned long cause;
	unsigned long badva;
};

extern struct _isr_table_entry _sw_isr_table[];

/*
 * Extract event information from pt_regs
 */
static inline unsigned long pt_elr(struct hvm_pt_regs *regs)
{
	return regs->elr;
}

static inline unsigned long pt_cause(struct hvm_pt_regs *regs)
{
	return regs->cause;
}

static inline unsigned long pt_badva(struct hvm_pt_regs *regs)
{
	return regs->badva;
}

static inline int user_mode(struct hvm_pt_regs *regs)
{
	/* In Zephyr, we're typically in kernel mode */
	return 0;
}

/*
 * Show register state for debugging - simplified version
 */
void z_hexagon_show_regs(struct hvm_pt_regs *regs)
{
	printk("HVM Event Register Dump:\n");
	printk("ELR: 0x%08lx  CAUSE: 0x%08lx  BADVA: 0x%08lx\n",
		pt_elr(regs), pt_cause(regs), pt_badva(regs));
	printk("R0-R3:   0x%08lx 0x%08lx 0x%08lx 0x%08lx\n",
		regs->r00, regs->r01, regs->r02, regs->r03);
	printk("R4-R7:   0x%08lx 0x%08lx 0x%08lx 0x%08lx\n",
		regs->r04, regs->r05, regs->r06, regs->r07);
	printk("R8-R11:  0x%08lx 0x%08lx 0x%08lx 0x%08lx\n",
		regs->r08, regs->r09, regs->r10, regs->r11);
	printk("GP: 0x%08lx  UGP: 0x%08lx  USR: 0x%08lx\n",
		regs->gp, regs->ugp, regs->sa0); /* USR stored in sa0 */
}

/*
 * HVM trap0 handler - handles semihosting and system calls
 */
void z_hexagon_do_trap0(struct hvm_pt_regs *regs)
{
	/* For Zephyr, trap0 is primarily used for semihosting */
	/* The actual semihosting implementation is in semihost.c */

	/* Call existing semihosting handler if enabled */
#ifdef CONFIG_SEMIHOST
	/* Trap0 instruction encoding contains the semihosting operation */
	/* R0 contains the operation, R1 contains the parameter */
	extern long semihost_exec(enum semihost_instr instr, void *args);

	/* Map register contents to semihosting call */
	enum semihost_instr instr = (enum semihost_instr)regs->r00;
	void *args = (void *)regs->r01;

	long result = semihost_exec(instr, args);

	/* Store result back in R0 */
	regs->r00 = result;
#else
	printk("HVM: Unhandled trap0 at ELR=0x%08lx\n", pt_elr(regs));
	z_hexagon_show_regs(regs);
#endif
}

/*
 * HVM machine check handler
 */
void z_hexagon_do_machcheck(struct hvm_pt_regs *regs)
{
	printk("HVM: Machine check exception at ELR=0x%08lx\n", pt_elr(regs));
	z_hexagon_show_regs(regs);

	/* In Zephyr, this is typically a fatal error */
	k_fatal_halt(K_ERR_CPU_EXCEPTION);
}

/*
 * HVM general exception handler
 */
void z_hexagon_do_genex(struct hvm_pt_regs *regs)
{
	printk("HVM: General exception at ELR=0x%08lx, CAUSE=0x%08lx\n",
		pt_elr(regs), pt_cause(regs));
	z_hexagon_show_regs(regs);

	/* In Zephyr, this is typically a fatal error */
	k_fatal_halt(K_ERR_CPU_EXCEPTION);
}

/*
 * HVM PMU read handler
 */
void z_hexagon_do_pmu_read(struct hvm_pt_regs *regs)
{
	printk("HVM: PMU read event at ELR=0x%08lx\n", pt_elr(regs));

	/* PMU operations not implemented in Zephyr yet */
	/* Just continue execution */
}

/*
 * HVM interrupt handler - main interrupt dispatch
 */
void z_hexagon_do_interrupt(struct hvm_pt_regs *regs)
{
	unsigned int irq;
	struct _isr_table_entry *entry;

	/* Extract interrupt number from cause register */
	irq = pt_cause(regs) & 0x1F; /* Lower 5 bits contain IRQ number */

	if (irq >= CONFIG_NUM_IRQS) {
		printk("HVM: Spurious interrupt %d\n", irq);
		return;
	}

	/* Call the registered interrupt handler */
	entry = &_sw_isr_table[irq];
	if (entry->isr) {
		sys_trace_isr_enter();
		entry->isr(entry->arg);
		sys_trace_isr_exit();
	} else {
		printk("HVM: No handler for interrupt %d\n", irq);
	}
}

/*
 * Initialize HVM event handling
 */
void z_hexagon_hvm_init(void)
{
	long hvm_version;

	/* Check if HVM is available */
	hvm_version = __vmversion();
	if (hvm_version <= 0) {
		printk("HVM: Virtual machine not available (version=%ld)\n", hvm_version);
		return;
	}

	printk("HVM: Hexagon Virtual Machine version %ld initialized\n", hvm_version);

	/* Initialize existing IRQ system */
	extern void z_hexagon_irq_init(void);
	z_hexagon_irq_init();
}

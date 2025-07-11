/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved. */

#include <zephyr/kernel.h>
#include <zephyr/irq.h>
#include <zephyr/sw_isr_table.h>
#include <errno.h>

/* Hexagon constants */
#define ARCH_IRQ_COUNT                  64
#define HEXAGON_IRQ_PRIORITY_LOWEST     63
#define HEXAGON_NO_PENDING_IRQ          0xFFFFFFFF
#define HEXAGON_EVENT_MACHINE_CHECK     1
#define HEXAGON_EVENT_GENERAL_EXCEPTION 2
#define HEXAGON_EVENT_INTERRUPT         7

/* Event context structure offsets matching assembly */
#define EVENT_CTX_R0_R1   0x00
#define EVENT_CTX_R2_R3   0x08
#define EVENT_CTX_R4_R5   0x10
#define EVENT_CTX_R6_R7   0x18
#define EVENT_CTX_R8_R9   0x20
#define EVENT_CTX_R10_R11 0x28
#define EVENT_CTX_R12_R13 0x30
#define EVENT_CTX_R14_R15 0x38
#define EVENT_CTX_PRED    0x40
#define EVENT_CTX_LR      0x44
#define EVENT_CTX_GUEST   0x48
#define EVENT_CTX_SIZE    0x58

/* Event context saved by assembly handlers */
struct event_context {
	uint32_t r0_r1[2];
	uint32_t r2_r3[2];
	uint32_t r4_r5[2];
	uint32_t r6_r7[2];
	uint32_t r8_r9[2];
	uint32_t r10_r11[2];
	uint32_t r12_r13[2];
	uint32_t r14_r15[2];
	uint32_t pred_regs;
	uint32_t link_reg;
	uint32_t guest_regs[4]; /* g0-g3 */
};

/* Forward declarations */
void z_hexagon_exception_handler(struct event_context *ctx);
void z_hexagon_interrupt_handler(struct event_context *ctx);
void z_hexagon_fatal_error(unsigned int reason);
uint32_t hexagon_intc_get_pending(void);
void hexagon_intc_ack(uint32_t irq);
void hexagon_intc_enable(uint32_t irq);
void hexagon_intc_disable(uint32_t irq);
void hexagon_intc_set_priority(uint32_t irq, uint32_t priority);
void hexagon_intc_init(void);

/* Main event handler called from assembly */
void z_hexagon_event_handler(unsigned int event_num, struct event_context *ctx)
{
	switch (event_num) {
	case HEXAGON_EVENT_MACHINE_CHECK:
		z_hexagon_fatal_error(K_ERR_CPU_EXCEPTION);
		break;

	case HEXAGON_EVENT_GENERAL_EXCEPTION:
		z_hexagon_exception_handler(ctx);
		break;

	case HEXAGON_EVENT_INTERRUPT:
		z_hexagon_interrupt_handler(ctx);
		break;

	default:
		z_hexagon_fatal_error(K_ERR_SPURIOUS_IRQ);
		break;
	}
}

/* Handle general exceptions */
void z_hexagon_exception_handler(struct event_context *ctx)
{
	/* Extract exception cause from guest registers */
	uint32_t cause = ctx->guest_regs[0]; /* g0 contains cause */
	uint32_t addr = ctx->guest_regs[1];  /* g1 contains fault address */

	printk("Exception: cause=0x%x, addr=0x%x\n", cause, addr);

	/* Fatal error for now */
	z_hexagon_fatal_error(K_ERR_CPU_EXCEPTION);
}

/* Handle interrupts */
void z_hexagon_interrupt_handler(struct event_context *ctx)
{
	uint32_t irq_num;

	/* Get interrupt number from interrupt controller */
	irq_num = hexagon_intc_get_pending();

	if (irq_num == HEXAGON_NO_PENDING_IRQ) {
		return; /* Spurious interrupt */
	}

	/* Call ISR from SW ISR table */
	const struct _isr_table_entry *entry = &_sw_isr_table[irq_num];
	entry->isr(entry->arg);

	/* Acknowledge interrupt */
	hexagon_intc_ack(irq_num);
}

/* Enable an IRQ */
void arch_irq_enable(unsigned int irq)
{
	if (irq >= ARCH_IRQ_COUNT) {
		return;
	}

	/* Use vmintop to enable specific interrupt */
	__asm__ volatile("r0 = #0\n\t" /* Op: enable */
			 "r1 = %0\n\t" /* IRQ number */
			 "trap1(#0x5)" /* vmintop */
			 :
			 : "r"(irq)
			 : "r0", "r1", "memory");
}

/* Disable an IRQ */
void arch_irq_disable(unsigned int irq)
{
	if (irq >= ARCH_IRQ_COUNT) {
		return;
	}

	/* Use vmintop to disable specific interrupt */
	__asm__ volatile("r0 = #1\n\t" /* Op: disable */
			 "r1 = %0\n\t" /* IRQ number */
			 "trap1(#0x5)" /* vmintop */
			 :
			 : "r"(irq)
			 : "r0", "r1", "memory");
}

/* Check if IRQ is enabled */
int arch_irq_is_enabled(unsigned int irq)
{
	uint32_t status;

	if (irq >= ARCH_IRQ_COUNT) {
		return 0;
	}

	/* Use vmintop to query interrupt state */
	__asm__ volatile("r0 = #2\n\t"     /* Op: query */
			 "r1 = %1\n\t"     /* IRQ number */
			 "trap1(#0x5)\n\t" /* vmintop */
			 "%0 = r0"
			 : "=r"(status)
			 : "r"(irq)
			 : "r0", "r1", "memory");

	return status & 1;
}

/* Forward declaration */
void hexagon_irq_priority_set(unsigned int irq, unsigned int priority);

/* Connect IRQ at runtime */
int arch_irq_connect_dynamic(unsigned int irq, unsigned int priority,
			     void (*routine)(const void *parameter), const void *parameter,
			     uint32_t flags)
{
	if (irq >= ARCH_IRQ_COUNT) {
		return -EINVAL;
	}

#ifdef CONFIG_DYNAMIC_INTERRUPTS
	/* Set up SW ISR table entry (only possible with dynamic interrupts) */
	_sw_isr_table[irq].isr = routine;
	_sw_isr_table[irq].arg = parameter;
#endif

	/* Set interrupt priority */
	hexagon_irq_priority_set(irq, priority);

	return 0;
}

/* Set interrupt priority */
void hexagon_irq_priority_set(unsigned int irq, unsigned int priority)
{
	if (irq >= ARCH_IRQ_COUNT || priority > HEXAGON_IRQ_PRIORITY_LOWEST) {
		return;
	}

	/* Use vmintop to set priority */
	__asm__ volatile("r0 = #3\n\t" /* Op: set priority */
			 "r1 = %0\n\t" /* IRQ number */
			 "r2 = %1\n\t" /* Priority */
			 "trap1(#0x5)" /* vmintop */
			 :
			 : "r"(irq), "r"(priority)
			 : "r0", "r1", "r2", "memory");
}

/* Initialize interrupt handling */
void z_hexagon_irq_init(void)
{
	/* Ensure interrupts are disabled during init */
	arch_irq_lock();

	/* Clear any pending interrupts */
	for (int i = 0; i < ARCH_IRQ_COUNT; i++) {
		hexagon_intc_disable(i);
	}

	/* Initialize interrupt controller */
	hexagon_intc_init();
}

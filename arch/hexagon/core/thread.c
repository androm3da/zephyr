#include <kernel.h>
#include <ksched.h>
#include <stdio.h>

void arch_new_thread(struct k_thread *thread, k_thread_stack_t *stack,
		     char *stack_ptr, k_thread_entry_t entry,
		     void *p1, void *p2, void *p3)
{
	extern void z_hexagon_thread_start(void);
	struct __esf *stack_init;

	/* Initial stack frame for thread */
	stack_init = (struct __esf *)Z_STACK_PTR_ALIGN(
				Z_STACK_PTR_TO_FRAME(struct __esf, stack_ptr)
				);

	/* Setup the initial stack frame */
	stack_init->r0 = (ulong_t)entry;
	stack_init->r1 = (ulong_t)p1;
	stack_init->r2 = (ulong_t)p2;
	stack_init->r3 = (ulong_t)p3;

#if defined(CONFIG_THREAD_LOCAL_STORAGE)
/* The UGP register is set to the highest-memory-address-plus-one of the
   process’s thread-local storage area. */
	stack_init->ugp = thread->tls;
	thread->callee_saved.ugp = thread->tls;
#endif

/* The R28 register is set to the address of a function which the process must call when it terminates.
The process is responsible for saving this address so it can be called later on. After saving the
address, the process can freely use the R28 register. */

	thread->callee_saved.r29 = (ulong_t)stack_init;

	/* where to go when returning from z_hexagon_switch() */
	thread->callee_saved.r31 = (ulong_t)z_hexagon_thread_start;

	/* our switch handle is the thread pointer itself */
	thread->switch_handle = thread;
}

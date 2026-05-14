/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#define WORKER_STACKSIZE 1024
#define WORKER_PRIORITY 5

K_MUTEX_DEFINE(my_mutex);
K_SEM_DEFINE(my_sem, 0, 1);

static void worker_entry(void *p1, void *p2, void *p3)
{
	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	printk("worker: locking mutex\n");
	k_mutex_lock(&my_mutex, K_FOREVER);
	printk("worker: mutex locked, signaling main\n");
	k_sem_give(&my_sem);
	/* Sleep with mutex held so GDB can inspect it */
	k_sleep(K_SECONDS(60));
	k_mutex_unlock(&my_mutex);
}

K_THREAD_STACK_DEFINE(worker_stack, WORKER_STACKSIZE);
static struct k_thread worker_thread;

void inspection_point(void)
{
	printk("%s: ready for GDB\n", __func__);
}

int main(void)
{
	printk("%s: starting GDB plugin test\n", __func__);

	k_thread_create(&worker_thread, worker_stack,
			K_THREAD_STACK_SIZEOF(worker_stack),
			worker_entry, NULL, NULL, NULL,
			WORKER_PRIORITY, 0, K_NO_WAIT);
	k_thread_name_set(&worker_thread, "worker");

	/* Wait until worker has locked the mutex */
	k_sem_take(&my_sem, K_FOREVER);
	printk("%s: worker has mutex, inspecting\n", __func__);

	inspection_point();

	printk("%s: test complete\n", __func__);
	return 0;
}

/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Hexagon MMU management via the H2 hypervisor page table interface.
 *
 * The Hexagon Virtual Machine (H2) provides a 2-level page table walker.
 * Level 1 is the Page Global Directory (PGD) with 1024 entries, each
 * covering 4MB of virtual address space.  Level 2 page table entries
 * (PTEs) cover individual pages.  With 64KB pages (the default), each
 * L2 table has 64 entries (256 bytes).
 *
 * At boot, Zephyr runs under H2's linear (identity) translation mode.
 * hexagon_mmu_init() builds page tables that identity-map all known
 * memory regions, then switches to table-based translation via the
 * vmnewmap hypercall.  After that, arch_mem_map() can create new
 * mappings dynamically.
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/__assert.h>
#include <zephyr/logging/log.h>
#include <hexagon_vm.h>

LOG_MODULE_REGISTER(hexagon_mmu, CONFIG_KERNEL_LOG_LEVEL);

/* Page table geometry for 64KB pages */
#define HEX_PAGE_SHIFT       16
#define HEX_PAGE_SIZE        (1U << HEX_PAGE_SHIFT)
#define HEX_PGDIR_SHIFT      22
#define HEX_PGDIR_SIZE       (1U << HEX_PGDIR_SHIFT)  /* 4MB per PGD entry */
#define HEX_PTRS_PER_PGD     1024
#define HEX_PTRS_PER_PTE     64   /* 4MB / 64KB */
#define HEX_L2_TABLE_SIZE    (HEX_PTRS_PER_PTE * sizeof(uint32_t))  /* 256 bytes */

/* PDE: physical address of L2 table (masked) | page size encoding */
#define HEX_PDE_PAGE_SIZE    __HVM_PDE_S_64KB
#define HEX_PDE_PTMASK       0xffffff00U

/* PTE: physical page frame | cache attrs (bits 6-8) | perms (bits 9-11) */
#define HEX_PTE_PGMASK       0xffff0000U
#define HEX_PTE_CACHE_SHIFT  6

/* Number of L2 page tables to pre-allocate.
 * Each covers 4MB.  We need enough for all identity-mapped regions plus
 * runtime mappings.  With 4GB address space / 4MB = 1024 possible entries,
 * but we only need tables for regions that are actually mapped.
 * Allocate enough for: SRAM (16MB=4), RAM (32MB=8), device IO (256MB=64),
 * plus headroom for runtime MMIO mappings.
 */
#define HEX_MAX_L2_TABLES    128

/* Page Global Directory - must be 4KB aligned (1024 * 4 bytes) */
static uint32_t pgd[HEX_PTRS_PER_PGD] __aligned(4096);

/* Pool of L2 page tables - each is 256 bytes, must be 256-byte aligned */
static uint32_t l2_tables[HEX_MAX_L2_TABLES][HEX_PTRS_PER_PTE] __aligned(256);
static uint32_t l2_table_next;

/**
 * Allocate an L2 page table from the static pool.
 * Returns the physical address of the table (identity-mapped at boot).
 */
static uint32_t *l2_table_alloc(void)
{
	__ASSERT(l2_table_next < HEX_MAX_L2_TABLES,
		 "L2 page table pool exhausted (%u/%u)",
		 l2_table_next, HEX_MAX_L2_TABLES);

	return l2_tables[l2_table_next++];
}

/**
 * Get or allocate the L2 page table for a given virtual address.
 */
static uint32_t *get_l2_table(uintptr_t virt)
{
	uint32_t pgd_idx = virt >> HEX_PGDIR_SHIFT;
	uint32_t pde = pgd[pgd_idx];
	uint32_t *l2;

	if (pde == 0) {
		/* No L2 table yet - allocate one */
		l2 = l2_table_alloc();
		memset(l2, 0, HEX_L2_TABLE_SIZE);
		pgd[pgd_idx] = ((uint32_t)(uintptr_t)l2 & HEX_PDE_PTMASK) |
			       HEX_PDE_PAGE_SIZE;
	} else {
		l2 = (uint32_t *)(uintptr_t)(pde & HEX_PDE_PTMASK);
	}

	return l2;
}

/**
 * Compute the PTE index within an L2 table for a virtual address.
 */
static inline uint32_t pte_index(uintptr_t virt)
{
	return (virt >> HEX_PAGE_SHIFT) & (HEX_PTRS_PER_PTE - 1);
}

/**
 * Build a PTE value from physical address, cache attributes, and permissions.
 */
static inline uint32_t make_pte(uintptr_t phys, uint32_t cache_attr, uint32_t perms)
{
	return (phys & HEX_PTE_PGMASK) |
	       (cache_attr << HEX_PTE_CACHE_SHIFT) |
	       perms;
}

/**
 * Convert Zephyr K_MEM_CACHE_* flags to Hexagon cache attribute value.
 */
static uint32_t flags_to_cache_attr(uint32_t flags)
{
	switch (flags & K_MEM_CACHE_MASK) {
	case K_MEM_CACHE_WB:
		return __HEXAGON_C_WB_L2;
	case K_MEM_CACHE_WT:
		return __HEXAGON_C_WT_L2;
	case K_MEM_CACHE_NONE:
		return __HEXAGON_C_DEV;
	default:
		return __HEXAGON_C_WB_L2;
	}
}

/**
 * Convert Zephyr K_MEM_PERM_* flags to Hexagon PTE permission bits.
 */
static uint32_t flags_to_perms(uint32_t flags)
{
	uint32_t perms = __HVM_PTE_R; /* Always readable */

	if (flags & K_MEM_PERM_RW) {
		perms |= __HVM_PTE_W;
	}
	if (flags & K_MEM_PERM_EXEC) {
		perms |= __HVM_PTE_X;
	}
	if (flags & K_MEM_PERM_USER) {
		perms |= __HVM_PTE_U;
	}

	return perms;
}

/**
 * Map a contiguous physical range into the page tables.
 * Used both at boot (identity map) and at runtime (arch_mem_map).
 */
static void map_range(uintptr_t virt, uintptr_t phys, size_t size,
		      uint32_t cache_attr, uint32_t perms)
{
	uintptr_t va = virt;
	uintptr_t pa = phys;
	uintptr_t end = virt + size;

	while (va < end) {
		uint32_t *l2 = get_l2_table(va);
		uint32_t idx = pte_index(va);

		l2[idx] = make_pte(pa, cache_attr, perms);
		va += HEX_PAGE_SIZE;
		pa += HEX_PAGE_SIZE;
	}
}

/**
 * Initialize the MMU page tables and switch from linear to table-based
 * translation.  Called once during early boot before z_cstart().
 *
 * Identity-maps all memory regions that Zephyr needs:
 * - SRAM at 0xa0000000 (16MB) - code/data, write-back cached
 * - RAM at 0x80000000 (32MB) - heap/stacks, write-back cached
 * - Device MMIO at 0x10000000 (256MB region covering UART and VirtIO)
 */
void hexagon_mmu_init(void)
{
	int32_t ret;
	uint32_t rwx = __HVM_PTE_R | __HVM_PTE_W | __HVM_PTE_X;

	memset(pgd, 0, sizeof(pgd));
	l2_table_next = 0;

	/* Identity-map SRAM: 0xa0000000, 16MB, cached WB+L2, RWX */
	map_range(0xa0000000, 0xa0000000, 16 * 1024 * 1024,
		  __HEXAGON_C_WB_L2, rwx);

	/* Identity-map RAM: 0x80000000, 32MB, cached WB+L2, RWX */
	map_range(0x80000000, 0x80000000, 32 * 1024 * 1024,
		  __HEXAGON_C_WB_L2, rwx);

	/* Identity-map device MMIO: 0x10000000, 256MB, device/uncached, RW
	 * This covers the PL011 UART (0x10000000) and VirtIO (0x11000000+)
	 */
	map_range(0x10000000, 0x10000000, 256 * 1024 * 1024,
		  __HEXAGON_C_DEV, __HVM_PTE_R | __HVM_PTE_W);

	/* Flush data cache to ensure page tables are visible to the walker */
	hexagon_vm_cache(hvmc_dccleaninva, 0, 0);

	/* Switch to table-based translation */
	ret = hexagon_vm_newmap(pgd, VM_TRANS_TYPE_TABLE,
				VM_TLB_INVALIDATE_TRUE);
	__ASSERT(ret == 0, "vmnewmap failed: %d", ret);
}

#ifdef CONFIG_MMU

void arch_mem_map(void *virt, uintptr_t phys, size_t size, uint32_t flags)
{
	uintptr_t va = (uintptr_t)virt;
	uint32_t cache_attr = flags_to_cache_attr(flags);
	uint32_t perms = flags_to_perms(flags);

	__ASSERT((va & (HEX_PAGE_SIZE - 1)) == 0,
		 "unaligned virtual address %p", virt);
	__ASSERT((phys & (HEX_PAGE_SIZE - 1)) == 0,
		 "unaligned physical address 0x%lx", phys);
	__ASSERT((size & (HEX_PAGE_SIZE - 1)) == 0,
		 "unaligned size 0x%zx", size);

	map_range(va, phys, size, cache_attr, perms);

	/* Flush cache and invalidate TLB for the mapped range */
	hexagon_vm_cache(hvmc_dccleaninva, 0, 0);
	hexagon_vm_clrmap((void *)va, (uint32_t)size);
}

void arch_mem_unmap(void *addr, size_t size)
{
	uintptr_t va = (uintptr_t)addr;
	uintptr_t end = va + size;

	__ASSERT((va & (HEX_PAGE_SIZE - 1)) == 0,
		 "unaligned virtual address %p", addr);
	__ASSERT((size & (HEX_PAGE_SIZE - 1)) == 0,
		 "unaligned size 0x%zx", size);

	while (va < end) {
		uint32_t pgd_idx = va >> HEX_PGDIR_SHIFT;
		uint32_t pde = pgd[pgd_idx];

		if (pde != 0) {
			uint32_t *l2 = (uint32_t *)(uintptr_t)(pde & HEX_PDE_PTMASK);
			uint32_t idx = pte_index(va);

			l2[idx] = 0;
		}
		va += HEX_PAGE_SIZE;
	}

	/* Flush cache and invalidate TLB for the unmapped range */
	hexagon_vm_cache(hvmc_dccleaninva, 0, 0);
	hexagon_vm_clrmap(addr, (uint32_t)size);
}

int arch_page_phys_get(void *virt, uintptr_t *phys)
{
	uintptr_t va = (uintptr_t)virt;
	uint32_t pgd_idx = va >> HEX_PGDIR_SHIFT;
	uint32_t pde = pgd[pgd_idx];

	if (pde == 0) {
		return -EFAULT;
	}

	uint32_t *l2 = (uint32_t *)(uintptr_t)(pde & HEX_PDE_PTMASK);
	uint32_t idx = pte_index(va);
	uint32_t pte_val = l2[idx];

	if (pte_val == 0) {
		return -EFAULT;
	}

	if (phys != NULL) {
		*phys = (pte_val & HEX_PTE_PGMASK) |
			(va & (HEX_PAGE_SIZE - 1));
	}

	return 0;
}

#endif /* CONFIG_MMU */

/* Cache operations using HVM vmcache hypercall */

void arch_dcache_flush_all(void)
{
	hexagon_vm_cache(hvmc_dccleaninva, 0, 0);
}

void arch_dcache_invd_all(void)
{
	hexagon_vm_cache(hvmc_dckill, 0, 0);
}

void arch_dcache_flush_and_invd_all(void)
{
	hexagon_vm_cache(hvmc_dccleaninva, 0, 0);
}

#ifndef ASM_HEXAGON_VM_H
#define ASM_HEXAGON_VM_H

#define HEXAGON_VM_vmversion   0
#define HEXAGON_VM_vmrte       1
#define HEXAGON_VM_vmsetvec    2
#define HEXAGON_VM_vmsetie     3
#define HEXAGON_VM_vmgetie     4
#define HEXAGON_VM_vmintop     5
#define HEXAGON_VM_vmclrmap    10
#define HEXAGON_VM_vmnewmap    11
#define HEXAGON_VM_vmcache     13
#define HEXAGON_VM_vmgettime   14
#define HEXAGON_VM_vmsettime   15
#define HEXAGON_VM_vmwait      16
#define HEXAGON_VM_vmyield     17
#define HEXAGON_VM_vmstart     18
#define HEXAGON_VM_vmstop      19
#define HEXAGON_VM_vmvpid      20
#define HEXAGON_VM_vmsetregs   21
#define HEXAGON_VM_vmgetregs   22
#define HEXAGON_VM_vmtimerop   24
#define HEXAGON_VM_vmpmuconfig 25
#define HEXAGON_VM_vmgetinfo   26

/* Special trap0 numbers */
#define HEXAGON_VM_hwconfig    31

#define VM_CLOBBERLIST_ABIV2                                                                       \
	"r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12", "r13", "r14",   \
		"r15", "r28", "r31", "sa0", "lc0", "sa1", "lc1", "m0", "m1", "usr", "p0", "p1",    \
		"p2", "p3"

#define VM_CLOBBERLIST VM_CLOBBERLIST_ABIV2

#ifndef __ASSEMBLER__
#include <stdint.h>

enum VM_CACHE_OPS {
	hvmc_ickill,
	hvmc_dckill,
	hvmc_l2kill,
	hvmc_dccleaninva,
	hvmc_icinva,
	hvmc_idsync,
	hvmc_fetch_cfg
};

enum VM_INT_OPS {
	hvmi_nop,
	hvmi_globen,
	hvmi_globdis,
	hvmi_locen,
	hvmi_locdis,
	hvmi_affinity,
	hvmi_get,
	hvmi_peek,
	hvmi_status,
	hvmi_post,
	hvmi_clear
};

enum VM_TIMER_OPS {
	getfreq,
	getres,
	gettime,
	gettimeout,
	settimeout,
	deltatimeout
};

enum VM_STOP_STATUS {
	none,
	poweroff,
	halt,
	restart,
	machinecheck
};

enum VM_INFO_TYPE {
	vm_info_build_id,
	vm_info_boot_flags,
	vm_info_stlb,
	vm_info_syscfg,
	vm_info_livelock,
	vm_info_rev,
	vm_info_ssbase,
	vm_info_tlb_free,
	vm_info_tlb_size,
	vm_info_physaddr,
	vm_info_tcm_base,
	vm_info_l2mem_size,
	vm_info_tcm_size,
	vm_info_h2k_pgsize,
	vm_info_h2k_npages,
	vm_info_l2vic_base,
	vm_info_timer_base,
	vm_info_timer_int,
	vm_info_error,
	vm_info_hthreads,
	vm_info_l2tag_size,
	vm_info_l2cfg_base,
	vm_info_clade_base,
	vm_info_cfgbase,
	vm_info_hvx_vlength,
	vm_info_hvx_contexts,
	vm_info_hvx_switch,
	vm_info_max
};

typedef enum {
	HWCONFIG_L2CACHE,
	HWCONFIG_PARTITIONS,
	HWCONFIG_PREFETCH,
	HWCONFIG_EXTBITS,
	HWCONFIG_VLENGTH,
	HWCONFIG_EXTPOWER,
	HWCONFIG_GETL2REG,
	HWCONFIG_SETL2REG,
	HWCONFIG_L2LOCKA,
	HWCONFIG_L2UNLOCK,
	HWCONFIG_HWINTOP,
	HWCONFIG_RESERVED_00,
	HWCONFIG_RESERVED_01,
	HWCONFIG_HWTHREADS_NUM,
	HWCONFIG_HWTHREADS_MASK,
	HWCONFIG_RESERVED_02,
	HWCONFIG_RESERVED_03,
	HWCONFIG_MAX
} hwconfig_type_t;

struct vm_syscfg {
	union {
		uint32_t raw;
		struct {
			uint32_t m: 1;
			uint32_t i: 1;
			uint32_t d: 1;
			uint32_t t: 1;
			uint32_t g: 1;
			uint32_t r: 1;
			uint32_t c: 1;

			uint32_t unused4: 1;

			uint32_t ida: 1;
			uint32_t pm: 1;

			uint32_t te: 1;
			uint32_t tl: 1;
			uint32_t kl: 1;

			uint32_t bq: 1;

			uint32_t unused3: 1;

			uint32_t dmt: 1;

			uint32_t l2cfg: 3;

			uint32_t itcm: 1;

			uint32_t unused2: 1;

			uint32_t l2nwa: 1;
			uint32_t l2nra: 1;

			uint32_t l2wb: 1;
			uint32_t l2p: 1;

			uint32_t l1dp: 2;
			uint32_t l1ip: 2;
			uint32_t l2part: 2;
			uint32_t unused: 1;
		};
	};
};

struct vm_rev {
	union {
		uint32_t raw;
		struct {
			uint32_t isa: 8;
			uint32_t l2tags: 4;
			uint32_t l2size: 4;
			uint32_t layer: 4;
			uint32_t unused: 4;
			uint32_t metal: 8;
		};
	};
};

struct vm_boot_flags {
	union {
		uint32_t raw;
		struct {
			uint32_t use_tcm: 1;
			uint32_t unused: 31;
		};
	};
};

struct vm_stlb_info {
	union {
		uint32_t raw;
		struct {
			uint32_t max_sets_log2: 8;
			uint32_t max_ways: 8;
			uint32_t size: 8;
			uint32_t unused: 7;
			uint32_t enabled: 1;
		};
	};
};

/* VM functions in alphabetical order */

static inline int32_t hexagon_vm_cache(enum VM_CACHE_OPS op, uint32_t addr, uint32_t len)
{
	int32_t result;
	__asm__ volatile(
		"r0 = %[op]\n\t"
		"r1 = %[addr]\n\t"
		"r2 = %[len]\n\t"
		"trap1(#%[trap_id])\n\t"
		"%0 = r0\n\t"
		: "=r"(result)
		: [op] "r"(op), [addr] "r"(addr), [len] "r"(len), [trap_id] "i"(HEXAGON_VM_vmcache)
		: VM_CLOBBERLIST);
	return result;
}

static inline int32_t hexagon_vm_clrmap(void *addr, uint32_t len)
{
	int32_t result;
	__asm__ volatile("r0 = %[addr]\n\t"
			 "r1 = %[len]\n\t"
			 "trap1(#%[trap_id])\n\t"
			 "%0 = r0\n\t"
			 : "=r"(result)
			 : [addr] "r"(addr), [len] "r"(len), [trap_id] "i"(HEXAGON_VM_vmclrmap)
			 : VM_CLOBBERLIST);
	return result;
}

static inline uint32_t hexagon_vm_getie(void)
{
	uint32_t result;
	__asm__ volatile("trap1(#%[trap_id]);"
			 : "=r"(result)
			 : [trap_id] "i"(HEXAGON_VM_vmgetie)
			 : VM_CLOBBERLIST);
	return result;
}

static inline uint32_t hexagon_vm_getinfo(uint32_t info_type)
{
	uint32_t result;

	__asm__ volatile("r0 = %[info_type]\n\t"
			 "trap1(#%[trap_id])\n\t"
			 "%0 = r0\n\t"
			 : "=r"(result)
			 : [info_type] "r"(info_type), [trap_id] "i"(HEXAGON_VM_vmgetinfo)
			 : VM_CLOBBERLIST);
	return result;
}

static inline int32_t hexagon_vm_getregs(void *ptr)
{
	int32_t result;
	__asm__ volatile("r0 = %[ptr]\n\t"
			 "trap1(#%[trap_id])\n\t"
			 "%0 = r0\n\t"
			 : "=r"(result)
			 : [ptr] "r"(ptr), [trap_id] "i"(HEXAGON_VM_vmgetregs)
			 : VM_CLOBBERLIST);
	return result;
}

static inline uint64_t hexagon_vm_gettime(void)
{
	uint64_t result;
	__asm__ volatile("trap1(#%[trap_id])\n\t"
			 "%0 = r1:0\n\t"
			 : "=r"(result)
			 : [trap_id] "i"(HEXAGON_VM_vmgettime)
			 : VM_CLOBBERLIST);
	return result;
}

static inline int32_t hexagon_vm_hwconfig(uint32_t arg0, uint32_t arg1, uint32_t arg2,
					  uint32_t arg3)
{
	int32_t result;
	__asm__ volatile("r0 = %[arg0]\n\t"
			 "r1 = %[arg1]\n\t"
			 "r2 = %[arg2]\n\t"
			 "r3 = %[arg3]\n\t"
			 "trap0(#%[trap_id])\n\t"
			 "%0 = r0\n\t"
			 : "=r"(result)
			 : [arg0] "r"(arg0), [arg1] "r"(arg1), [arg2] "r"(arg2),
			   [arg3] "r"(arg3),
			   [trap_id] "i"(HEXAGON_VM_hwconfig)
			 : VM_CLOBBERLIST);
	return result;
}

static inline int32_t hexagon_vm_intop(enum VM_INT_OPS op, int32_t arg1, int32_t arg2, int32_t arg3,
				       int32_t arg4)
{
	int32_t result;
	__asm__ volatile("r0 = %[op]\n\t"
			 "r1 = %[arg1]\n\t"
			 "r2 = %[arg2]\n\t"
			 "r3 = %[arg3]\n\t"
			 "r4 = %[arg4]\n\t"
			 "trap1(#%[trap_id])\n\t"
			 "%0 = r0\n\t"
			 : "=r"(result)
			 : [op] "r"(op), [arg1] "r"(arg1), [arg2] "r"(arg2), [arg3] "r"(arg3),
			   [arg4] "r"(arg4), [trap_id] "i"(HEXAGON_VM_vmintop)
			 : VM_CLOBBERLIST);
	return result;
}

static inline int32_t hexagon_vm_newmap(void *addr, uint32_t type, uint32_t tlb_flush_flag)
{
	int32_t result;
	__asm__ volatile("r0 = %[addr]\n\t"
			 "r1 = %[type]\n\t"
			 "r2 = %[tlb_flush_flag]\n\t"
			 "trap1(#%[trap_id])\n\t"
			 "%0 = r0\n\t"
			 : "=r"(result)
			 : [addr] "r"(addr), [type] "r"(type), [tlb_flush_flag] "r"(tlb_flush_flag),
			   [trap_id] "i"(HEXAGON_VM_vmnewmap)
			 : VM_CLOBBERLIST);
	return result;
}

static inline uint32_t hexagon_vm_pmucfg(uint32_t operation, uint32_t r1, int32_t r2, uint32_t r3)
{
	uint32_t result;
	__asm__ volatile("r0 = %[operation]\n\t"
			 "r1 = %[r1]\n\t"
			 "r2 = %[r2]\n\t"
			 "r3 = %[r3]\n\t"
			 "trap1(#%[trap_id])\n\t"
			 "%0 = r0\n\t"
			 : "=r"(result)
			 : [operation] "r"(operation), [r1] "r"(r1), [r2] "r"(r2), [r3] "r"(r3),
			   [trap_id] "i"(HEXAGON_VM_vmpmuconfig)
			 : VM_CLOBBERLIST);
	return result;
}

static inline void hexagon_vm_rte(void)
{
	__asm__ volatile("trap1(#%[trap_id]);" : : [trap_id] "i"(HEXAGON_VM_vmrte) : VM_CLOBBERLIST);
}

static inline int32_t hexagon_vm_setie(int32_t enable)
{
	int32_t result;
	__asm__ volatile("r0 = %[enable]\n\t"
			 "trap1(#%[trap_id])\n\t"
			 "%0 = r0\n\t"
			 : "=r"(result)
			 : [enable] "r"(enable), [trap_id] "i"(HEXAGON_VM_vmsetie)
			 : VM_CLOBBERLIST);
	return result;
}

static inline int32_t hexagon_vm_setie_atomic(int32_t new_ie, volatile int32_t *cached_ie)
{
	int32_t old_ie;

	/* Simple implementation for now - can be optimized later with full assembly version */
	old_ie = hexagon_vm_getie();
	if (old_ie != new_ie) {
		hexagon_vm_setie(new_ie);
		if (cached_ie) {
			*cached_ie = new_ie;
		}
	}
	return old_ie;
}

static inline int32_t hexagon_vm_setregs(void *ptr)
{
	int32_t result;
	__asm__ volatile("r0 = %[ptr]\n\t"
			 "trap1(#%[trap_id])\n\t"
			 "%0 = r0\n\t"
			 : "=r"(result)
			 : [ptr] "r"(ptr), [trap_id] "i"(HEXAGON_VM_vmsetregs)
			 : VM_CLOBBERLIST);
	return result;
}

static inline int32_t hexagon_vm_settime(uint64_t time)
{
	int32_t result;
	__asm__ volatile("r1:0 = %[time]\n\t"
			 "trap1(#%[trap_id])\n\t"
			 "%0 = r0\n\t"
			 : "=r"(result)
			 : [time] "r"(time), [trap_id] "i"(HEXAGON_VM_vmsettime)
			 : VM_CLOBBERLIST);
	return result;
}

static inline int32_t hexagon_vm_setvec(void *ptr)
{
	int32_t result;
	__asm__ volatile("r0 = %[ptr]\n\t"
			 "trap1(#%[trap_id])\n\t"
			 "%0 = r0\n\t"
			 : "=r"(result)
			 : [ptr] "r"(ptr), [trap_id] "i"(HEXAGON_VM_vmsetvec)
			 : VM_CLOBBERLIST);
	return result;
}

static inline void hexagon_vm_stop(enum VM_STOP_STATUS status)
{
	__asm__ volatile("r0 = %[status]\n\t"
			 "trap1(#%[trap_id])\n\t"
			 :
			 : [status] "r"(status), [trap_id] "i"(HEXAGON_VM_vmstop)
			 : VM_CLOBBERLIST);
}

static inline uint64_t hexagon_vm_timerop(enum VM_TIMER_OPS op, uint32_t dummy, uint64_t timeout)
{
	uint64_t result;
	__asm__ volatile("r0 = %[op]\n\t"
			 "r1 = %[dummy]\n\t"
			 "r3:2 = %[timeout]\n\t"
			 "trap1(#%[trap_id])\n\t"
			 "%0 = r1:0\n\t"
			 : "=r"(result)
			 : [op] "r"(op), [dummy] "r"(dummy), [timeout] "r"(timeout),
			   [trap_id] "i"(HEXAGON_VM_vmtimerop)
			 : VM_CLOBBERLIST);
	return result;
}

static inline uint32_t hexagon_vm_version(void)
{
	uint32_t result;
	__asm__ volatile("trap1(#%[trap_id]);"
			 : "=r"(result)
			 : [trap_id] "i"(HEXAGON_VM_vmversion)
			 : VM_CLOBBERLIST);
	return result;
}

static inline int32_t hexagon_vm_vpid(void)
{
	int32_t result;
	__asm__ volatile("trap1(#%[trap_id]);"
			 : "=r"(result)
			 : [trap_id] "i"(HEXAGON_VM_vmvpid)
			 : VM_CLOBBERLIST);
	return result;
}

static inline int32_t hexagon_vm_wait(void)
{
	int32_t result;

	__asm__ volatile("trap1(#%[trap_id]);"
			 : "=r"(result)
			 : [trap_id] "i"(HEXAGON_VM_vmwait)
			 : VM_CLOBBERLIST);
	return result;
}

static inline void hexagon_vm_yield(void)
{
	__asm__ volatile("trap1(#%[trap_id]);"
			 :
			 : [trap_id] "i"(HEXAGON_VM_vmyield)
			 : VM_CLOBBERLIST);
}

static inline uint32_t hexagon_vm_start_thread(int32_t hw_thread_id, void *entry_point,
					       void *stack_ptr)
{
	uint32_t new_proc_num;

	__asm__ volatile(
		"r0 = %[entry_point]\n\t"
		"r1 = %[stack_ptr]\n\t"
		"trap1(#%[trap_id])\n\t"
		"%0 = r0\n\t"
			 : "=r"(new_proc_num)
			 : [hw_thread_id] "r"(hw_thread_id), [entry_point] "r"(entry_point),
			   [stack_ptr] "r"(stack_ptr), [trap_id] "i"(HEXAGON_VM_vmstart)
			 : VM_CLOBBERLIST);
	return new_proc_num;
}

static inline int32_t hexagon_vm_stop_thread(int32_t hw_thread_id)
{
	int32_t result;

	__asm__ volatile("r0 = %[hw_thread_id]\n\t"
			 "trap1(#%[trap_id])\n\t"
			 "%0 = r0\n\t"
			 : "=r"(result)
			 : [hw_thread_id] "r"(hw_thread_id), [trap_id] "i"(HEXAGON_VM_vmstop)
			 : VM_CLOBBERLIST);
	return result;
}

/* Complex VM operations that require special handling */
extern void load_ie_cache(void);
extern int32_t vmgetie_cached(void);
extern int32_t vmsetie_cached(int32_t enable);
extern int32_t hexagon_vm_setie_cached(int32_t enable, int32_t *prev);
extern void clear_ie_cached(void);

/* Cache operation wrappers - alphabetically ordered */
static inline int32_t hexagon_vm_cache_dccleaninva(uint32_t addr, uint32_t len)
{
	return hexagon_vm_cache(hvmc_dccleaninva, addr, len);
}

static inline int32_t hexagon_vm_cache_dckill(void)
{
	return hexagon_vm_cache(hvmc_dckill, 0, 0);
}

static inline int32_t hexagon_vm_cache_fetch_cfg(uint32_t val)
{
	return hexagon_vm_cache(hvmc_fetch_cfg, val, 0);
}

static inline int32_t hexagon_vm_cache_icinva(uint32_t addr, uint32_t len)
{
	return hexagon_vm_cache(hvmc_icinva, addr, len);
}

static inline int32_t hexagon_vm_cache_ickill(void)
{
	return hexagon_vm_cache(hvmc_ickill, 0, 0);
}

static inline int32_t hexagon_vm_cache_idsync(uint32_t addr, uint32_t len)
{
	return hexagon_vm_cache(hvmc_idsync, addr, len);
}

static inline int32_t hexagon_vm_cache_l2kill(void)
{
	return hexagon_vm_cache(hvmc_l2kill, 0, 0);
}

/* Interrupt operation wrappers - alphabetically ordered */
static inline int32_t hexagon_vm_intop_affinity(int32_t i, int32_t cpu)
{
	return hexagon_vm_intop(hvmi_affinity, i, cpu, 0, 0);
}

static inline int32_t hexagon_vm_intop_clear(int32_t i)
{
	return hexagon_vm_intop(hvmi_clear, i, 0, 0, 0);
}

static inline int32_t hexagon_vm_intop_get(void)
{
	return hexagon_vm_intop(hvmi_get, 0, 0, 0, 0);
}

static inline int32_t hexagon_vm_intop_globdis(int32_t i)
{
	return hexagon_vm_intop(hvmi_globdis, i, 0, 0, 0);
}

static inline int32_t hexagon_vm_intop_globen(int32_t i)
{
	return hexagon_vm_intop(hvmi_globen, i, 0, 0, 0);
}

static inline int32_t hexagon_vm_intop_locdis(int32_t i)
{
	return hexagon_vm_intop(hvmi_locdis, i, 0, 0, 0);
}

static inline int32_t hexagon_vm_intop_locen(int32_t i)
{
	return hexagon_vm_intop(hvmi_locen, i, 0, 0, 0);
}

static inline int32_t hexagon_vm_intop_nop(void)
{
	return hexagon_vm_intop(hvmi_nop, 0, 0, 0, 0);
}

static inline int32_t hexagon_vm_intop_peek(void)
{
	return hexagon_vm_intop(hvmi_peek, 0, 0, 0, 0);
}

static inline int32_t hexagon_vm_intop_post(int32_t i, int32_t cpu)
{
	return hexagon_vm_intop(hvmi_post, i, cpu, 0, 0);
}

static inline int32_t hexagon_vm_intop_status(int32_t i)
{
	return hexagon_vm_intop(hvmi_status, i, 0, 0, 0);
}
#endif /* __ASSEMBLER__ */

/*
 * Constants for virtual instruction parameters and return values
 */

/* vmnewmap arguments */

#define VM_TRANS_TYPE_LINEAR    0
#define VM_TRANS_TYPE_TABLE     1
#define VM_TLB_INVALIDATE_FALSE 0
#define VM_TLB_INVALIDATE_TRUE  1

/* vmsetie arguments */

#define VM_INT_DISABLE 0
#define VM_INT_ENABLE  1

/* vmsetimask arguments */

#define VM_INT_UNMASK 0
#define VM_INT_MASK   1

#define VM_NEWMAP_TYPE_LINEAR   0
#define VM_NEWMAP_TYPE_PGTABLES 1

/*
 * Event Record definitions useful to both C and Assembler
 */

/* VMEST Layout */

#define HVM_VMEST_UM_SFT       31
#define HVM_VMEST_UM_MSK       1
#define HVM_VMEST_IE_SFT       30
#define HVM_VMEST_IE_MSK       1
#define HVM_VMEST_SS_SFT       29
#define HVM_VMEST_SS_MSK       1
#define HVM_VMEST_EVENTNUM_SFT 16
#define HVM_VMEST_EVENTNUM_MSK 0xff
#define HVM_VMEST_CAUSE_SFT    0
#define HVM_VMEST_CAUSE_MSK    0xffff

/*
 * The initial program gets to find a system environment descriptor
 * on its stack when it begins exection. The first word is a version
 * code to indicate what is there.  Zero means nothing more.
 */

#define HEXAGON_VM_SED_NULL 0

/*
 * Event numbers for vector binding
 */

#define HVM_EV_RESET     0
#define HVM_EV_MACHCHECK 1
#define HVM_EV_GENEX     2
#define HVM_EV_TRAP      8
#define HVM_EV_INTR      15
/* These shoud be nuked as soon as we know the VM is up to spec v0.1.1 */
#define HVM_EV_INTR_0    16
#define HVM_MAX_INTR     240

/*
 * Cause values for General Exception
 */

#define HVM_GE_C_BUS    0x01
#define HVM_GE_C_XPROT  0x11
#define HVM_GE_C_XUSER  0x12
#define HVM_GE_C_INVI   0x15
#define HVM_GE_C_COPROC 0x16
#define HVM_GE_C_PRIVGI 0x1A
#define HVM_GE_C_PRIVI  0x1B
#define HVM_GE_C_XMAL   0x1C
#define HVM_GE_C_WREG   0x1D
#define HVM_GE_C_PCAL   0x1E
#define HVM_GE_C_RMAL   0x20
#define HVM_GE_C_WMAL   0x21
#define HVM_GE_C_RPROT  0x22
#define HVM_GE_C_WPROT  0x23
#define HVM_GE_C_RUSER  0x24
#define HVM_GE_C_WUSER  0x25
#define HVM_GE_C_VMEM   0x26
#define HVM_GE_C_STACK  0x27
#define HVM_GE_C_CACHE  0x28

/* Extended TLB miss cause codes; earlier ones might be deprecated */
#define HVM_GE_C_TLBMISSX_0      0x60
#define HVM_GE_C_TLBMISSX_1      0x61
#define HVM_GE_C_TLBMISSX_ICINVA 0x62
#define HVM_GE_C_TLBMISSR        0x70
#define HVM_GE_C_TLBMISSW        0x71

/*
 * Cause codes for Machine Check
 */

#define HVM_MCHK_C_DOWN  0x00
#define HVM_MCHK_C_BADSP 0x01
#define HVM_MCHK_C_BADEX 0x02
#define HVM_MCHK_C_BADPT 0x03
#define HVM_MCHK_C_REGWR 0x29

#endif

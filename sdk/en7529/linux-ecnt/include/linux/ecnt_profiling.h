#ifndef __ECNT_PROFILING_H
#define __ECNT_PROFILING_H

#ifdef TCSUPPORT_CPU_ARMV8
#ifdef TCSUPPORT_CPU_ARMV8_64

#define PMCR_EL0_EN         (1<<0)

/* PMCR_EL0[15:11] will decide how many pmu counters each CPU supports.
 * Currectly, it's 6, so each CPU can use pmcnt 0~5 */
#define PMCNT0            (1<<0)
#define PMCNT1            (1<<1)
#define PMCNT2            (1<<2)
#define PMCNT3            (1<<3)
#define PMCNT4            (1<<4)
#define PMCNT5            (1<<4)
#define PMCNTC            (1<<31)


#define TYPE_INST_RETIRED    0x08
#define TYPE_CPU_CYCLES      0x11
#define TYPE_INST_SPEC       0x1B

static inline void __attribute__((always_inline))
pmu_set_pmcr_el0(unsigned long r)
{
	__asm__ __volatile__("MSR PMCR_EL0, %0" :: "r"(r));
}

static inline unsigned long __attribute__((always_inline))
pmu_get_pmcr_el0(void)
{
	unsigned long r;

	__asm__ __volatile__("MRS %0, PMCR_EL0" : "=r"(r));

	return r;
}

static inline void __attribute__((always_inline))
pmu_set_pmcnten_el0(unsigned long r)
{
	__asm__ __volatile__("MSR PMCNTENSET_EL0, %0" :: "r"(r));
}

static inline unsigned long __attribute__((always_inline))
pmu_get_pmcnten_el0(void)
{
	unsigned long r;

	__asm__ __volatile__("MRS %0, PMCNTENSET_EL0" : "=r"(r));

	return r;
}

static inline void __attribute__((always_inline))
pmu_set_pmselr(unsigned long r)
{
	__asm__ __volatile__("MSR PMSELR_EL0, %0" :: "r"(r));
}

static inline unsigned long __attribute__((always_inline))
pmu_get_pmselr(void)
{
	unsigned long r;

	__asm__ __volatile__("MRS %0, PMSELR_EL0" : "=r"(r));

	return r;
}

/* backward compatible with mips code */
#define read_c0_count pmu_get_cycleCnt

static inline unsigned long __attribute__((always_inline))
pmu_get_cycleCnt(void)
{
	unsigned long c;

	__asm__ __volatile__("MRS %0, PMCCNTR_EL0" : "=r"(c));

	return c;
}

static inline void __attribute__((always_inline))
pmu_reset_event_counter(unsigned long type)
{
	pmu_set_pmselr(type);

	__asm__ __volatile__("MSR PMXEVCNTR_EL0, %0" :: "r"(0));
}

static inline unsigned long __attribute__((always_inline))
pmu_get_event_counter(unsigned long type)
{
	unsigned long c = 0;

	pmu_set_pmselr(type);

	__asm__ __volatile__("MRS %0, PMXEVCNTR_EL0" : "=r"(c));

	return c;
}

static inline unsigned long __attribute__((always_inline))
pmu_get_pmxevtyper(void)
{
	unsigned long r;

	__asm__ __volatile__("MRS %0, PMXEVTYPER_EL0" : "=r"(r));

	return r;
}

static inline void __attribute__((always_inline))
pmu_set_pmxevtyper(unsigned long r)
{
	__asm__ __volatile__("MSR PMXEVTYPER_EL0, %0" :: "r"(r));
}

#define enablePMU() pmu_set_pmcr_el0((pmu_get_pmcr_el0() | PMCR_EL0_EN));
#define disablePMU() pmu_set_pmcr_el0((pmu_get_pmcr_el0() & ~(PMCR_EL0_EN)));

#define enableCCNT() pmu_set_pmcnten_el0((pmu_get_pmcnten_el0() | PMCNTC));	
#define disableCCNT() pmu_set_pmcnten_el0((pmu_get_pmcnten_el0() & ~(PMCNTC)));

#else
#include <asm/tc3162/arm_v7_pmu.h>
#endif

#else /* ! TCSUPPORT_CPU_ARMV8 */

#define _measure_read_base_by_addr(_cnt1,_cnt2,_addr)				\
({								\
	unsigned int _tmp;					\
	__asm__ __volatile__(   \
		 "mfc0    %0, $9, 0\n\t"    \
		 "la      %2, %3\n\t"    \
		 "addiu   %2, 1\n\t"    \
		 "mfc0    %1, $9, 0\n\t"    \
		 : "=r" (_cnt1), "=r" (_cnt2), "=r" (_tmp)  \
		 : "i" (_addr) \
	 ); \
})

#define _measure_read_by_addr(_cnt1,_cnt2,_addr)				\
({								\
	unsigned int _tmp;					\
	__asm__ __volatile__(   \
		 "mfc0    %0, $9, 0\n\t"    \
		 "la      %2, %3\n\t"    \
		 "lw      %2, 0(%2)\n\t"   \
		 "addiu   %2, 1\n\t"    \
		 "mfc0    %1, $9, 0\n\t"    \
		 : "=r" (_cnt1), "=r" (_cnt2), "=r" (_tmp)  \
		 : "i" (_addr) \
	); \
})


#define _measure_write_base_by_addr(_cnt1,_cnt2,_addr)				\
({								\
	unsigned int _tmp;					\
	__asm__ __volatile__(   \
		 "mfc0    %0, $9, 0\n\t"    \
		 "la      %2, %3\n\t"    \
		 "mfc0    %1, $9, 0\n\t"    \
		 : "=r" (_cnt1), "=r" (_cnt2), "=r" (_tmp)  \
		 : "i" (_addr) \
	); \
})

#define _measure_write_by_addr(_cnt1,_cnt2,_addr)				\
({								\
	unsigned int _tmp;					\
	__asm__ __volatile__(   \
		 "mfc0    %0, $9, 0\n\t"    \
		 "la      %2, %3\n\t"    \
		 "sw      %2, 0(%2)\n\t"   \
		 "mfc0    %1, $9, 0\n\t"    \
		 : "=r" (_cnt1), "=r" (_cnt2), "=r" (_tmp)  \
		 : "i" (_addr) \
	); \
})

#define _measure_base(_cnt1,_cnt2)				\
({								\
	__asm__ __volatile__(   \
		"mfc0    %0, $9, 0\n\t"    \
		"mfc0    %1, $9, 0\n\t"    \
		 : "=r" (_cnt1), "=r" (_cnt2)  \
	); \
})

#define _measure_sync(_cnt1,_cnt2)				\
({								\
	__asm__ __volatile__(   \
		"sync\n\t"    \
		"mfc0    %0, $9, 0\n\t"    \
		"sync\n\t"    \
		"mfc0    %1, $9, 0\n\t"    \
		: "=r" (_cnt1), "=r" (_cnt2)  \
	); \
})

#define _Dcache_inv_by_addr(_addr)				\
({								\
	__asm__ __volatile__(   \
		 "cache   0x11, %0\n\t"    \
		 "sync\n\t"    \
		 :              \
		 : "i" (_addr)  \
	); \
})

#define _L2cache_inv_by_addr(_addr)				\
({                              \
	__asm__ __volatile__(   \
		"cache   0x13, %0\n\t"    \
		"sync\n\t"    \
		:               \
		: "i" (_addr)  \
	); \
})

#define _Dcache_wback_inv_by_addr(_addr)				\
({                              \
	__asm__ __volatile__(   \
		"cache   0x15, %0\n\t"    \
		"sync\n\t"    \
		:               \
		: "i" (_addr)  \
	); \
})

#define _measure_Dcache_inv_by_addr(_cnt1,_cnt2,_addr)				\
({                              \
	__asm__ __volatile__(   \
		"sync\n\t"    \
		"mfc0    %0, $9, 0\n\t"    \
		"cache   0x11, %2\n\t"    \
		"sync\n\t"    \
		"mfc0    %1, $9, 0\n\t"    \
		: "=r" (_cnt1), "=r" (_cnt2)  \
		: "i" (_addr)  \
	); \
})

#define _measure_L2cache_inv_by_addr(_cnt1,_cnt2,_addr)				\
({                              \
	__asm__ __volatile__(   \
		"sync\n\t"    \
		"mfc0    %0, $9, 0\n\t"    \
		"cache   0x13, %2\n\t"    \
		"sync\n\t"    \
		"mfc0    %1, $9, 0\n\t"    \
		: "=r" (_cnt1), "=r" (_cnt2)  \
		: "i" (_addr)  \
	); \
})

#define _measure_Dcache_wback_inv_by_addr(_cnt1,_cnt2,_addr)				\
({                              \
	__asm__ __volatile__(   \
		"sync\n\t"    \
		"mfc0    %0, $9, 0\n\t"    \
		"cache   0x15, %2\n\t"    \
		"sync\n\t"    \
		"mfc0    %1, $9, 0\n\t"    \
		: "=r" (_cnt1), "=r" (_cnt2)  \
		: "i" (_addr)  \
	); \
})

#define _measure_L2cache_wback_inv_by_addr(_cnt1,_cnt2,_addr)				\
({                              \
	__asm__ __volatile__(   \
		"sync\n\t"    \
		"mfc0    %0, $9, 0\n\t"    \
		"cache   0x17, %2\n\t"    \
		"sync\n\t"    \
		"mfc0    %1, $9, 0\n\t"    \
		: "=r" (_cnt1), "=r" (_cnt2)  \
		: "i" (_addr)  \
	); \
})
#endif /* TCSUPPORT_CPU_ARMV8 */

#endif


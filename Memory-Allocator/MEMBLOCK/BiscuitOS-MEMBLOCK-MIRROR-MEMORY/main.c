/*
 * MEMBLOCK Memory Allocator: Memory Mirror on MEMBLOCK
 *
 * - must support 'kernelcore=mirror; on CMDLINE
 *
 * (C) 2022.10.16 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/memblock.h>

/* BiscuitOS NUMA Layout
 *  - Layout0: 2CPUs with 2 NUMA NODE
 *    NODE0: 0x00000000 - 0x100000000
 *    NODE1: 0x10000000 - 0x200000000
 *
 *  - Layout1: 4CPUs with 3 NUMA NODE
 *    NODE0: 0x00000000 - 0x200000000
 *    NODE1: 0x20000000 - 0x300000000
 *    NODE2: 0x30000000 - 0x400000000
 */
#define MEMBLOCK_MIRROR_BASE	0x22000000
#define MEMBLOCK_MIRROR_SIZE	0x10000
#define MEMBLOCK_MIRROR_END	(MEMBLOCK_MIRROR_BASE + MEMBLOCK_MIRROR_SIZE)
#define MEMBLOCK_FAKE_SIZE	0x10
#define MEMBLOCK_FAKE_NODE	1

int __init BiscuitOS_Running(void)
{
	phys_addr_t phys, start, end;
	void *mem;
	u64 idx;
	int nid;

	/* Mark Memory Mirrorable */
	memblock_mark_mirror(MEMBLOCK_MIRROR_BASE, MEMBLOCK_MIRROR_SIZE);

	/* Iterate Mirror memory on memblock.memory */
	printk("=== Iterate mirror on memblock.memory ===\n");
	__for_each_mem_range(idx, &memblock.memory, NULL, MEMBLOCK_FAKE_NODE,
					MEMBLOCK_MIRROR, &start, &end, &nid)
		printk("Mirror-Range %lld: %#llx - %#llx NID %d\n",
							idx, start, end, nid);

	/* Alloc Memory */
	phys = memblock_alloc_range_nid(MEMBLOCK_FAKE_SIZE,
				SMP_CACHE_BYTES, MEMBLOCK_MIRROR_BASE,
				MEMBLOCK_MIRROR_END, MEMBLOCK_FAKE_NODE, true);
	if (!phys) {
		printk("FATAL ERROR: No Free Memory on MEMBLOCK.\n");
		return -ENOMEM;
	}

	/* Iterate Mirror memory on memblock.reserved */
	printk("=== Iterate mirror on memblock.reserved ===\n");
	__for_each_mem_range(idx, &memblock.reserved, NULL, MEMBLOCK_FAKE_NODE,
					MEMBLOCK_MIRROR, &start, &end, &nid)
		printk("Mirror-Range %lld: %#llx - %#llx NID %d\n",
							idx, start, end, nid);

	mem = phys_to_virt(phys);
	sprintf(mem, "Hello %s", "BiscuitOS");
	printk("==== %s ====\n", (char *)mem);
	printk("Physical Address %#lx\n", __pa(mem));

	/* Free Memory */
	memblock_free(mem, MEMBLOCK_FAKE_SIZE);

	return 0;
}

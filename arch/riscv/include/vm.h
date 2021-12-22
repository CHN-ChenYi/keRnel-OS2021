#pragma once

#include "defs.h"

extern unsigned long swapper_pg_dir[512];

/**
 * @brief create virtual memory mapping for the given physical address
 *
 * @param pgtbl root page table
 * @param va virtual address (aligned to 4KiB)
 * @param pa physical address (aligned to 4KiB)
 * @param sz size of the mapping
 * @param perm permission (UXWR)
 */
void create_mapping(uint64 *pgtbl, uint64 va, uint64 pa, uint64 sz, int perm);

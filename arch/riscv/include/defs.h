#ifndef _DEFS_H
#define _DEFS_H

#include "types.h"

/**
 * @brief read from csr
 *
 * @param csr the csr
 */
#define csr_read(csr)                                        \
  ({                                                         \
    register uint64 __v;                                     \
    asm volatile("csrr %0, " #csr : "=r"(__v) : : "memory"); \
    __v;                                                     \
  })

#define csr_write(csr, val)                                    \
  ({                                                           \
    uint64 __v = (uint64)(val);                                \
    asm volatile("csrw " #csr ", %0" : : "r"(__v) : "memory"); \
  })

#define PHY_START 0x0000000080000000
#define PHY_SIZE 128 * 1024 * 1024  // 128MB， QEMU default mem size
#define PHY_END (PHY_START + PHY_SIZE)

#define PGSIZE 0x1000  // 4KB
#define PGROUNDUP(addr) ((addr + PGSIZE - 1) & (~(PGSIZE - 1)))
#define PGROUNDDOWN(addr) (addr & (~(PGSIZE - 1)))

#endif

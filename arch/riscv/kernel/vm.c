#include "defs.h"
#include "mm.h"
#include "printk.h"
#include "string.h"

const unsigned long PA2VA_OFFSET = (VM_START - PHY_START);

unsigned long early_pgtbl[512] __attribute__((__aligned__(0x1000)));

/**
 * @brief set up virtual memory for memory initialization
 *
 */
void setup_vm() {
  // use 1 GiB gigapages
  for (uint64 i = PHY_START; i < PHY_END; i += PGSIZE) {
    early_pgtbl[((i + PA2VA_OFFSET) >> 30) & 0x1ff] =
        (((i >> 30) & 0x3ffffff) << 28) | 0b1111;
  }
  // early_pgtbl[(PHY_START >> 30) & 0x1ff] =
  //     (((PHY_START >> 30) & 0x3ffffff) << 28) | 0b1111;
  log(LOG_LEVEL_OK, "First-time virtual memory mapping done\n");
}

/**
 * @brief create virtual memory mapping for the given physical address
 *
 * @param pgtbl root page table
 * @param va virtual address (aligned to 4KiB)
 * @param pa physical address (aligned to 4KiB)
 * @param sz size of the mapping
 * @param perm permission (XWR)
 */
void create_mapping(uint64 *pgtbl, uint64 va, uint64 pa, uint64 sz, int perm) {
  for (uint64 i = 0, v = va, p = pa; i < sz;
       i += PGSIZE, v += PGSIZE, p += PGSIZE) {
    const unsigned vpn0 = (v >> 12) & 0x1ff;
    const unsigned vpn1 = (v >> 21) & 0x1ff;
    const unsigned vpn2 = (v >> 30) & 0x1ff;

    uint64 *pte2 = pgtbl + vpn2;
    uint64 *pgtbl1;
    if (!(*pte2 & 1)) {
      pgtbl1 = (uint64 *)kalloc();
      *pte2 = ((((uint64)pgtbl1 - PA2VA_OFFSET) >> 12) << 10) | 1;
    } else {
      pgtbl1 = (uint64 *)(((*pte2 >> 10) << 12) + PA2VA_OFFSET);
    }
    uint64 *pte1 = pgtbl1 + vpn1;
    uint64 *pgtbl0;
    if (!(*pte1 & 1)) {
      pgtbl0 = (uint64 *)kalloc();
      *pte1 = ((((uint64)pgtbl0 - PA2VA_OFFSET) >> 12) << 10) | 1;
    } else {
      pgtbl0 = (uint64 *)(((*pte1 >> 10) << 12) + PA2VA_OFFSET);
    }
    pgtbl0[vpn0] = ((p >> 12) << 10) | (perm << 1) | 1;
  }
}

// root page table
unsigned long swapper_pg_dir[512] __attribute__((__aligned__(0x1000)));
extern void _stext(), _srodata(), _sdata(), _ebss(), _skernel();

/**
 * @brief set up the final virtual memory mapping
 *
 */
void setup_vm_final() {
  memset(swapper_pg_dir, 0x0, PGSIZE);

  // No OpenSBI mapping required

  // mapping kernel text X|-|R|V
  create_mapping(swapper_pg_dir, (uint64)_stext, PHY_START + OPENSBI_SIZE,
                 _srodata - _skernel, 0b101);
  log(LOG_LEVEL_OK, ".text   virtual memory mapped, permission set to R-X\n");

  // mapping kernel rodata -|-|R|V
  create_mapping(swapper_pg_dir, (uint64)_srodata,
                 PHY_START + OPENSBI_SIZE + _srodata - _skernel,
                 _sdata - _srodata, 0b001);
  log(LOG_LEVEL_OK, ".rodata virtual memory mapped, permission set to R--\n");

  // mapping other memory (kernel data, kernel bss and others) -|W|R|V
  create_mapping(swapper_pg_dir, (uint64)_sdata,
                 PHY_START + OPENSBI_SIZE + _sdata - _skernel,
                 PHY_END - (PHY_START + OPENSBI_SIZE + _sdata - _skernel),
                 0b011);
  log(LOG_LEVEL_OK, ".data   virtual memory mapped, permission set to RW-\n");
  log(LOG_LEVEL_OK, ".bss    virtual memory mapped, permission set to RW-\n");
  log(LOG_LEVEL_OK, "other   virtual memory mapped, permission set to RW-\n");

  // set satp with swapper_pg_dir
  const uint64 root_pgtbl_ppn = ((uint64)swapper_pg_dir - PA2VA_OFFSET) >> 12;
  uint64 new_satp = csr_read(satp);
  new_satp = (new_satp >> 44) << 44;
  new_satp |= root_pgtbl_ppn;
  csr_write(satp, new_satp);

  // flush TLB
  asm volatile("sfence.vma zero, zero");

  log(LOG_LEVEL_OK, "TLB flushed, second-time virtual memory mapping done\n");
  return;
}

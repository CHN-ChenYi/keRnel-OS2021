#include "defs.h"
#include "printk.h"
#include "sbi.h"

extern void schedule();

extern void test();

extern void _srodata();

int start_kernel() {
  log(LOG_LEVEL_INFO, "Kernel starts\n");

  // test rodata section permission
  // *(uint64 *)_srodata = 0;

  schedule();

  test();  // DO NOT DELETE !!!

  return 0;
}

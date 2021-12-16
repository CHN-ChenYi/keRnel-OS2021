#include "defs.h"
#include "printk.h"
#include "sbi.h"

extern void test();

extern void _srodata();

int start_kernel() {
  // test rodata section permission
  // *(uint64 *)_srodata = 0;

  test();  // DO NOT DELETE !!!

  return 0;
}

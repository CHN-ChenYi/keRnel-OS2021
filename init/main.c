#include "defs.h"
#include "printk.h"
#include "sbi.h"

extern void test();

int start_kernel() {
  printk("kernel is running!\n");

  test();  // DO NOT DELETE !!!

  return 0;
}

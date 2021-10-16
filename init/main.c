#include "print.h"
#include "sbi.h"

#include "defs.h"

extern void test();

int start_kernel() {
  puti(2021);
  puts(" Hello RISC-V 3190104500\n");

  uint64 csr = csr_read(sepc);
  puti(csr);

  test();  // DO NOT DELETE !!!

  return 0;
}

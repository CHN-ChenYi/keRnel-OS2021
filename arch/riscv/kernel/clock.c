#include "sbi.h"

const unsigned long TIMECLOCK = 10000000;  // clock frequency in QEMU is 10MHz
const unsigned long SBI_TIMER_EXT_ID = 0x54494D45;
const unsigned long SBI_TIMER_FUNC_ID = 0;

unsigned long get_cycles() {
  unsigned long ret;
  __asm__ volatile("rdtime %[ret]\n" : [ret] "=r"(ret) : :);
  return ret;
}

void clock_set_next_event() {
  unsigned long next = get_cycles() + TIMECLOCK;
  sbi_ecall(SBI_TIMER_EXT_ID, SBI_TIMER_FUNC_ID, next, 0, 0, 0, 0,
            0);  // set mtimecmp
}

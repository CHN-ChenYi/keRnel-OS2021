#include "clock.h"
#include "defs.h"
#include "printk.h"
#include "proc.h"

const unsigned long INTERRUPT_MASK = 1ll << 63;

/**
 * @brief print interrupt info and set next timer interrupt
 *
 */
void trap_handler(unsigned long scause, unsigned long sepc) {
  if (INTERRUPT_MASK & scause) {         // interrupt
    if (scause ^ INTERRUPT_MASK == 5) {  // Supervisor timer interrupt
      printk("[S] Supervisor Mode Timer Interrupt\n");
      clock_set_next_event();
      do_timer();
    }
  }
}

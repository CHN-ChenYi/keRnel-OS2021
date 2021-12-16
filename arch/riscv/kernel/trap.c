#include "clock.h"
#include "defs.h"
#include "printk.h"
#include "proc.h"

const unsigned long INTERRUPT_MASK = 1ll << 63;

/**
 * @brief print interrupt info and set next timer interrupt
 *
 * @return sepc dalta
 */
uint64 trap_handler(uint64 scause, uint64 sepc) {
  if (INTERRUPT_MASK & scause) {           // interrupt
    if ((scause ^ INTERRUPT_MASK) == 5) {  // Supervisor timer interrupt
      log(LOG_LEVEL_INFO, "[" ANSI_COLOR_YELLOW "S" ANSI_COLOR_RESET
                          "] Supervisor Mode Timer Interrupt\n");
      clock_set_next_event();
      do_timer();
    } else {
      log(LOG_LEVEL_WARN, "Unknown interrupt: scause[%lld], sepc[0x%llx]\n",
          scause, sepc);
    }
    return 0;
  } else {
    log(LOG_LEVEL_ERROR, "Unknown exception: scause[%lld], sepc[0x%llx]\n",
        scause, sepc);
    return 4;
  }
}

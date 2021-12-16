#include "clock.h"
#include "defs.h"
#include "printk.h"

void test() {
  for (;;) {
    static unsigned long last_print_time = 0;
    unsigned long time = get_cycles();
    if ((time - last_print_time) > TIMECLOCK || time < last_print_time) {
      log(LOG_LEVEL_INFO, "kernel is running!\n");
      last_print_time = time;
    }
  }
}

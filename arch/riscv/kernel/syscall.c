#include "syscall.h"

#include "printk.h"
#include "proc.h"

#define a0 (regs->x[10])
#define a1 (regs->x[11])
#define a2 (regs->x[12])
#define a7 (regs->x[17])

void syscall(pt_regs *regs) {
  switch (a7) {
    case SYS_WRITE:
      if (a0 == 1) {
        ((char *)a1)[a2] = '\0';
        a0 = printk((char *)a1);
      } else {
        log(LOG_LEVEL_ERROR, "Unknown sys_write fd: a0[%lld]\n", a0);
      }
      break;
    case SYS_GETPID:
      a0 = current->pid;
      break;
    default:
      log(LOG_LEVEL_ERROR, "Unknown syscall: a7[%lld]\n", a7);
      break;
  }
}

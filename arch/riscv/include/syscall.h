#pragma once

#include "defs.h"
#include "syscall_def.h"

typedef struct pt_regs {
  uint64 x[31];  // x1 ... x31
  uint64 sepc;
  uint64 sstatus;
} pt_regs;

void syscall(pt_regs *regs);

extern "C" {

#include "proc.h"

#include "defs.h"
#include "mm.h"
#include "printk.h"
#include "rand.h"
#include "string.h"
#include "vm.h"

extern void __switch_to(struct task_struct* prev, struct task_struct* next);
extern void __dummy();

#define MAX_COUNTER 10

const unsigned long kRAOffsetInThreadStruct =
    reinterpret_cast<unsigned long>(&(static_cast<thread_struct*>(0)->ra));
const unsigned long kSPOffsetInThreadStruct =
    reinterpret_cast<unsigned long>(&(static_cast<thread_struct*>(0)->sp));
const unsigned long kSOffsetInThreadStruct =
    reinterpret_cast<unsigned long>(&(static_cast<thread_struct*>(0)->s));
const unsigned long kSepcOffsetInThreadStruct =
    reinterpret_cast<unsigned long>(&(static_cast<thread_struct*>(0)->sepc));
const unsigned long kSStatusOffsetInThreadStruct =
    reinterpret_cast<unsigned long>(&(static_cast<thread_struct*>(0)->sstatus));
const unsigned long kSScratchOffsetInThreadStruct =
    reinterpret_cast<unsigned long>(
        &(static_cast<thread_struct*>(0)->sscratch));
const unsigned long kThreadOffsetInTaskStruct =
    reinterpret_cast<unsigned long>(&(static_cast<task_struct*>(0)->thread));
const unsigned long kThreadRAOffsetInTaskStruct =
    kThreadOffsetInTaskStruct + kRAOffsetInThreadStruct;
const unsigned long kThreadSPOffsetInTaskStruct =
    kThreadOffsetInTaskStruct + kSPOffsetInThreadStruct;
const unsigned long kThreadSOffsetInTaskStruct =
    kThreadOffsetInTaskStruct + kSOffsetInThreadStruct;
const unsigned long kThreadSepcOffsetInTaskStruct =
    kThreadOffsetInTaskStruct + kSepcOffsetInThreadStruct;
const unsigned long kThreadSStatusOffsetInTaskStruct =
    kThreadOffsetInTaskStruct + kSStatusOffsetInThreadStruct;
const unsigned long kThreadSScratchOffsetInTaskStruct =
    kThreadOffsetInTaskStruct + kSScratchOffsetInThreadStruct;
const unsigned long kPgdOffsetInTaskStruct =
    reinterpret_cast<unsigned long>(&(static_cast<task_struct*>(0)->pgd));

struct task_struct* idle;            // idle process
struct task_struct* current;         // the current running process
struct task_struct* task[NR_TASKS];  // all the processes

extern void uapp_start(), uapp_end();

void task_init() {
  idle = current = task[0] = reinterpret_cast<task_struct*>(kalloc());
  task[0]->state = TASK_RUNNING;
  task[0]->counter = 0;
  task[0]->priority = PRIORITY_MIN;
  task[0]->pid = 0;
  for (int i = 1; i < NR_TASKS; ++i) {
    task[i] = reinterpret_cast<task_struct*>(kalloc());
    task[i]->state = TASK_RUNNING;
    task[i]->counter = 0;
    task[i]->priority =
        i ? rand() % (PRIORITY_MAX - PRIORITY_MIN + 1) + PRIORITY_MIN : 0;
    task[i]->pid = i;
    task[i]->thread.ra =
        reinterpret_cast<decltype(task[i]->thread.ra)>(__dummy);
    task[i]->thread.sp =
        reinterpret_cast<decltype(task[i]->thread.sp)>(task[i]) + PGSIZE;
    task[i]->thread.sepc = USER_START;
    task[i]->thread.sstatus = csr_read(sstatus);
    task[i]->thread.sstatus &= ~(1ull << 8);  // clear SPP
    task[i]->thread.sstatus |= 1ull << 5;     // set SPIE
    task[i]->thread.sstatus |= 1ull << 18;    // set SUM
    task[i]->thread.sscratch = USER_END;
    // set up the page table
    pagetable_t pgtbl = reinterpret_cast<pagetable_t>(kalloc());
    memcpy(pgtbl, swapper_pg_dir, PGSIZE);
    create_mapping(pgtbl, USER_START,
                   reinterpret_cast<uint64>(uapp_start) - PA2VA_OFFSET,
                   reinterpret_cast<uint64>(uapp_end) -
                       reinterpret_cast<uint64>(uapp_start),
                   0b1111);  // uapp
    create_mapping(pgtbl, USER_END - PGSIZE, kalloc() - PA2VA_OFFSET, PGSIZE,
                   0b1011);  // user stack
    const uint64 pgtbl_ppn =
        (reinterpret_cast<uint64>(pgtbl) - PA2VA_OFFSET) >> 12;
    uint64 new_satp = csr_read(satp);
    new_satp = (new_satp >> 44) << 44;
    new_satp |= pgtbl_ppn;
    task[i]->pgd = new_satp;
  }
  log(LOG_LEVEL_OK, "Process initialization done\n");
}

void do_timer() {
  if (current != task[0] && --current->counter > 0) return;
  schedule();
}

void schedule() {
  task_struct* nxt = current;
#if defined(SJS) or defined(PRIORITY)
schedule_start:
  uint64 min = -1;
  for (int i = 1; i < NR_TASKS; ++i) {
    if (task[i]->counter <= 0) continue;
#ifdef SJS
    if (task[i]->counter < min) {
      min = task[i]->counter;
#else
    if (task[i]->priority < min) {
      min = task[i]->priority;
#endif
      nxt = task[i];
    }
  }
  if (min == -1ull) {
    for (int i = 1; i < NR_TASKS; ++i) {
      task[i]->counter = rand() % MAX_COUNTER + 1;
      log(LOG_LEVEL_INFO, "SET [PID = %lld COUNTER = %lld]\n", task[i]->pid,
          task[i]->counter);
    }
    goto schedule_start;
  }
#endif
  switch_to(nxt);
}

void switch_to(task_struct* next) {
  if (current == next) return;
  log(LOG_LEVEL_INFO, "switch to [PID = %lld PRIORITY = %lld COUNTER = %lld]\n",
      next->pid, next->priority, next->counter);
  const auto prev = current;
  current = next;
  __switch_to(prev, next);
}

}  // extern "C"

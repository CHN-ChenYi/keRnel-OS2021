extern "C" {

#include "proc.h"

#include "defs.h"
#include "mm.h"
#include "printk.h"
#include "rand.h"

extern void __switch_to(struct task_struct* prev, struct task_struct* next);
extern void __dummy();

#define MAX_COUNTER 10

const unsigned long kRAOffsetInThreadStruct =
    reinterpret_cast<unsigned long>(&(static_cast<thread_struct*>(0)->ra));
const unsigned long kSPOffsetInThreadStruct =
    reinterpret_cast<unsigned long>(&(static_cast<thread_struct*>(0)->sp));
const unsigned long kSOffsetInThreadStruct =
    reinterpret_cast<unsigned long>(&(static_cast<thread_struct*>(0)->s));
const unsigned long kThreadOffsetInTaskStruct =
    reinterpret_cast<unsigned long>(&(static_cast<task_struct*>(0)->thread));
const unsigned long kThreadRAOffsetInTaskStruct =
    kThreadOffsetInTaskStruct + kRAOffsetInThreadStruct;
const unsigned long kThreadSPOffsetInTaskStruct =
    kThreadOffsetInTaskStruct + kSPOffsetInThreadStruct;
const unsigned long kThreadSOffsetInTaskStruct =
    kThreadOffsetInTaskStruct + kSOffsetInThreadStruct;

struct task_struct* idle;            // idle process
struct task_struct* current;         // the current running process
struct task_struct* task[NR_TASKS];  // all the processes

void task_init() {
  for (int i = 0; i < NR_TASKS; ++i) {
    task[i] = (task_struct*)kalloc();
    task[i]->thread_info = NULL;
    task[i]->state = TASK_RUNNING;
    task[i]->counter = 0;
    task[i]->priority =
        i ? rand() % (PRIORITY_MAX - PRIORITY_MIN + 1) + PRIORITY_MIN : 0;
    task[i]->pid = i;
    task[i]->thread.ra =
        reinterpret_cast<decltype(task[i]->thread.ra)>(__dummy);
    task[i]->thread.sp =
        reinterpret_cast<decltype(task[i]->thread.sp)>(task[i]) + PGSIZE;
  }
  printk("...proc_init done!\n");
  idle = current = task[0];
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
  if (min == -1) {
    for (int i = 1; i < NR_TASKS; ++i) {
      task[i]->counter = rand() % MAX_COUNTER + 1;
      printk("SET [PID = %lld COUNTER = %lld]\n", task[i]->pid,
             task[i]->counter);
    }
    goto schedule_start;
  }
#endif
  switch_to(nxt);
}

void switch_to(task_struct* next) {
  if (current == next) return;
  printk("switch to [PID = %lld PRIORITY = %lld COUNTER = %lld]\n", next->pid,
         next->priority, next->counter);
  const auto prev = current;
  current = next;
  __switch_to(prev, next);
}

void dummy() {
  uint64 MOD = 1000000007;
  uint64 auto_inc_local_var = 0;
  int last_counter = -1;
  while (1) {
    if (last_counter == -1 || current->counter != last_counter) {
      last_counter = current->counter;
      auto_inc_local_var = (auto_inc_local_var + 1) % MOD;
      printk("[PID = %lld] is running. auto_inc_local_var = %lld\n",
             current->pid, auto_inc_local_var);
    }
  }
}

}  // extern "C"

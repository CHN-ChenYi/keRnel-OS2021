#pragma once

#include "types.h"

// #define NR_TASKS (1 + 31)  // max process number plus 1(idle process)
#define NR_TASKS (1 + 2)  // max process number plus 1(idle process)

#define TASK_RUNNING 0  // for simplicity, all processes have only one state

#define PRIORITY_MIN 1
#define PRIORITY_MAX 10

typedef struct thread_info {
  uint64 kernel_sp;
  uint64 user_sp;
} thread_info;

// state segment
typedef struct thread_struct {
  uint64 ra;
  uint64 sp;
  uint64 s[12];
} thread_struct;
extern const unsigned long kRAOffsetInThreadStruct;
extern const unsigned long kSPOffsetInThreadStruct;
extern const unsigned long kSOffsetInThreadStruct;

typedef struct task_struct {
  thread_info* thread_info;
  uint64 state;
  uint64 counter;   // remaining time
  uint64 priority;  // the smaller, the higher
  uint64 pid;

  thread_struct thread;
} task_struct;
extern const unsigned long kThreadOffsetInTaskStruct;
extern const unsigned long kThreadRAOffsetInTaskStruct;
extern const unsigned long kThreadSPOffsetInTaskStruct;
extern const unsigned long kThreadSOffsetInTaskStruct;

/**
 * @brief Initialize NR_TASKS threads
 *
 */
void task_init();

/**
 * @brief Timer interrupt handler
 * @details Decide whether to schedule the tasks.
 */
void do_timer();

/**
 * @brief Scheduler
 * @details Select the task to be executed next.
 */
void schedule();

/**
 * @brief Context switcher
 *
 * @param next the task to be executed
 */
void switch_to(task_struct* next);

/**
 * @brief A testing loop function
 * @details constantly print its pid and an incresing local variable
 */
void dummy();

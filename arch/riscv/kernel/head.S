.extern start_kernel                     # main.c
.extern _traps                           # entry.S
.extern unsigned long TIMECLOCK          # clock.c
.extern unsigned long SBI_TIMER_EXT_ID   # clock.c
.extern unsigned long SBI_TIMER_FUNC_ID  # clock.c

.set    STACK_SIZE, 4  # unit: KB

.macro LOAD_D   reg, var
    la  \reg, \var
    ld  \reg, 0(\reg)
.endm

    .section .text.init
    .globl _start
_start:
    # set stvec = _traps
    la      t0, _traps
    csrw    stvec, t0

    # set sie[STIE] = 1
    li      t0, 0b100000
    csrs    sie, t0

    # set first time interrupt
    rdtime  a0
    LOAD_D  t0, TIMECLOCK
    add     a0, a0, t0
    li      a1, 0
    li      a2, 0
    li      a3, 0
    li      a4, 0
    li      a5, 0
    LOAD_D  a6, SBI_TIMER_FUNC_ID
    LOAD_D  a7, SBI_TIMER_EXT_ID
    ecall  # sbi_set_timer

    # set sstatus[SIE] = 1
    csrsi   sstatus, 0b10

    # set up program stack
    la      sp, boot_stack_top
    j       start_kernel

    .section .bss.stack
    .globl boot_stack
boot_stack:
    .space 1024 * STACK_SIZE

    .globl boot_stack_top
boot_stack_top:

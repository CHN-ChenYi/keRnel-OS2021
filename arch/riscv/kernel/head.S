.extern setup_vm                         # vm.c
.extern setup_vm_final                   # vm.c
.extern mm_init                          # mm.c
.extern task_init                        # proc.cc
.extern start_kernel                     # main.c
.extern _traps                           # entry.S
.extern unsigned long TIMECLOCK          # clock.c
.extern unsigned long SBI_TIMER_EXT_ID   # clock.c
.extern unsigned long SBI_TIMER_FUNC_ID  # clock.c
.extern unsigned long PA2VA_OFFSET       # vm.c
.extern unsigned long[] early_pgtbl      # vm.c

.set    STACK_SIZE, 4  # unit: KB

.macro LOAD_D   reg, var
    la  \reg, \var
    ld  \reg, 0(\reg)
.endm

    .section .text.init
    .globl _start
_start:
    # set up program stack
    la      sp, boot_stack_top

    call    setup_vm
    call    relocate

    call    mm_init

    call    setup_vm_final

    call    task_init

    call    setup_trap

    j       start_kernel

relocate:
    LOAD_D  t0, PA2VA_OFFSET
    add     ra, ra, t0
    add     sp, sp, t0

    # Point stvec to virtual address of intruction after satp write
    la      t1, 1f
    add     t1, t1, t0
    csrw    stvec, a1

    # set satp with early_pgtbl
    la      t0, early_pgtbl
    srli    t0, t0, 12  # PPN = page table base address >> 12
    li      t1, 8
    sll     t1, t1, 60  # Sv39 mode
    add     t0, t1, t0
    csrw    satp, t0
1:
    # flush tlb
    sfence.vma zero, zero

    ret

setup_trap:
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
    # csrsi   sstatus, 0b10

    ret

    .section .bss.stack
    .globl boot_stack
boot_stack:
    .space 1024 * STACK_SIZE

    .globl boot_stack_top
boot_stack_top:

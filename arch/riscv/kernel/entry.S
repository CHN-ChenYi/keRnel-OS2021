.extern trap_handler

.altmacro
.set    REG_SIZE, 8
.set    CONTEXT_SIZE, 36

.macro SAVE reg, offset
    sd  \reg, \offset*8(sp)
.endm

.macro SAVE_N n
    SAVE    x\n, \n
.endm

.macro LOAD reg, offset
    ld  \reg, \offset*8(sp)
.endm

.macro LOAD_N n
    LOAD    x\n, \n
.endm

    .section .text.entry
    .align 2
    .globl _traps
_traps:
    # save 31 registers and sepc to stack
    addi    sp, sp, -1 * CONTEXT_SIZE * REG_SIZE
    SAVE    x1, 1
    .set    n, 3
    .rept   29
        SAVE_N  %n
        .set    n, n + 1
    .endr
    addi    t0, sp, 1 * CONTEXT_SIZE * REG_SIZE  # original sp
    SAVE    t0, 2  # save sp
    # save csrs
    # https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/tree/arch/riscv/kernel/entry.S#n85
    csrr    t0, sepc
    csrr    t1, sstatus
    csrr    t2, stval
    csrr    t3, scause
    csrr    t4, sscratch
    SAVE    t0, 32
    SAVE    t1, 33
    SAVE    t2, 34
    SAVE    t3, 35
    SAVE    t4, 36

    # call trap_handler
    mv      a0, t3  # scause
    mv      a1, t0  # sepc
    jal     trap_handler

    # restore sepc and 32 registers (x2(sp) should be restore last) from stack
    LOAD    t0, 32
    LOAD    t1, 33
    LOAD    t2, 34
    LOAD    t3, 35
    LOAD    t4, 36
    csrw    sepc, t0
    csrw    sstatus, t1
    csrw    stval, t2
    csrw    scause, t3
    csrw    sscratch, t4
    .set    n, 3
    .rept   29
        LOAD_N %n
        .set    n, n + 1
    .endr
    LOAD    x1, 1
    LOAD    x2, 2  # load sp    

    sret

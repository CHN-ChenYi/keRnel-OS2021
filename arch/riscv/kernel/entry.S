.extern trap_handler                                    # trap.c
.extern current                                         # proc.cc
.extern unsigned long kThreadOffsetInTaskStruct         # proc.cc
.extern unsigned long kThreadRAOffsetInTaskStruct       # proc.cc
.extern unsigned long kThreadSPOffsetInTaskStruct       # proc.cc
.extern unsigned long kThreadSOffsetInTaskStruct        # proc.cc
.extern unsigned long kThreadSepcOffsetInTaskStruct     # proc.cc
.extern unsigned long kThreadSStatusOffsetInTaskStruct  # proc.cc
.extern unsigned long kThreadSScratchOffsetInTaskStruct # proc.cc
.extern unsigned long kPgdOffsetInTaskStruct            # proc.cc

.altmacro
.set    REG_SIZE, 8
.set    CONTEXT_SIZE, 36

.macro LOAD_D   reg, var
    la  \reg, \var
    ld  \reg, 0(\reg)
.endm

.macro SAVE_X_SP reg, offset
    sd  \reg, \offset*8(sp)
.endm

.macro SAVE_X_SP_N n
    SAVE_X_SP    x\n, \n
.endm

.macro LOAD_X_SP reg, offset
    ld  \reg, \offset*8(sp)
.endm

.macro LOAD_X_SP_N n
    LOAD_X_SP    x\n, \n
.endm

.macro SAVE_S_T0 reg, offset
    sd  \reg, \offset*8(t0)
.endm

.macro SAVE_S_T0_N n
    SAVE_S_T0    s\n, \n
.endm

.macro LOAD_S_T0 reg, offset
    ld  \reg, \offset*8(t0)
.endm

.macro LOAD_S_T0_N n
    LOAD_S_T0    s\n, \n
.endm

.macro SWAP_SSCRATCH_AND_SP reg
    csrr    \reg, sscratch
    csrw    sscratch, sp
    addi    sp, \reg, 0
.endm

    .section .text.entry
    .align 2
    .globl _traps
_traps:
    csrr    t0, sscratch
    beq     t0, x0, _traps_entry
    SWAP_SSCRATCH_AND_SP t0
_traps_entry:
    # save 31 registers and sepc to stack
    addi    sp, sp, -1 * CONTEXT_SIZE * REG_SIZE
    SAVE_X_SP    x1, 0
    .set    n, 2
    .rept   29
        SAVE_X_SP_N  %n
        .set    n, n + 1
    .endr
    addi    t0, sp, 1 * CONTEXT_SIZE * REG_SIZE  # original sp
    SAVE_X_SP    t0, 1  # save sp
    # save csrs
    # https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/tree/arch/riscv/kernel/entry.S#n85
    csrr    t0, sepc
    csrr    t1, sstatus
    csrr    t2, stval
    csrr    t3, scause
    csrr    t4, sscratch
    SAVE_X_SP    t0, 31
    SAVE_X_SP    t1, 32
    SAVE_X_SP    t2, 33
    SAVE_X_SP    t3, 34
    SAVE_X_SP    t4, 35

    # call trap_handler
    mv      a0, t3    # scause
    mv      a1, t0    # sepc
    addi    a2, sp, 0 # pt_regs
    call    trap_handler

    # restore sepc and 32 registers (x2(sp) should be restore last) from stack
    LOAD_X_SP    t0, 31
    LOAD_X_SP    t1, 32
    LOAD_X_SP    t2, 33
    LOAD_X_SP    t3, 34
    LOAD_X_SP    t4, 35
    add     t0, t0, a0  # return value from trap handler
    csrw    sepc, t0
    csrw    sstatus, t1
    csrw    stval, t2
    csrw    scause, t3
    csrw    sscratch, t4
    .set    n, 2
    .rept   29
        LOAD_X_SP_N %n
        .set    n, n + 1
    .endr
    LOAD_X_SP    x1, 0
    LOAD_X_SP    x2, 1  # load sp

    csrr    t0, sscratch
    beq     t0, x0, _traps_end
    SWAP_SSCRATCH_AND_SP t0
_traps_end:
    sret

    .globl __switch_to
__switch_to:
    LOAD_D  t1, kThreadOffsetInTaskStruct
    LOAD_D  t2, kThreadRAOffsetInTaskStruct
    LOAD_D  t3, kThreadSPOffsetInTaskStruct
    LOAD_D  t4, kThreadSOffsetInTaskStruct
    LOAD_D  t5, kThreadSepcOffsetInTaskStruct
    LOAD_D  t6, kThreadSStatusOffsetInTaskStruct
    LOAD_D  s7, kThreadSScratchOffsetInTaskStruct
    LOAD_D  s8, kPgdOffsetInTaskStruct

    # save state to prev process
    add     t0, a0, t2
    sd      ra, 0(t0)
    add     t0, a0, t3
    sd      sp, 0(t0)
    add     t0, a0, t4
    .set    n, 0
    .rept   12
        SAVE_S_T0_N %n
        .set    n, n + 1
    .endr
    add     t0, a0, t5
    csrr    s0, sepc
    sd      s0, 0(t0)
    add     t0, a0, t6
    csrr    s0, sstatus
    sd      s0, 0(t0)
    add     t0, a0, s7
    csrr    s0, sscratch
    sd      s0, 0(t0)
    add     t0, a0, s8
    csrr    s0, satp
    sd      s0, 0(t0)

    # restore state from next process
    add     t0, a1, t2
    ld      ra, 0(t0)
    add     t0, a1, t3
    ld      sp, 0(t0)
    add     t0, a0, t4
    .set    n, 0
    .rept   12
        LOAD_S_T0_N %n
        .set    n, n + 1
    .endr
    add     t0, a1, t5
    ld      s0, 0(t0)
    csrw    sepc, s0
    add     t0, a1, t6
    ld      s0, 0(t0)
    csrw    sstatus, s0
    add     t0, a1, s7
    ld      s0, 0(t0)
    csrw    sscratch, s0
    add     t0, a1, s8
    ld      s0, 0(t0)
    csrw    satp, s0
    sfence.vma zero, zero

    ret

    .global __dummy
__dummy:
    SWAP_SSCRATCH_AND_SP t0
    sret

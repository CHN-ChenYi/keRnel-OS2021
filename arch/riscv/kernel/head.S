.extern start_kernel

    .section .text.entry
    .globl _start
_start:
    # set up program stack
    la sp, boot_stack_top
    j start_kernel

    .section .bss.stack
    .globl boot_stack
boot_stack:
    .space 1024 * 4  # 4K

    .globl boot_stack_top
boot_stack_top:

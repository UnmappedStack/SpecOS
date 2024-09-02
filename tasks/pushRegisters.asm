; A super short assembly file for pushing all registers onto the stack
; which is used for task switching, as part of the SpecOS kernel project.
; Copyright (C) 2024 Jake Steinburger under the MIT license. See the GitHub repository for more information.

[BITS 64]

section .text

global pushAllRegisters

extern taskSwitchPart2

pushAllRegisters:
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
    ; call part 2
    jmp taskSwitchPart2

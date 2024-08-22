; Exception handlers for SpecOS kernel project.
; Copyright (C) 2024 Jake Steinburger under the MIT license. See the GitHub repo for more details.
; Oh, how very much I love writing assembly. It's not too bad I guess. (this is nasm btw)

[bits 64]

section .text

extern exceptionHandler ; defined in panic.c (this isn't kpanic)

; and now just basically define all the individual handlers

global divideException
global debugException
global breakpointException
global overflowException
global boundRangeExceededException
global invalidOpcodeException
global deviceNotAvaliableException
global doubleFaultException ; no push 0
global coprocessorSegmentOverrunException
global invalidTSSException ; no push 0
global segmentNotPresentException ; no push 0
global stackSegmentFaultException ; no push 0
global generalProtectionFaultException ; no push 0
global pageFaultException ; no push 0
; skip one
global floatingPointException
global alignmentCheckException ; no push 0
global machineCheckException
global simdFloatingPointException
global virtualisationException

divideException:
    push 0
    push 0
    jmp baseHandler

debugException:
    push 0
    push 1
    jmp baseHandler

breakpointException:
    push 0
    push 3
    jmp baseHandler

overflowException:
    push 0
    push 4
    jmp baseHandler

boundRangeExceededException:
    push 0
    push 5
    jmp baseHandler

invalidOpcodeException:
    push 0
    push 6
    jmp baseHandler

deviceNotAvaliableException:
    push 0
    push 7
    jmp baseHandler

doubleFaultException:
    push 8
    jmp baseHandler

coprocessorSegmentOverrunException:
    push 0
    push 9
    jmp baseHandler

invalidTSSException:
    push 10
    jmp baseHandler

segmentNotPresentException:
    push 11
    jmp baseHandler

stackSegmentFaultException:
    push 12
    jmp baseHandler

generalProtectionFaultException:
    push 13
    jmp baseHandler

align 0x08, db 0x00
pageFaultException:
    push 14
    jmp baseHandler

floatingPointException:
    push 0
    push 16
    jmp baseHandler

alignmentCheckException:
    push 17
    jmp baseHandler

machineCheckException:
    push 0
    push 18
    jmp baseHandler

simdFloatingPointException:
    push 0
    push 19
    jmp baseHandler

virtualisationException:
    push 0
    push 20
    jmp baseHandler

align 0x08, db 0x00
baseHandler:
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
   mov rax, cr2
   push rax
   cld
   mov rdi, rsp
   call exceptionHandler 
   add rsp, 8
   pop r15
   pop r14
   pop r13
   pop r12
   pop r11
   pop r10
   pop r9
   pop r8
   pop rbp
   pop rdi
   pop rsi
   pop rdx
   pop rcx
   pop rbx
   pop rax
   add rsp, 0x10
   iretq

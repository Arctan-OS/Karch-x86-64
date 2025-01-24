bits 64
section .text

global _syscall
extern Arc_SyscallTable
extern pml4
extern __KERNEL_STACK__
_syscall:
    mov rsp, __KERNEL_STACK__
    mov rbp, rsp
    push rcx
    push r11
    lea rbx, [rel pml4]
    xor rax, rax
    mov eax, dword [rbx]
    mov cr3, rax
    shl rdi, 3
    mov rax, Arc_SyscallTable
    add rax, rdi
    mov rax, [rax]
    call rax
    pop r11
    pop rcx
    jmp $
    o64 sysret

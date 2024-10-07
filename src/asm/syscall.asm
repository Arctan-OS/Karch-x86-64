bits 64
section .text

global _syscall
extern Arc_SyscallTable
_syscall:
    push rcx
    push r11
    shl rdi, 3
    mov rax, [rel Arc_SyscallTable]
    add rax, rdi
    mov rdi, rsi
    call [rax]
    pop r11
    pop rcx
    o64 sysret

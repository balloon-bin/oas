_start:
    mov eax, 555            ; move 555 into eax
    push 0o777
    xor eax, 0xDEADBEEF
    and ecx, 0o770
    mov edx, 0b01010101
    push 0xffff:64
    push 0o777:16
    push 0b0001:16

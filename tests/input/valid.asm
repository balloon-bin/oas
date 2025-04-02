.section text

; Small valid code snippet that should contain all different AST nodes

_start:
    mov eax, ebx
    lea eax, [eax + ebx * 4 + 8]
    lea eax, [eax + 8]
    lea eax, [eax + ebx * 8]
    lea eax, [esp - 24]
    lea eax, [eax + ebx * 4 - 8]
    lea eax, [_start]
    mov eax, _start
    mov eax, 555
    push 0o777
    xor eax, 0xDEADBEEF
    and ecx, 0o770
    mov edx, 0b01010101
    push 0xffff:64
    push 0o777:16
    push 0b0001:16

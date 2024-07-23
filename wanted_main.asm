.data

.code
ALIGN 16

EXTERN wanted_flag : BYTE
EXTERN JumpBack2 : QWORD

wanted_main PROC
newmemB:
	cmp byte ptr [wanted_flag], 0
	je codeB
	ret

codeB:
    push rsi
	push rdi
	sub rsp,58h
	lea eax,[rcx-01]
	mov rdi,rdx
    mov esi,ecx
    jmp [JumpBack2]

wanted_main ENDP
END
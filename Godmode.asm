.data

.code
ALIGN 16

EXTERN moduleBase : QWORD
EXTERN Godmode_flag : BYTE
EXTERN JumpBack0 : QWORD

Godmode PROC
newmemA:
	cmp byte ptr [Godmode_flag], 0
	je codeA
	push rsi
	mov rsi, [moduleBase]
	cmp rcx, [rsi + 4D6CC70h]
	pop rsi
	jne codeA
	ret

codeA:
	mov [rsp+18h],rbx
	mov [rsp+20h],rsi
	push rdi
	push r12
	push r13
    jmp [JumpBack0]

Godmode ENDP
END
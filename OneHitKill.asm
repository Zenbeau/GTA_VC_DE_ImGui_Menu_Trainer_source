.data

.code
ALIGN 16

EXTERN moduleBase : QWORD
EXTERN ohk_flag : BYTE
EXTERN JumpBack1 : QWORD
EXTERN dmgVal : DWORD

OneHitKill PROC
newmemF:
	cmp [ohk_flag],0
	je codeF
	push rbx
	mov rbx,[moduleBase]
	cmp rcx,[rbx + 4D6CC70h]
	pop rbx
	je codeF
	movss xmm3,[dmgVal]

codeF:
	push r14
	push r15
	sub rsp,000000B0h
	push rbx
	mov rbx,[moduleBase]
	movzx eax,byte ptr [rbx + 4FBF06Ah]
	pop rbx
	jmp [JumpBack1]

OneHitKill ENDP
END
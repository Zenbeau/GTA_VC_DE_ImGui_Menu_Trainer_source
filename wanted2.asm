.data

.code
ALIGN 16

EXTERN moduleBase : QWORD
EXTERN wanted_flag : BYTE
EXTERN JumpBack4 : QWORD
EXTERN conditional_jump4 : QWORD

wanted2 PROC
newmemD:
	cmp byte ptr [wanted_flag], 0
	je codeD
	ret

codeD:
	push rax
	mov rax,[moduleBase]
	cmp byte ptr [moduleBase + 53A3C6Eh],00
	pop rax
	mov r8,rcx
	push rax
	mov rax,[moduleBase]
	jne conditionA
    jmp [JumpBack4]

conditionA: 
  jmp [conditional_jump4]

wanted2 ENDP
END
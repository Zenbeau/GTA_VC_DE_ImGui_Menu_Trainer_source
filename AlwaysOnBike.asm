.data

.code
ALIGN 16

EXTERN moduleBase : QWORD
EXTERN toggle10 : BYTE
EXTERN JumpBack10 : QWORD
EXTERN conditionaljumpF : QWORD

AlwaysOnBike PROC
newmemK:
	cmp byte ptr [toggle10], 0
	je codeK
	push rax
	mov rax,[moduleBase]
	cmp rsi,[rax + 4D6CC70h]
	pop rax
	jne codeK
	jmp conditionF

codeK:
	mov byte ptr [rsp+28h],00
	mov r9,rsi
	mov edx,00000027h
	mov byte ptr [rsp+20h],00
    jmp [JumpBack10]

conditionF:
	jmp [conditionaljumpF]

AlwaysOnBike ENDP
END
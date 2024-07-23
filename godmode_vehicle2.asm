.data

.code
ALIGN 16

EXTERN moduleBase : QWORD
EXTERN Godmode_flag : BYTE
EXTERN JumpBack7 : QWORD
EXTERN conditionaljumpD : QWORD

Godmode_vehicle2 PROC
newmemH:
	mov edx,[rcx+6Ch]  ;originalcode
	and edx,000000F8h  ;originalcode
	lea eax,[rdx-28h]  ;originalcode

	push rax
	mov rax,[moduleBase]
	cmp r9,[rax + 4D6CC70h]
	pop rax
	jne codeH

	jmp conditionD
	
codeH:
	test eax,0FFFFFFF7h

conditionD:
	jmp [conditionaljumpD]

Godmode_vehicle2 ENDP
END
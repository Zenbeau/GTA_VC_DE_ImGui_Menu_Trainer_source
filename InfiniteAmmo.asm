.data

.code
ALIGN 16

EXTERN moduleBase : QWORD
EXTERN ammo_flag : BYTE
EXTERN JumpBack4 : QWORD
EXTERN conditionaljumpA : QWORD
EXTERN conditionaljumpB : QWORD

InfiniteAmmo PROC
newmemD:
	cmp byte ptr [ammo_flag], 0
	je codeD
	push rsi
    mov rsi,[moduleBase]
    cmp rbx,[rsi + 4D6CC70h]
    pop rsi
    jne codeD
    mov dword ptr [rdi+0Ch],9931
    mov dword ptr [rdi+08],31
    cmp dword ptr [rdi+08],00
    jg conditionA
    mov r8d,[rdi+0Ch]
    test r8d,r8d
    jng conditionB

codeD:
	cmp dword ptr [rdi+08],00
    jg conditionA
    mov r8d,[rdi+0Ch]
    test r8d,r8d
    jng conditionB
    jmp [JumpBack4]
    
conditionA:
    jmp [conditionaljumpA]

conditionB:
    jmp [conditionaljumpB]

InfiniteAmmo ENDP
END
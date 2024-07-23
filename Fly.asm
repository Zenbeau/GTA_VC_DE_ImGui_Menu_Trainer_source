.data

.code
ALIGN 16

;EXTERN moduleBase : QWORD
EXTERN toggle1 : BYTE
EXTERN JumpBack1 : QWORD

Fly PROC
newmemB:
    
codeB:
    movss xmm0,dword ptr [rdi+40h]
    movss xmm1,dword ptr [rdi+38h]
    movss xmm2,dword ptr [rdi+3Ch]
    jmp [JumpBack1]

Fly ENDP
END
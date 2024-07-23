.data

.code
ALIGN 16

EXTERN moduleBase : QWORD
EXTERN wanted_flag : BYTE
EXTERN JumpBack2 : QWORD

wanted1 PROC
newmemB:
	pushf
    cmp byte ptr [wanted_flag], 0
	je codeB
    popf
    cmp dword ptr [rcx+20h],0
    jne codeB
    ret
    
codeB:
    popf
	mov [rsp+08],rbx
    push rdi
    sub rsp,20h
    mov eax,[rcx]
    mov rbx,rcx
    jmp [JumpBack2]

wanted1 ENDP
END
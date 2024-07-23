.data

.code
ALIGN 16

EXTERN Godmode_flag : BYTE
EXTERN JumpBack6 : QWORD

Godmode_vehicle PROC
newmemG:
	cmp byte ptr [Godmode_flag], 0
	je codeG
	ret

codeG:
	mov [rsp+08h],rbx
	mov [rsp+10h],rsi
	push rdi
	sub rsp,40h
    jmp [JumpBack6]

Godmode_vehicle ENDP
END
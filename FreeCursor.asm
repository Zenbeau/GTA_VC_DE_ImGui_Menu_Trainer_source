.data

.code
ALIGN 16

EXTERN show : BYTE
EXTERN JumpBack3 : QWORD

FreeCursor PROC
newmemC:
	cmp byte ptr [show], 0
	je codeC
	ret

codeC:
	mov [rsp+10h],rbx
    mov [rsp+18h],rsi
    push rdi
    sub rsp,20h
    jmp [JumpBack3]

FreeCursor ENDP
END
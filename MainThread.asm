.data

.code
ALIGN 16

EXTERN JumpBack9 : QWORD
EXTERN OrderStuff : PROC

MainThread PROC
newmemJ:
	push rbx
    push rdx
    push rcx
    push rdi
    push rsi
    push rbp
    push r8
    push r9
    push r13
    push r14
    sub rsp,48h
    call OrderStuff
    add rsp,48h
    pop r14
    pop r13
    pop r9
    pop r8
    pop rbp
    pop rsi
    pop rdi
    pop rcx
    pop rdx
    pop rbx

codeJ:
	mov [rsp+08],rbx
    mov [rsp+10h],rbp
    mov [rsp+18h],rsi
    jmp [JumpBack9]

MainThread ENDP
END
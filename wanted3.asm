.data

.code
ALIGN 16

EXTERN wanted_flag : BYTE
EXTERN JumpBack5 : QWORD

wanted3 PROC
newmemE:
	cmp byte ptr [wanted_flag], 0
	je codeE
	;mov dword ptr [rcx+0],0     ;chaos meter
    ;mov dword ptr [rcx+18h],0   ;maybe has to do with which entity will spawn and chase you i.e. cop, swat, fbi, army...not sure
    ;mov dword ptr [rcx+20h],0  ;wanted stars
    ret

codeE:
    push rbx
    push rsi
    push r15
    sub rsp,30h
    mov eax,[rcx+10h]
    xor r15d,r15d
    jmp [JumpBack5]

wanted3 ENDP
END
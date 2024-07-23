.data

.code
ALIGN 16

EXTERN timer1_flag : BYTE
EXTERN JumpBack5 : QWORD
EXTERN conditionaljumpC : QWORD

MissionTimer PROC
newmemE:
	cmp byte ptr [timer1_flag], 0
	je codeE
	;mov [r8+r12+04F771D0h],ecx
    jns conditionC
    mov [r8+r12+04F771D0h],r15d

codeE:
	mov [r8+r12+04F771D0h],ecx
    jns conditionC
    mov [r8+r12+04F771D0h],r15d
    jmp [JumpBack5]

conditionC:
    jmp [conditionaljumpC]

MissionTimer ENDP
END
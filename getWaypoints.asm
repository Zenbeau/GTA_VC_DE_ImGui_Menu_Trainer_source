.data

.code
ALIGN 16

;EXTERN waypoints_flag : BYTE
EXTERN JumpBack8 : QWORD
EXTERN waypointBase : QWORD

Waypoints PROC
newmemI:
	;cmp byte ptr [waypoints_flag], 0
	;je codeI
	push rax
    lea rax,[rcx+rdx]
    mov [waypointBase],rax
    pop rax

codeI:
    movss xmm9,dword ptr [rcx+rdx+0518629Ch]
    movss xmm8,dword ptr [rcx+rdx+051862A0h]
    jmp [JumpBack8]

Waypoints ENDP
END
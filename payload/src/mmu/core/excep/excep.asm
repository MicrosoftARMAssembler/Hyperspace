.code
extern kd_trap_org:dq
extern get_exception_context:proc
extern get_exception_record:proc
extern exception_handler:proc
extern ke_bug_check:proc

;r13 = exception context
kd_trap_hk proc
	push rcx
	push rdx
    sub rsp,0E8h  

	call get_exception_context
	cmp rax,1
	jne exit

	call get_exception_record
	cmp rax,0
	je bugcheck

    mov rcx, rax
	mov rdx, r13
	call exception_handler

bugcheck:
    xor rcx, rcx
    call ke_bug_check

exit:
    add rsp, 0E8h  
	pop rdx
	pop rcx

    jmp [kd_trap_org]
kd_trap_hk endp

callout_return proc
    mov eax, ss
    push rax

    mov rax, [rcx + 0]
    push rax

    mov rax, [rcx + 78h]    
    ;xor rax, 200h ; enable interrupts
    push rax

    mov eax, cs
    push rax

    mov rax, [rcx + 8]
    push rax

    mov rdx, [rcx + 18h]
    mov r8,  [rcx + 20h]
    mov r9,  [rcx + 28h]
    mov rax, [rcx + 30h]

    mov r12, [rcx + 38h]
    mov r13, [rcx + 40h]
    mov r14, [rcx + 48h]
    mov r15, [rcx + 50h]
    mov rdi, [rcx + 58h]
    mov rsi, [rcx + 60h]
    mov rbx, [rcx + 68h]
    mov rbp, [rcx + 70h]

    mov rcx, [rcx + 10h]

    xor rax, rax
    iretq
callout_return endp

get_r12 proc
    mov rax,r12
    ret
get_r12 endp

set_r12 proc
    mov r12,rcx
    ret
set_r12 endp

get_r13 proc
    mov rax,r13
    ret
get_r13 endp

set_r13 proc
    mov r13,rcx
    ret
set_r13 endp
end
.data

info struct

	rtl_user_thread_start_ptr	dq 1
	base_thread_init_thunk_ptr  dq 1

	frames_first_ptr			dq 1
	frames_second_ptr			dq 1

	gadgets_jmp_ptr				dq 1
	gadgets_add_ptr				dq 1

	frames_first_size			dq 1
	frames_second_size			dq 1

	gadgets_jmp_size			dq 1
	gadgets_add_size			dq 1

	rtl_user_thread_start_size	dq 1
	base_thread_init_thunk_size	dq 1

	stack_offset				dq 1

	gadgets_jmp_ref				dq 1
	spoof_ptr					dq 1
	ret_addr					dq 1

	args_cnt					dq 1
	arg1						dq 1
	arg2						dq 1
	arg3						dq 1
	arg4						dq 1
	arg5						dq 1
	arg6						dq 1
	arg7						dq 1
	arg8						dq 1

	is_float_arg1				db 1
	is_float_arg2				db 1
	is_float_arg3				db 1
	is_float_arg4				db 1

info ends

.code

get_float_register proc
	
	ret

get_float_register endp

get_double_register proc

	ret

get_double_register	endp

get_current_rsp	proc
	
	mov rax, rsp
	add rax, 08h
	ret

get_current_rsp	endp

restore proc

    mov	rsp, rbp
    add rsp, 200h

    mov rbp, [rsp + 08h]
    mov rbx, [rsp + 10h]
    mov r15, [rsp + 18h]

    ret

restore endp

spoof_call proc

	mov [rsp + 08h], rbp
	mov [rsp + 10h], rbx
	mov [rsp + 18h], r15

	mov rbx, [rcx].info.gadgets_jmp_ptr
	mov	[rsp + 20h], rbx
	mov	rbx, rsp
	add	rbx, 20h
	mov	[rcx].info.gadgets_jmp_ref, rbx

	sub	rsp, 200h
	mov	rbp, rsp

	lea	rax, restore
	push rax

	lea	rbx, [rsp]

	push [rcx].info.frames_first_ptr
	add	qword ptr [rsp], 20h

	mov rax, [rcx].info.ret_addr
	sub	rax, [rcx].info.frames_first_size
	sub rsp, [rcx].info.frames_second_size
	mov	r10, [rcx].info.stack_offset

	mov	[rsp + r10], rax

	xor	rax, rax
	push rax

	mov	rax, [rcx].info.frames_first_ptr
	sub	rax, [rcx].info.frames_first_size

	sub	rsp, [rcx].info.rtl_user_thread_start_size
	mov	[rsp + 30h], rax

	push [rcx].info.rtl_user_thread_start_ptr
	add	qword ptr [rsp], 21h
	sub rsp, [rcx].info.base_thread_init_thunk_size

	push [rcx].info.base_thread_init_thunk_ptr
	add qword ptr [rsp], 14h
	sub rsp, [rcx].info.gadgets_jmp_size

	mov rax, [rcx].info.rtl_user_thread_start_ptr
	sub rax, [rcx].info.rtl_user_thread_start_size
	mov [rsp + 30h], rax

	push [rcx].info.gadgets_jmp_ptr

	mov	rax, [rcx].info.base_thread_init_thunk_ptr
	sub	rax, [rcx].info.frames_second_size
	sub rsp, [rcx].info.gadgets_add_size
	mov	[rsp + 30h], rax

	mov r10, [rcx].info.gadgets_jmp_ptr
	mov [rsp + 38h], r10

	push [rcx].info.gadgets_add_ptr

	mov rax, [rcx].info.gadgets_add_size
	mov [rbp + 28h], rax

	mov rax, [rbp + 28h]
	mov [rsp + 28h], rax
	mov rax, [rbp + 30h]
	mov rax, [rcx].info.spoof_ptr

	jmp parameter_handler

spoof_call endp

parameter_handler proc

	cmp	[rcx].info.args_cnt, 8
	je	param_handler_eight

	cmp	[rcx].info.args_cnt, 7
	je	param_handler_seven

	cmp	[rcx].info.args_cnt, 6
	je	param_handler_six

	cmp	[rcx].info.args_cnt, 5
	je	param_handler_five

	cmp	[rcx].info.args_cnt, 4
	je	param_handler_four

	cmp	[rcx].info.args_cnt, 3
	je	param_handler_three

	cmp	[rcx].info.args_cnt, 2
	je	param_handler_two

	cmp	[rcx].info.args_cnt, 1
	je	param_handler_one

	cmp	[rcx].info.args_cnt, 0
	je	param_handler_zero

parameter_handler endp

param_handler_eight proc

	push r15
	mov r15, [rcx].info.arg8
	mov	[rsp + 48h], r15
	pop r15

	jmp param_handler_seven

param_handler_eight endp

param_handler_seven proc

	push r15
	mov r15, [rcx].info.arg7
	mov	[rsp + 40h], r15
	pop r15

	jmp param_handler_six

param_handler_seven endp

param_handler_six proc

	push r15
	mov r15, [rcx].info.arg6
	mov	[rsp + 38h], r15
	pop r15

	jmp param_handler_five

param_handler_six endp

param_handler_five proc

	push r15
	mov r15, [rcx].info.arg5
	mov	[rsp + 30h], r15
	pop r15

	jmp param_handler_four

param_handler_five endp

param_handler_four proc

	cmp [rcx].info.is_float_arg4, 1
	jne mov_reg
	movq xmm3, [rcx].info.arg4
	jmp execute

	mov_reg:
	mov r9, [rcx].info.arg4

	execute:
	jmp param_handler_three

param_handler_four endp

param_handler_three proc

	cmp [rcx].info.is_float_arg3, 1
	jne mov_reg
	movq xmm2, [rcx].info.arg3
	jmp execute

	mov_reg:
	mov r8, [rcx].info.arg3

	execute:
	jmp param_handler_two

param_handler_three endp

param_handler_two proc

	cmp [rcx].info.is_float_arg2, 1
	jne mov_reg
	movq xmm1, [rcx].info.arg2
	jmp execute

	mov_reg:
	mov rdx, [rcx].info.arg2

	execute:
	jmp param_handler_one

param_handler_two endp

param_handler_one proc

	cmp [rcx].info.is_float_arg1, 1
	jne mov_reg
	movq xmm0, [rcx].info.arg1
	jmp execute

	mov_reg:
	mov rcx, [rcx].info.arg1

	execute:
	jmp param_handler_zero

param_handler_one endp

param_handler_zero proc

	jmp qword ptr rax

param_handler_zero endp

end
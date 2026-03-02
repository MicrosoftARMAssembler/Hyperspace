#pragma once

namespace uwd_private {

	typedef struct {

		void*		rtl_user_thread_start_ptr;
		void*		base_thread_init_thunk_ptr;

		void*		frames_first_ptr;
		void*		frames_second_ptr;

		void*		gadgets_jmp_ptr;
		void*		gadgets_add_ptr;

		size_t		frames_first_size, 
					frames_second_size;

		size_t		gadgets_jmp_size, 
					gadgets_add_size;

		size_t		rtl_user_thread_start_size, 
					base_thread_init_thunk_size;

		size_t		stack_offset;

		void*		gadgets_jmp_ref;
		void*		spoof_ptr;

		void*		ret_addr;

		size_t		args_cnt;
		void*		args [8];

		bool		is_floating [4];

	} unwind_cfg;

	extern "C" uint64_t spoof_call (unwind_cfg* settings);
	extern "C" float get_float_register ();
	extern "C" double get_double_register ();

	extern "C" void* get_rsp ();

}
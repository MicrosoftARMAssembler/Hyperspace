#pragma once

namespace uwd_engine {

	using namespace uwd_private;

	typedef struct {

		uint64_t modules[2];
		void* exp[2];

	} uwd_frames;

	class uwd_search {

	public:

		typedef enum {

			gdt_add_rsp,
			gdt_jmp_rbx

		} gadget_type;

	public:

		uwd_search(uint64_t mod_base, unwind_cfg* cfg) : m_mod(mod_base), m_cfg(cfg) {};

		uint32_t find_prolog(PIMAGE_RUNTIME_FUNCTION_ENTRY table, uint32_t last_index, uint32_t* stack_size, uint32_t* save_index, uint32_t* skip, uint64_t* target_off) {

			unwind_info* info;
			uint32_t status = 0, suitable_frames = 0;

			*stack_size = 0;

			for (uint32_t idx_iter = 0; idx_iter < last_index; idx_iter++) {

				info = reinterpret_cast <unwind_info*> (m_mod + table[idx_iter].UnwindData);
				status = get_stack_frame_size(m_mod, info, stack_size);

				if (status != 0) {

					suitable_frames++;

					if (*skip >= suitable_frames) {
						continue;
					}

					*skip = suitable_frames;
					*save_index = idx_iter;
					break;

				}

			}

			*target_off = m_mod + table[*save_index].BeginAddress;

			m_cfg->frames_first_ptr = reinterpret_cast <uint64_t*> (*target_off);
			m_cfg->frames_first_size = *stack_size;

			return status;
		}

		uint32_t find_push_rbp(PIMAGE_RUNTIME_FUNCTION_ENTRY table, uint32_t last_index, uint32_t* stack_size, uint32_t* save_index, uint32_t* skip, uint64_t* target_off) {

			unwind_info* info;
			uint32_t status = 0, suitable_frames = 0;

			*stack_size = 0;

			for (uint32_t idx_iter = 0; idx_iter < last_index; idx_iter++) {

				info = reinterpret_cast <unwind_info*> (m_mod + table[idx_iter].UnwindData);
				status = search_rbp_on_stack(m_mod, info, stack_size);

				if (status != 0) {

					suitable_frames++;

					if (*skip >= suitable_frames) {
						continue;
					}

					*skip = suitable_frames;
					*save_index = idx_iter;
					break;

				}

			}

			*target_off = m_mod + table[*save_index].BeginAddress;

			m_cfg->frames_second_ptr = reinterpret_cast <uint64_t*> (*target_off);
			m_cfg->frames_second_size = *stack_size;

			m_cfg->stack_offset = status;
			return status;
		}

		void find_gadget(PIMAGE_RUNTIME_FUNCTION_ENTRY table, uint32_t last_index, uint32_t* stack_size, uint32_t* save_index, uint32_t* skip, gadget_type type) {

			uint32_t gadgets = 0, status = 0;
			unwind_info* info;

			uint32_t rsp_magic = 952402760;

			for (uint32_t idx_iter = 0; idx_iter < last_index; idx_iter++) {

				uint32_t is_found = FALSE;

				for (uint64_t fn = m_mod + table[idx_iter].BeginAddress; fn < m_mod + table[idx_iter].EndAddress; fn++) {

					if ((*reinterpret_cast <uint32_t*> (fn) == rsp_magic && *reinterpret_cast <uint8_t*> (fn + 4) == 195 && type == gadget_type::gdt_add_rsp) || (*reinterpret_cast <uint16_t*> (fn) == 9215 && type == gadget_type::gdt_jmp_rbx)) {

						*stack_size = 0;

						info = reinterpret_cast <unwind_info*> (m_mod + table[idx_iter].UnwindData);
						status = get_stack_size_ignore_fpreg(m_mod, info, stack_size);

						if (status != 0) {

							gadgets++;

							if (*skip >= gadgets) {
								continue;
							}

							*skip = gadgets;

							switch (type) {

							case gadget_type::gdt_add_rsp: {

								m_cfg->gadgets_add_ptr = reinterpret_cast <void*> (fn);
								m_cfg->gadgets_add_size = *stack_size;

								is_found = true;
								*save_index = idx_iter;

							}; break;

							case gadget_type::gdt_jmp_rbx: {

								m_cfg->gadgets_jmp_ptr = reinterpret_cast <void*> (fn);
								m_cfg->gadgets_jmp_size = *stack_size;
								m_cfg->gadgets_jmp_ref = reinterpret_cast <void*> (fn);

								is_found = true;
								*save_index = idx_iter;

							}; break;

							}; break;

						}

					}

				}

				if (is_found) {
					break;
				}

			}

		}

	private:

		uint64_t m_mod = 0;
		unwind_cfg* m_cfg;

	};

}
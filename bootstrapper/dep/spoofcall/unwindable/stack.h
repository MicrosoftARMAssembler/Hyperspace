#pragma once

namespace uwd_private {

#define uwd_max_stack_frames 200

    static uint32_t search_rbp_on_stack(uint64_t mod_base, void* unwind_info_ref, uint32_t* target_stack_off) {

        using namespace uwd_private;

        uint32_t save_stack_off, backup_stack_off;
        uint32_t frame_offsets[uwd_max_stack_frames] = { 0 };

        PRUNTIME_FUNCTION chained_fn;

        uint32_t is_rbp_pushed = FALSE;

        auto info = reinterpret_cast <unwind_info*> (unwind_info_ref);
        auto code = reinterpret_cast <unwind_code*> (info->code);

        uint32_t frame_sz = 0, node_idx = 0, code_cnt = info->code_cnt;

        save_stack_off = 0;
        *target_stack_off = 0;
        backup_stack_off = *target_stack_off;

        // Initialise context
        context ctx;
        memset(&ctx, 0, sizeof(context));

        while (node_idx < code_cnt) {

            // Ensure frameSize is reset
            frame_sz = 0;

            switch (code->unwind_opcode) {

            case UWOP_PUSH_NONVOL: { // 0

                if (code->opcode_info == RSP) {

                    // We break here
                    return 0;

                }

                if (code->opcode_info == RBP && is_rbp_pushed) {
                    return 0;
                }
                else if (code->opcode_info == RBP) {

                    save_stack_off = *target_stack_off;
                    is_rbp_pushed = 1;

                }

                *target_stack_off += 8;

            }; break;

            case UWOP_ALLOC_LARGE: { // 1

                // If the operation info equals 0 -> allocation size / 8 in next slot
                // If the operation info equals 1 -> unscaled allocation size in next 2 slots
                // In any case, we need to advance 1 slot and record the size

                // Skip to next Unwind Code
                code = reinterpret_cast <unwind_code*> (reinterpret_cast <uint16_t*> (code) + 1);

                // Keep track of current node
                node_idx++;

                // Register size in next slot
                frame_sz = code->frame_off;

                if (code->opcode_info == 0) {

                    // If the operation info equals 0, then the size of the allocation divided by 8 
                    // is recorded in the next slot, allowing an allocation up to 512K - 8.
                    // We already advanced of 1 slot, and recorded the allocation size
                    // We just need to multiply it for 8 to get the unscaled allocation size
                    frame_sz *= 8;

                }
                else {

                    // If the operation info equals 1, then the unscaled size of the allocation is 
                    // recorded in the next two slots in little-endian format, allowing allocations 
                    // up to 4GB - 8.
                    // Skip to next Unwind Code
                    code = reinterpret_cast <unwind_code*> (reinterpret_cast <uint16_t*> (code) + 1);

                    // Keep track of current node
                    node_idx++;

                    // Unmask the rest of the allocation size
                    frame_sz += code->frame_off << 16;

                }

                *target_stack_off += frame_sz;

            }; break;

            case UWOP_ALLOC_SMALL: { // 2

                // Allocate a small-sized area on the stack. The size of the allocation is the operation 
                // info field * 8 + 8, allowing allocations from 8 to 128 bytes.
                *target_stack_off += 8 * (code->opcode_info + 1);

            };  break;


            case UWOP_SET_FPREG: { // 3

                return 0;

            }; break; // EARLY RET

            case UWOP_SAVE_NONVOL: { // 4

                // Save a nonvolatile integer register on the stack using a MOV instead of a PUSH. This code is 
                // primarily used for shrink-wrapping, where a nonvolatile register is saved to the stack in a position 
                // that was previously allocated. The operation info is the number of the register. The scaled-by-8 
                // stack offset is recorded in the next unwind operation code slot, as described in the note above.

                if (code->opcode_info == RSP) {
                    // This time, we return only if RSP was saved
                    return 0;
                }
                else {

                    // For future use: save the scaled by 8 stack offset
                    *((uint32_t*)&ctx + code->opcode_info) = *target_stack_off + static_cast <uint32_t> ((reinterpret_cast <unwind_code*> (reinterpret_cast <uint16_t*> (code) + 1))->frame_off * 8);

                    // Skip to next Unwind Code
                    code = reinterpret_cast <unwind_code*> (reinterpret_cast <uint16_t*> (code) + 1);
                    node_idx++;

                    if (code->opcode_info != RBP) {

                        // Restore original stack size (!?)
                        *target_stack_off = backup_stack_off;

                        break;
                    }

                    if (is_rbp_pushed) {
                        return 0;
                    }

                    is_rbp_pushed = TRUE;

                    // We save the stack offset where MOV [RSP], RBP happened
                    // During unwinding, this address will be popped back in RBP
                    save_stack_off = *((uint32_t*)&ctx + code->opcode_info);

                    // Restore original stack size (!?)
                    *target_stack_off = backup_stack_off;

                }

            }; break;

            case UWOP_SAVE_NONVOL_BIG: { // 5

                // Save a nonvolatile integer register on the stack with a long offset, using a MOV instead of a PUSH. 
                // This code is primarily used for shrink-wrapping, where a nonvolatile register is saved to the stack 
                // in a position that was previously allocated. The operation info is the number of the register. 
                // The unscaled stack offset is recorded in the next two unwind operation code slots, as described 
                // in the note above.

                if (code->opcode_info == RSP) {
                    return 0;
                }

                // For future use
                *((uint32_t*)&ctx + code->opcode_info) = *target_stack_off + static_cast <uint32_t> ((reinterpret_cast <unwind_code*> (reinterpret_cast <uint16_t*> (code) + 1))->frame_off);
                *((uint32_t*)&ctx + code->opcode_info) += static_cast <uint32_t> ((reinterpret_cast <unwind_code*> (reinterpret_cast <uint16_t*> (code) + 2))->frame_off << 16);

                if (code->opcode_info != RBP) {
                    // Restore original stack size (!?)
                    *target_stack_off = backup_stack_off;
                    break;
                }

                if (is_rbp_pushed) {
                    return 0;
                }

                // We save the stack offset where MOV [RSP], RBP happened
                // During unwinding, this address will be popped back in RBP
                save_stack_off = *((uint32_t*)&ctx + code->opcode_info);

                // Restore Stack Size
                *target_stack_off = backup_stack_off;

                // Skip the other two nodes used for this unwind operation
                code = reinterpret_cast <unwind_code*> (reinterpret_cast <uint16_t*> (code) + 2);
                node_idx += 2;

            }; break;

            case UWOP_EPILOG:            // 6
            case UWOP_SAVE_XMM128: {      // 8

                // Save all 128 bits of a nonvolatile XMM register on the stack. The operation info is the number of 
                // the register. The scaled-by-16 stack offset is recorded in the next slot.

                // TODO: Handle this

                // Skip to next Unwind Code
                code = reinterpret_cast <unwind_code*> (reinterpret_cast <uint16_t*> (code) + 1);
                node_idx++;

            };  break;

            case UWOP_SPARE_CODE:        // 7
            case UWOP_SAVE_XMM128BIG: {    // 9

                // Save all 128 bits of a nonvolatile XMM register on the stack with a long offset. The operation info 
                // is the number of the register. The unscaled stack offset is recorded in the next two slots.

                // TODO: Handle this

                // Advancing next 2 nodes
                code = reinterpret_cast <unwind_code*> (reinterpret_cast <uint16_t*> (code) + 2);
                node_idx += 2;

            }; break;

            case UWOP_PUSH_MACH_FRAME: {    // 10

                // Push a machine frame. This unwind code is used to record the effect of a hardware interrupt or exception. 
                // There are two forms.

                // NOTE: UNTESTED
                // TODO: Test this
                if (code->opcode_info == 0) {
                    *target_stack_off += 0x40;
                }
                else {
                    *target_stack_off += 0x48;
                }

            }; break;

            }

            code = reinterpret_cast <unwind_code*> (reinterpret_cast <uint16_t*> (code) + 1);
            node_idx++;

        }

        // If chained unwind information is present then we need to
        // also recursively parse this and add to total stack size.
        if (unwind_get_chain_info(info->flags)) {

            node_idx = info->code_cnt;

            if (0 != (node_idx & 1)) {
                node_idx += 1;
            }

            chained_fn = reinterpret_cast <PRUNTIME_FUNCTION> (&info->code[node_idx]);
            return search_rbp_on_stack(mod_base, reinterpret_cast <unwind_info*> (mod_base + chained_fn->UnwindData), target_stack_off);
        }

        return save_stack_off;
    }

    static uint32_t get_stack_size_ignore_fpreg(uint64_t mod_base, void* unwind_info_ref, uint32_t* target_stack_off) {

        using namespace uwd_private;

        uint32_t save_stack_off, backup_stack_off;
        uint32_t frame_offsets[uwd_max_stack_frames] = { 0 };

        PRUNTIME_FUNCTION chained_fn;

        auto info = reinterpret_cast <unwind_info*> (unwind_info_ref);
        auto code = reinterpret_cast <unwind_code*> (info->code);

        uint32_t frame_sz = 0, node_idx = 0, code_cnt = info->code_cnt;

        save_stack_off = 0;
        *target_stack_off = 0;
        backup_stack_off = *target_stack_off;

        // Initialise context
        context ctx;
        memset(&ctx, 0, sizeof(context));

        while (node_idx < code_cnt) {

            // Ensure frameSize is reset
            frame_sz = 0;

            switch (code->unwind_opcode) {

            case UWOP_PUSH_NONVOL: { // 0

                if (code->opcode_info == RSP) {

                    // We break here
                    return 0;

                }

                *target_stack_off += 8;

            };  break;

            case UWOP_ALLOC_LARGE: { // 1

                // If the operation info equals 0 -> allocation size / 8 in next slot
                // If the operation info equals 1 -> unscaled allocation size in next 2 slots
                // In any case, we need to advance 1 slot and record the size

                // Skip to next Unwind Code
                code = reinterpret_cast <unwind_code*> (reinterpret_cast <uint16_t*> (code) + 1);

                // Keep track of current node
                node_idx++;

                // Register size in next slot
                frame_sz = code->frame_off;

                if (code->opcode_info == 0) {

                    // If the operation info equals 0, then the size of the allocation divided by 8 
                    // is recorded in the next slot, allowing an allocation up to 512K - 8.
                    // We already advanced of 1 slot, and recorded the allocation size
                    // We just need to multiply it for 8 to get the unscaled allocation size
                    frame_sz *= 8;

                }
                else {

                    // If the operation info equals 1, then the unscaled size of the allocation is 
                    // recorded in the next two slots in little-endian format, allowing allocations 
                    // up to 4GB - 8.
                     // Skip to next Unwind Code
                    code = reinterpret_cast <unwind_code*> (reinterpret_cast <uint16_t*> (code) + 1);

                    // Keep track of current node
                    node_idx++;

                    // Unmask the rest of the allocation size
                    frame_sz += code->frame_off << 16;

                }

                *target_stack_off += frame_sz;

            }; break;

            case UWOP_ALLOC_SMALL: { // 2

                // Allocate a small-sized area on the stack. The size of the allocation is the operation 
                // info field * 8 + 8, allowing allocations from 8 to 128 bytes.
                *target_stack_off += 8 * (code->opcode_info + 1);

            }; break;


            case UWOP_SET_FPREG: { // 3

                // IGNORED

            }; break;

            case UWOP_SAVE_NONVOL: { // 4

                // Save a nonvolatile integer register on the stack using a MOV instead of a PUSH. This code is 
                // primarily used for shrink-wrapping, where a nonvolatile register is saved to the stack in a position 
                // that was previously allocated. The operation info is the number of the register. The scaled-by-8 
                // stack offset is recorded in the next unwind operation code slot, as described in the note above.

                if (code->opcode_info == RSP) {

                    // This time, we return only if RSP was saved
                    return 0;

                }
                else {

                    // For future use: save the scaled by 8 stack offset
                    *((uint32_t*)&ctx + code->opcode_info) = *target_stack_off + static_cast <uint32_t> ((reinterpret_cast <unwind_code*> (reinterpret_cast <uint16_t*> (code) + 1))->frame_off * 8);

                    // Skip to next Unwind Code
                    code = reinterpret_cast <unwind_code*> (reinterpret_cast <uint16_t*> (code) + 1);
                    node_idx++;

                    // We save the stack offset where MOV [RSP], RBP happened
                    // During unwinding, this address will be popped back in RBP
                    save_stack_off = *((uint32_t*)&ctx + code->opcode_info);

                    // Restore original stack size (!?)
                    *target_stack_off = backup_stack_off;
                }

            };  break;

            case UWOP_SAVE_NONVOL_BIG: { // 5

                // Save a nonvolatile integer register on the stack with a long offset, using a MOV instead of a PUSH. 
                // This code is primarily used for shrink-wrapping, where a nonvolatile register is saved to the stack 
                // in a position that was previously allocated. The operation info is the number of the register. 
                // The unscaled stack offset is recorded in the next two unwind operation code slots, as described 
                // in the note above.

                if (code->opcode_info == RSP) {
                    return 0;
                }

                // For future use
                *((uint32_t*)&ctx + code->opcode_info) = *target_stack_off + static_cast <uint32_t> ((reinterpret_cast <unwind_code*> (reinterpret_cast <uint16_t*> (code) + 1))->frame_off);
                *((uint32_t*)&ctx + code->opcode_info) += static_cast <uint32_t> ((reinterpret_cast <unwind_code*> (reinterpret_cast <uint16_t*> (code) + 2))->frame_off) << 16;

                // We save the stack offset where MOV [RSP], RBP happened
                // During unwinding, this address will be popped back in RBP
                save_stack_off = *((uint32_t*)&ctx + code->opcode_info);

                // Restore Stack Size
                *target_stack_off = backup_stack_off;

                // Skip the other two nodes used for this unwind operation
                code = reinterpret_cast <unwind_code*> (reinterpret_cast <uint16_t*> (code) + 2);
                node_idx += 2;

            }; break;

            case UWOP_EPILOG:            // 6
            case UWOP_SAVE_XMM128: {      // 8

                // Save all 128 bits of a nonvolatile XMM register on the stack. The operation info is the number of 
                // the register. The scaled-by-16 stack offset is recorded in the next slot.

                // TODO: Handle this

                // Skip to next Unwind Code
                code = reinterpret_cast <unwind_code*> (reinterpret_cast <uint16_t*> (code) + 1);
                node_idx++;

            }; break;

            case UWOP_SPARE_CODE:        // 7
            case UWOP_SAVE_XMM128BIG: {    // 9

                // Save all 128 bits of a nonvolatile XMM register on the stack with a long offset. The operation info 
                // is the number of the register. The unscaled stack offset is recorded in the next two slots.

                // TODO: Handle this

                // Advancing next 2 nodes
                code = reinterpret_cast <unwind_code*> (reinterpret_cast <uint16_t*> (code) + 2);
                node_idx += 2;

            }; break;

            case UWOP_PUSH_MACH_FRAME: {    // 10

                // Push a machine frame. This unwind code is used to record the effect of a hardware interrupt or exception. 
                // There are two forms. 

                // NOTE: UNTESTED
                // TODO: Test this
                if (code->opcode_info == 0) {
                    *target_stack_off += 0x40;
                }
                else {
                    *target_stack_off += 0x48;
                }

            }; break;
            }

            code = reinterpret_cast <unwind_code*> (reinterpret_cast <uint16_t*> (code) + 1);
            node_idx++;

        }

        // If chained unwind information is present then we need to
        // also recursively parse this and add to total stack size.
        if (unwind_get_chain_info(info->flags)) {

            node_idx = info->code_cnt;

            if (0 != (node_idx & 1)) {
                node_idx += 1;
            }

            chained_fn = reinterpret_cast <PRUNTIME_FUNCTION> (&info->code[node_idx]);
            return get_stack_size_ignore_fpreg(mod_base, reinterpret_cast <unwind_info*> (mod_base + chained_fn->UnwindData), target_stack_off);
        }

        return *target_stack_off;
    }

    static uint32_t get_stack_frame_size(uint64_t mod_base, void* unwind_info_ref, uint32_t* target_stack_off) {

        using namespace uwd_private;

        uint64_t save_id;
        uint32_t frame_offsets[uwd_max_stack_frames] = { 0 };

        PRUNTIME_FUNCTION chained_fn;

        uint16_t _fo;

        uint32_t frame_sz = 0, node_idx = 0;
        uint32_t is_uwop_set_fpreg_hit = FALSE;

        auto info = reinterpret_cast <unwind_info*> (unwind_info_ref);
        auto code = reinterpret_cast <unwind_code*> (info->code);

        // Restore Stack Size
        *target_stack_off = 0;

        // Initialise context
        context ctx;
        memset(&ctx, 0, sizeof(context));

        while (node_idx < info->code_cnt) {

            // Ensure frameSize is reset
            frame_sz = 0;

            switch (code->unwind_opcode) {

            case UWOP_PUSH_NONVOL: { // 0

                if (code->opcode_info == RSP && !is_uwop_set_fpreg_hit) {

                    // We break here
                    return 0;
                }

                *target_stack_off += 8;

            }; break;

            case UWOP_ALLOC_LARGE: { // 1

                // If the operation info equals 0 -> allocation size / 8 in next slot
                // If the operation info equals 1 -> unscaled allocation size in next 2 slots
                // In any case, we need to advance 1 slot and record the size

                // Skip to next Unwind Code
                code = reinterpret_cast <unwind_code*> (reinterpret_cast <uint16_t*> (code) + 1);

                // Keep track of current node
                node_idx++;

                // Register size in next slot
                frame_sz = code->frame_off;

                if (code->opcode_info == 0) {

                    // If the operation info equals 0, then the size of the allocation divided by 8 
                    // is recorded in the next slot, allowing an allocation up to 512K - 8.
                    // We already advanced of 1 slot, and recorded the allocation size
                    // We just need to multiply it for 8 to get the unscaled allocation size
                    frame_sz *= 8;

                }
                else {

                    // If the operation info equals 1, then the unscaled size of the allocation is 
                    // recorded in the next two slots in little-endian format, allowing allocations 
                    // up to 4GB - 8.
                    // Skip to next Unwind Code
                    code = reinterpret_cast <unwind_code*> (reinterpret_cast <uint16_t*> (code) + 1);

                    // Keep track of current node
                    node_idx++;

                    // Unmask the rest of the allocation size
                    frame_sz += code->frame_off << 16;

                }

                *target_stack_off += frame_sz;

            }; break;

            case UWOP_ALLOC_SMALL: { // 2

                // Allocate a small-sized area on the stack. The size of the allocation is the operation 
                // info field * 8 + 8, allowing allocations from 8 to 128 bytes.
                *target_stack_off += 8 * (code->opcode_info + 1);

            }; break;


            case UWOP_SET_FPREG: { // 3

                // Establish the frame pointer register by setting the register to some offset of the current RSP. 
                // The offset is equal to the Frame Register offset (scaled) field in the UNWIND_INFO * 16, allowing 
                // offsets from 0 to 240. The use of an offset permits establishing a frame pointer that points to the
                // middle of the fixed stack allocation, helping code density by allowing more accesses to use short 
                // instruction forms. The operation info field is reserved and shouldn't be used.


                if (unwind_get_e_handler(info->flags) && unwind_get_chain_info(info->flags)) {
                    return 0;
                }

                is_uwop_set_fpreg_hit = TRUE;

                frame_sz = -0x10 * (info->frame_off);
                *target_stack_off += frame_sz;

            }; break;

            case UWOP_SAVE_NONVOL: { // 4

                // Save a nonvolatile integer register on the stack using a MOV instead of a PUSH. This code is 
                // primarily used for shrink-wrapping, where a nonvolatile register is saved to the stack in a position 
                // that was previously allocated. The operation info is the number of the register. The scaled-by-8 
                // stack offset is recorded in the next unwind operation code slot, as described in the note above.
                if (code->opcode_info == RBP || code->opcode_info == RSP) {
                    return 0;
                }

                // Skip to next Unwind Code
                code = reinterpret_cast <unwind_code*> (reinterpret_cast <uint16_t*> (code) + 1);
                node_idx++;

                // For future use
                *((uint32_t*)&ctx + code->opcode_info) = *target_stack_off + static_cast <uint32_t> (reinterpret_cast <unwind_code*> (reinterpret_cast <uint16_t*> (code) + 1)->frame_off * 8);

            }; break;

            case UWOP_SAVE_NONVOL_BIG: { // 5

                // Save a nonvolatile integer register on the stack with a long offset, using a MOV instead of a PUSH. 
                // This code is primarily used for shrink-wrapping, where a nonvolatile register is saved to the stack 
                // in a position that was previously allocated. The operation info is the number of the register. 
                // The unscaled stack offset is recorded in the next two unwind operation code slots, as described 
                // in the note above.
                if (code->opcode_info == RBP || code->opcode_info == RSP) {
                    return 0;
                }

                // For future use
                *((uint32_t*)&ctx + code->opcode_info) = *target_stack_off + static_cast <uint32_t> ((reinterpret_cast <unwind_code*> (reinterpret_cast <uint16_t*> (code) + 1))->frame_off);
                *((uint32_t*)&ctx + code->opcode_info) += static_cast <uint32_t> ((reinterpret_cast <unwind_code*> (reinterpret_cast <uint16_t*> (code) + 2))->frame_off << 16);

                // Skip the other two nodes used for this unwind operation
                code = reinterpret_cast <unwind_code*> (reinterpret_cast <uint16_t*> (code) + 2);
                node_idx += 2;

            }; break;

            case UWOP_EPILOG:            // 6
            case UWOP_SAVE_XMM128: {      // 8

                // Save all 128 bits of a nonvolatile XMM register on the stack. The operation info is the number of 
                // the register. The scaled-by-16 stack offset is recorded in the next slot.

                // TODO: Handle this

                // Skip to next Unwind Code
                code = reinterpret_cast <unwind_code*> (reinterpret_cast <uint16_t*> (code) + 1);
                node_idx++;

            }; break;

            case UWOP_SPARE_CODE:        // 7
            case UWOP_SAVE_XMM128BIG: {    // 9
                // Save all 128 bits of a nonvolatile XMM register on the stack with a long offset. The operation info 
                // is the number of the register. The unscaled stack offset is recorded in the next two slots.

                // TODO: Handle this

                // Advancing next 2 nodes
                code = reinterpret_cast <unwind_code*> (reinterpret_cast <uint16_t*> (code) + 2);
                node_idx += 2;

            }; break;

            case UWOP_PUSH_MACH_FRAME: {    // 10
                // Push a machine frame. This unwind code is used to record the effect of a hardware interrupt or exception. 
                // There are two forms.

                // NOTE: UNTESTED
                // TODO: Test this
                if (code->opcode_info == 0) {
                    *target_stack_off += 0x40;
                }
                else {
                    *target_stack_off += 0x48;
                }

            }; break;
            }

            code = reinterpret_cast <unwind_code*> (reinterpret_cast <uint16_t*> (code) + 1);
            node_idx++;

        }

        // If chained unwind information is present then we need to
        // also recursively parse this and add to total stack size.
        if (unwind_get_chain_info(info->flags)) {

            node_idx = info->code_cnt;

            if (0 != (node_idx & 1)) {
                node_idx += 1;
            }


            chained_fn = reinterpret_cast <PRUNTIME_FUNCTION> (&info->code[node_idx]);
            return get_stack_frame_size(mod_base, reinterpret_cast <unwind_info*> (mod_base + chained_fn->UnwindData), target_stack_off);
        }

        return is_uwop_set_fpreg_hit;

    }

}
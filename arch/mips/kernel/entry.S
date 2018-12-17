#include "asm.h"

.equ    KERNEL, 0
.equ    USER,   156

#开中断
.macro STI
    mfc0    k0, CP0_STATUS
    ori     k0, 0x1
    mtc0    k0, CP0_STATUS	
.endm
#关中断
.macro CLI
    mfc0    k0, CP0_STATUS
    and     k0, ~0x1
    mtc0    k0, CP0_STATUS
.endm

LEAF(asm_start)
    mtc0    zero, CP0_STATUS
    mtc0    zero, CP0_WATCHLO
    mtc0    zero, CP0_WATCHHI

    mfc0    t0, CP0_CONFIG
    and     t0, ~0x7
    ori     t0, 0x2
    mtc0    t0, CP0_CONFIG
    
    jr  ra
	nop
END(asm_start)

.macro SAVE_CONTEXT offset
	lw     k0, current_running
	addiu  k0, k0, \offset	
	#sw AT, 4(k0)
	sw v0, 8(k0)
	sw v1, 12(k0)
	sw a0, 16(k0)
	sw a1, 20(k0)
	sw a2, 24(k0)
	sw a3, 28(k0)
	sw t0, 32(k0)
	sw t1, 36(k0)
	sw t2, 40(k0)
	sw t3, 44(k0)
	sw t4, 48(k0)
	sw t5, 52(k0)
	sw t6, 56(k0)
	sw t7, 60(k0)

	sw t8, 96(k0)
	sw t9, 100(k0)

	sw s0, 64(k0)
	sw s1, 68(k0)
	sw s2, 72(k0)
	sw s3, 76(k0)
	sw s4, 80(k0)
	sw s5, 84(k0)
	sw s6, 88(k0)
	sw s7, 92(k0)
	
	sw s8, 120(k0)
	
	sw gp, 112(k0)
	
	sw sp, 116(k0)
	
	sw fp, 120(k0)

	sw ra, 124(k0)
	
	mfhi k1
	sw k1, 132(k0)
	mflo k1
	sw k1, 136(k0)

	mfc0 k1, CP0_BADVADDR
	sw k1, 140(k0)

	mfc0 k1, CP0_EPC
	sw k1, 148(k0)
.endm

.macro RESTORE_CONTEXT offset
	lw     k0, current_running
	addiu  k0, k0, \offset
	#lw AT, 4(k0)
	lw v0, 8(k0)
	lw v1, 12(k0)
	lw a0, 16(k0)
	lw a1, 20(k0)
	lw a2, 24(k0)
	lw a3, 28(k0)
	
	lw t0, 32(k0)
	lw t1, 36(k0)
	lw t2, 40(k0)
	lw t3, 44(k0)
	lw t4, 48(k0)
	lw t5, 52(k0)
	lw t6, 56(k0)
	lw t7, 60(k0)
	
	lw t8, 96(k0)
	lw t9, 100(k0)
	
	lw s0, 64(k0)
	lw s1, 68(k0)
	lw s2, 72(k0)
	lw s3, 76(k0)
	lw s4, 80(k0)
	lw s5, 84(k0)
	lw s6, 88(k0)
	lw s7, 92(k0)
	
	lw s8, 120(k0)
	
	lw sp, 116(k0)
	
	lw gp, 112(k0)
	
	lw fp, 120(k0)
	
	lw ra, 124(k0)
	
	lw k1, 132(k0)
	mthi k1
	lw k1, 136(k0)
	mtlo k1

	lw k1, 140(k0)
	mtc0 k1, CP0_BADVADDR

	lw k1, 148(k0)
	mtc0 k1, CP0_EPC
.endm

NESTED(init_scheduler, 0, ra)
	#!!!!!!!!!!!!!!!注意如果上下文加入status之类的，需要赋初值！！！
    jal     scheduler
	nop
    RESTORE_CONTEXT(USER)
	jr      ra
	nop
END(init_scheduler)

NESTED(do_scheduler, 0, ra)
    SAVE_CONTEXT(USER)
    jal     scheduler
	nop
    RESTORE_CONTEXT(USER)
    jr      ra
	nop
END(do_scheduler)

LEAF(reset_CPCOUNT)
    mtc0    zero, CP0_COUNT
	li      k0, 0x2000000
    mtc0    k0, CP0_COMPARE
    jr  ra
END(reset_CPCOUNT)

LEAF(copy_code)
    CLI
	
	li      t2, 0x0
	la      t4, exception_handler_end
loop:
	la      t0, exception_handler_begin
	li      t1, 0x80000180
	add     t0, t0, t2
	add     t1, t1, t2
	lw      t3, 0(t0)
	sw      t3, 0(t1)
	add     t2, t2, 0x4
	ble     t0, t4, loop
	nop

	jr      ra
	nop
END(copy_code)

NESTED(enable_interrupt, 0, ra)
    mtc0    zero, CP0_COUNT
	li      k0, 0x5000000
    mtc0    k0, CP0_COMPARE

    mfc0    k0, CP0_STATUS
    ori     k0, 0xff01
    mtc0    k0, CP0_STATUS	
    jr  ra
END(enable_interrupt)

.global exception_handler_begin
.global exception_handler_end

NESTED(exception_handler_entry, 0, sp)   
exception_handler_begin:
	#关中断
    CLI
	#保存现场
    SAVE_CONTEXT(USER)
	#切换到内核栈
	li      sp, 0xa0f00000

	mfc0    k0, CP0_CAUSE
	and     k0, 0x7c
	beqz    k0, 1f	#判断是否为中断，若是则跳转，否则继续执行

	and     k0, 0xff
	li      k1, 0x20
	beq     k0, k1, 2f	#判断是否为系统调用，若是则跳转，否则继续执行

	and     k0, 0xff
	li      k1, 0x8
	beq     k0, k1, 3f	#判断是否为tlb invalid，若是则跳转，否则继续执行
	li      k1, 0x0c
	beq     k0, k1, 3f	#判断是否为tlb invalid，若是则跳转，否则继续执行

	RESTORE_CONTEXT(USER)
    mtc0    zero, CP0_COUNT
	li      k0, 0x2000000
    mtc0    k0, CP0_COMPARE
	#开中断
    STI
	eret
	nop
1:
	jal     handle_int
	nop
2:
	li      k1, 0x4
    mfc0    k0, CP0_EPC
	add     k0, k0, k1
	mtc0    k0, CP0_EPC
    SAVE_CONTEXT(USER)
	jal     handle_syscall
	nop
3:
	jal     handle_tlb_refill
	nop    
exception_handler_end:
END(exception_handler_entry)

NESTED(handle_int, 0, sp)

	mfc0    a0, CP0_STATUS
	mfc0    a1, CP0_CAUSE	
	jal interrupt_helper

    RESTORE_CONTEXT(USER)

	#恢复计时器
    mtc0    zero, CP0_COUNT
	li      k0, 0x400000
    mtc0    k0, CP0_COMPARE

    mfc0    k0, CP0_CAUSE
    and     k0, 0xFFFFFF00
    mtc0    k0, CP0_CAUSE	

	#开中断
    STI
	eret
END(handle_int)

NESTED(handle_syscall, 0, sp)

	jal     system_call_helper

    RESTORE_CONTEXT(USER)

	#开中断
    STI
	eret
END(handle_syscall)

NESTED(handle_other, 0, sp)
    // other exception handler
END(handle_other)


LEAF(get_cp0_status)
    mfc0  v0, CP0_STATUS
    jr ra
END(get_cp0_status)

LEAF(set_cp0_status)
    mtc0  a0, CP0_STATUS
    jr ra
END(set_cp0_status)

LEAF(get_C0_ENHI)
    mfc0  v0, CP0_ENTRYHI
    jr ra
END(get_C0_ENHI)

LEAF(set_C0_ENHI)
    mtc0  a0, CP0_ENTRYHI
    jr ra
END(set_C0_ENHI)

LEAF(set_C0_ENLO0)
    mtc0  a0, CP0_ENTRYLO0
    jr ra
END(set_C0_ENLO0)

LEAF(set_C0_ENLO1)
    mtc0  a0, CP0_ENTRYLO1
    jr ra
END(set_C0_ENLO1)

LEAF(set_C0_PAGEMASK)
    mtc0  a0, CP0_PAGEMASK
    jr ra
END(set_C0_PAGEMASK)

LEAF(set_C0_INDEX)
    mtc0  a0, CP0_INDEX
    jr ra
END(set_C0_INDEX)

LEAF(set_tlb)
	tlbwi
    jr ra
END(set_tlb)

LEAF(set_tlbwr)
	tlbwr
    jr ra
END(set_tlbwr)

LEAF(get_index)
	mfc0 v0, CP0_INDEX
    jr ra
END(get_index)

LEAF(find_tlbp)
	mtc0 a0, CP0_ENTRYHI
	tlbp
    jr ra
END(find_tlbp)


LEAF(copy_tlb_code)	
	li      t2, 0x0
	la      t4, tlb_handler_end
looptlb:
	la      t0, tlb_handler_begin
	li      t1, 0x80000000
	add     t0, t0, t2
	add     t1, t1, t2
	lw      t3, 0(t0)
	sw      t3, 0(t1)
	add     t2, t2, 0x4
	ble     t0, t4, looptlb
	nop

	jr      ra
	nop
END(copy_tlb_code)

.global tlb_handler_begin
.global tlb_handler_end

NESTED(tlb_handler_entry, 0, sp)   
tlb_handler_begin:
	#关中断
    CLI
	#保存现场
    SAVE_CONTEXT(USER)
	#切换到内核栈
	li      sp, 0xa0f00000

	jal     handle_tlb_refill
	nop

tlb_handler_end:
END(tlb_handler_entry)

NESTED(handle_tlb_refill, 0, sp)

	mfc0 a0, CP0_CONTEXT
	jal do_TLB_Refill

    RESTORE_CONTEXT(USER)

	#开中断
    STI
	eret
END(handle_tlb_refill)

.text
.global main

main:
	li $t2, 0x0
loop:
	li $t0, 0xa0800000
	li $t1, 0xa080fff0
	add $t0, $t0, $t2
	add $t1, $t1, $t2
	lw $t3, 0($t0)
	sw $t3, 0($t1)
	add $t2, $t2, 0x4
	ble $t2,0x200,loop 
	#不一定是0xa0800800，此位置可以自由指定，只要保证不超过可用范围以及在kernel占据的扇区之外
	#li $a0, 0xa0809000	
    #li $a1, 0x000
    #li $a2, 0x200
	#jal 0x8007b1cc
	beq $a1, $a1, 0xfff0

    la  $a0, msg
	jal 0x8007b980
	# 1) task1 call BIOS print string "It's bootblock!"
	lw  $t0, 0xa08001fc
	li  $t1, 0x200
	mult $t0, $t1
	mflo $a2
	li $a0, 0xa0800000	
    li $a1, 0x200
   
	jal 0x8007b1cc


	jal 0xa0800000
	# 2) task2 call BIOS read kernel in SD card and jump to kernel start

# while(1) --> stop here
stop:
	j stop

.data

msg: .asciiz "It's a bootloader...\n"

# 1. PMON read SD card function address
# read_sd_card();
read_sd_card: .word 0x8007b1cc

# 2. PMON print string function address
# printstr(char *string)
printstr: .word 0x8007b980

# 3. PMON print char function address
# printch(char ch)
printch: .word 0x8007ba00

# 4. kernel address (move kernel to here ~)
kernel : .word 0xa0800000

# 5. kernel main address (jmp here to start kernel main!)
kernel_main : .word 0xa0800000

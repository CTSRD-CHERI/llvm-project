# Check that aligning the .fini section doesn't insert trap instructions

.section .fini,"ax",%progbits
.p2align 4
.set noreorder
sd $gp, 0($sp)
sd $ra, 8($sp)
jr $ra
daddu $sp, $sp, 32
.set reorder

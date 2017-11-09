	.text
	.abicalls
	.set    noreorder
	.globl  too_far
	.ent    too_far
too_far:
	nop
	jr      $ra
	nop
	.end    too_far

# RUN: not %cheri_llvm-mc %s -show-encoding 2>&1 | FileCheck %s

.set noat
  # Error: 16 bit relocation in a 11 bit immediate:

  csc $c1, $zero, %capcall_hi(foo)($c3)
  # CHECK: wrong-size-relocs.s:[[@LINE-1]]:19: error: invalid operand for instruction
  csc $c1, $zero, %capcall_lo(foo)($c3)
  # CHECK: wrong-size-relocs.s:[[@LINE-1]]:19: error: invalid operand for instruction
  csc $c1, $zero, %got(foo)($c3)
  # CHECK: wrong-size-relocs.s:[[@LINE-1]]:19: error: invalid operand for instruction


  # TODO: should also be an error: 11 bit relocation in a 16 bit immediate
  lui	$1, %capcall(foo)
  ori	$1, $1, %captab(foo)

.data
foo:
.word 1

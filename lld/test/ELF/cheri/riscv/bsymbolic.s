# REQUIRES: riscv
# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %s -o %t.o
# RUN: ld.lld -shared -Bsymbolic %t.o -o %t.so
# RUN: llvm-readobj -r --cap-relocs %t.so | FileCheck %s

## Test that we emit __cap_relocs entries for defined symbols when using
## -Bsymbolic even if they would normally be preemptible.

# CHECK:      Relocations [
# CHECK-NEXT: ]
# CHECK-NEXT: CHERI __cap_relocs [
# CHECK-NEXT:   0x0033d0 (data) Base: 0x33d0 (data+0) Length: 32 Perms: Object
# CHECK-NEXT:   0x0033e0 Base: 0x1320 (func+0) Length: 4 Perms: Function
# CHECK-NEXT: ]

  .type func, @function
  .global func
func:
  ret
  .size func, . - func

  .data
  .type data, @object
  .global data
data:
  .chericap data
  .chericap func
  .size data, . - data

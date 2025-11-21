# REQUIRES: riscv
# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %s -o %t.o
# RUN: ld.lld -shared -Bsymbolic %t.o -o %t.so
# RUN: llvm-readobj -r --cap-relocs %t.so | FileCheck %s

## Test that we emit __cap_relocs entries for defined symbols when using
## -Bsymbolic even if they would normally be preemptible.

# CHECK:      Relocations [
# CHECK-NEXT: ]
# CHECK:      __cap_relocs {
# CHECK-NEXT:   0x3390 DATA - 0x3390 [0x3390-0x33B0]
# CHECK-NEXT:   0x33A0 FUNC - 0x12E8 [0x12E8-0x12EC]
# CHECK-NEXT: }

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

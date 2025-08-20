# REQUIRES: riscv

# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %s -o %t.o
# RUN: ld.lld -shared -z separate-code %t.o -o %t.so
# RUN: llvm-readobj -r --cap-relocs %t.so | FileCheck %s

# CHECK:      .rela.dyn {
# CHECK-NEXT:   0x3100 R_RISCV_FUNC_RELATIVE - 0x1000
# CHECK-NEXT:   0x3108 R_RISCV_RELATIVE - 0x30D0
# CHECK-NEXT: }
# CHECK:      CHERI __cap_relocs [
# CHECK-NEXT:   0x0030d0 (data) Base: 0x1000 (func+0) Length: 4 Perms: Function
# CHECK-NEXT:   0x0030e0 Base: 0x1000 (func+0) Length: 4 Perms: Code
# CHECK-NEXT:   0x0030f0 Base: 0x30d0 (data+0) Length: 64 Perms: Object
# CHECK-NEXT: ]

  .type func, @function
func:
  ret
  .size func, . - func

  .data
  .type data, @object
data:
  .chericap func
  .chericap %code(func)
  .chericap data
  .quad func
  .quad data
  .size data, . - data

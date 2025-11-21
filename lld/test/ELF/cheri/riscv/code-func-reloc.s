# REQUIRES: riscv

# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %s -o %t.o
# RUN: ld.lld -shared -z separate-code %t.o -o %t.so
# RUN: llvm-readobj -r --cap-relocs %t.so | FileCheck %s

# CHECK:      .rela.dyn {
# CHECK-NEXT:   0x3100 R_RISCV_FUNC_RELATIVE - 0x1000
# CHECK-NEXT:   0x3108 R_RISCV_RELATIVE - 0x30D0
# CHECK-NEXT: }
# CHECK:      __cap_relocs {
# CHECK-NEXT:   0x30D0 FUNC - 0x1000 [0x1000-0x1004]
# CHECK-NEXT:   0x30E0 CODE - 0x1000 [0x1000-0x1004]
# CHECK-NEXT:   0x30F0 DATA - 0x30D0 [0x30D0-0x3110]
# CHECK-NEXT: }

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

# REQUIRES: riscv
# RUN: split-file %s %t

# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %t/lib.s -o %t/lib.o
# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %t/prog.s -o %t/prog.o
# RUN: ld.lld -shared %t/lib.o -o %t/lib.so
# RUN: not ld.lld %t/prog.o %t/lib.so -o %t/prog 2>&1 | FileCheck %s

# CHECK:      error: relocation R_RISCV_PCREL_HI20 cannot be used against symbol 'x'; recompile with -fPIC
# CHECK-NEXT: >>> defined in {{.*}}/lib.so
# CHECK-NEXT: >>> referenced by {{.*}}/prog.o:(.text+0x0)

#--- lib.s

  .data
  .global x
  .type x, @object
x:
  .zero 1
  .size x, . - x

#--- prog.s

  .global _start
  .type _start, @function
_start:
  cllc ca0, x

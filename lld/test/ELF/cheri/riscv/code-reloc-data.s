# REQUIRES: riscv

# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %s -o %t.o
# RUN: not ld.lld -shared %t.o -o /dev/null 2>&1 | FileCheck %s

# CHECK: error: code relocation against non-function symbol local object data
# CHECK-NEXT: >>> defined in ({{.*}}/code-reloc-data.s.tmp.o:(data))
# CHECK-NEXT: >>> referenced by local object data
# CHECK-NEXT: >>> defined in ({{.*}}/code-reloc-data.s.tmp.o:(data))

  .data
  .type data, @object
data:
  .chericap %code(data)
  .size data, . - data

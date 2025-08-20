# REQUIRES: riscv

# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %s -o %t.o
# RUN: not ld.lld -shared %t.o -o /dev/null 2>&1 | FileCheck %s

# CHECK: error: relocation R_RISCV_CHERI_CAPABILITY_CODE cannot be used against preemptible symbol 'func'
# CHECK-NEXT: >>> defined in {{.*}}/code-reloc-preemptible.s.tmp.o
# CHECK-NEXT: >>> referenced by {{.*}}/code-reloc-preemptible.s.tmp.o:(data)

  .global func
  .type func, @function
func:
  ret
  .size func, . - func

  .data
  .type data, @object
data:
  .chericap %code(func)
  .size data, . - data

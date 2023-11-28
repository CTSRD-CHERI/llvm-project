
// RUN: %riscv32_cheri_purecap_cc1 -emit-llvm -o - %s | FileCheck %s
// RUN: %riscv64_cheri_purecap_cc1 -emit-llvm -o - %s | FileCheck %s

// RUN: %riscv32_cheri_cc1 -fsyntax-only -verify %s
// RUN: %riscv64_cheri_cc1 -fsyntax-only -verify %s
extern int foo;

// expected-warning@+1 {{unknown attribute 'nocaprelocs' ignored}}
void __attribute__((nocaprelocs)) bar(){
  int * y = &foo;
}

// CHECK: attributes #0 = { {{.*}}nocaprelocs{{.*}}}

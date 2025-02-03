
// RUN: %riscv32_cheri_purecap_cc1 -emit-llvm -o - %s | FileCheck %s
// RUN: %riscv64_cheri_purecap_cc1 -emit-llvm -o - %s | FileCheck %s

// RUN: %riscv32_cheri_cc1 -fsyntax-only -verify %s
// RUN: %riscv64_cheri_cc1 -fsyntax-only -verify %s

// expected-warning@+1 {{unknown attribute 'cheri_dont_seal' ignored}}
void __attribute__((cheri_dont_seal)) bar(){}

// CHECK: attributes #0 = { {{.*}}dont_seal{{.*}}}

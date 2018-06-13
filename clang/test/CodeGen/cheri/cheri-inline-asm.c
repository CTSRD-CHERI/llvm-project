// RUN: %cheri_purecap_cc1 -emit-llvm -o - %s | FileCheck --check-prefix=RM %s
// RUN: %cheri_purecap_cc1 -emit-llvm -o - %s | FileCheck --check-prefix=R %s
// RUN: %cheri_purecap_cc1 -emit-llvm -o - %s | FileCheck --check-prefix=M %s
void f(char *p) {
  // RM: asm sideeffect "", "C,~{memory},~{$1}"
  asm volatile("" : : "rm"(p) : "memory");
  // R: asm sideeffect "", "C,~{memory},~{$1}"
  asm volatile("" : : "r"(p) : "memory");
  // M: asm sideeffect "", "C,~{memory},~{$1}"
  asm volatile("" : : "m"(p) : "memory");
}

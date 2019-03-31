// REQUIRES: mips-registered-target

// RUN: %cheri_purecap_cc1 -emit-llvm -o - %s | FileCheck %s
// RUN: %cheri_purecap_cc1 -S -o /dev/null %s -verify
void f(char *p) {
  // CHECK: call void asm sideeffect "", "C,~{memory},~{$1}"(i8 addrspace(200)* %{{.+}})
  asm volatile("" : : "rm"(p) : "memory");
  // CHECK: call void asm sideeffect "", "C,~{memory},~{$1}"(i8 addrspace(200)* %{{.+}})
  asm volatile("" : : "r"(p) : "memory");
  // CHECK: call void asm sideeffect "", "C,~{memory},~{$1}"(i8 addrspace(200)* addrspace(200)* %{{.+}})
  asm volatile("" : : "m"(p) : "memory");
}


_Bool f2(void) {
  _Bool b = 0;
  asm volatile("" : "+C"(b)); // NOTE: this is invalid but is only detected when emitting asm:
  // expected-error@-1{{couldn't allocate output register for constraint 'C'}}
  // CHECK: call i8 asm sideeffect "", "=C,0,~{$1}"(i1 %{{.+}})
  return b;
}

// check that it doesn't crash
// RUN: %cheri_purecap_cc1 -emit-obj -O2 -std=gnu99 -o /dev/null %s
// next check that it emits sensible IR:
// RUN: %cheri_purecap_cc1 -O2 -std=gnu99 -o - -emit-llvm %s | FileCheck %s

// https://github.com/CTSRD-CHERI/clang/issues/144

int a();
int d();
int e();
int *b() {
  int *c;
  if (a)
    d();
  c = a;
  if (c == 0)
    return 0;
  e();
  return c;
}

// CHECK: {{%.+}} = phi i32 addrspace(200)* [ {{%.+}}, {{%.+}} ], [ null, {{%.+}}]{{$}}

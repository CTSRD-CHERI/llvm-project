// RUN: %cheri_purecap_cc1 -emit-llvm -mrelocation-model pic -pic-level 1 -mthread-model posix -target-feature -noabicalls -O2 -std=gnu99 -ftls-model=local-exec -x c -o - %s | FileCheck %s -check-prefix PURECAP
// RUN: %cheri_purecap_cc1 -emit-llvm -mrelocation-model pic -pic-level 1 -mthread-model posix -target-feature -noabicalls -O2 -std=gnu99 -ftls-model=local-exec -x c -o - %s | FileCheck %s -check-prefix HYBRID
// check that compiling it to an object doesn't crash
// RUN: %cheri_purecap_cc1 -emit-obj -mrelocation-model pic -pic-level 1 -mthread-model posix -target-feature -noabicalls -O2 -std=gnu99 -ftls-model=local-exec -x c -o - -S %s | FileCheck %s

_Thread_local int dtors;
// TODO: in the future it would be nice to be able to use AS200 for thread_local
// PURECAP: @dtors = thread_local(localexec) local_unnamed_addr global i32 0, align 4
// HYBRID:  @dtors = thread_local(localexec) local_unnamed_addr global i32 0, align 4


int test(void) {
  // CHECK: rdhwr   $3, $29
  // CHECK: daddu   [[TLS_ADDR:\$[0-9]+]], $3, ${{[0-9]+}}
  // CHECK-O0: cfromddc $c2, [[TLS_ADDR]]
  // CHECK: lw $2, 0([[TLS_ADDR]])
  int *i = &dtors;
  return *i;
}

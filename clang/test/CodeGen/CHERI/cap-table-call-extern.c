// RUN: %cheri_purecap_cc1 -mrelocation-model pic -pic-level 1 -mthread-model posix -target-feature -noabicalls -mllvm -mips-ssection-threshold=0 -target-linker-version 302.3 -ffunction-sections -O3 -fcolor-diagnostics -mllvm -cheri-cap-table -x c -o - %s -emit-llvm -o - | FileCheck %s

typedef void(*fnptr)(void);

void extern_func(void);

fnptr fn = &extern_func;
static fnptr fn2 = &extern_func;

void test(void) {
  fn2 = fn;
  fn();
}

// CHECK: define void @test()
// CHECK:   %0 = load void () addrspace(200)*, void () addrspace(200)* addrspace(200)* @fn, align 32, !tbaa !3
// CHECK:   store void () addrspace(200)* %0, void () addrspace(200)* addrspace(200)* @fn2, align 32, !tbaa !3
// CHECK:   tail call void %0() #2

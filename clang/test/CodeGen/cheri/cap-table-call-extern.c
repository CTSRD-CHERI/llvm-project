// RUN: %cheri_purecap_cc1 -O3 -mllvm -cheri-cap-table-abi=plt -o - %s -emit-llvm -o - | %cheri_FileCheck %s

typedef void(*fnptr)(void);

void extern_func(void);

fnptr fn = &extern_func;
static fnptr fn2 = &extern_func;

void test(void) {
  fn2 = fn;
  fn();
}

// CHECK-LABEL: define void @test()
// CHECK:   %0 = load void () addrspace(200)*, void () addrspace(200)* addrspace(200)* @fn, align [[$CAP_SIZE]]
// CHECK:   store void () addrspace(200)* %0, void () addrspace(200)* addrspace(200)* @fn2, align [[$CAP_SIZE]]
// CHECK:   tail call void %0() #2

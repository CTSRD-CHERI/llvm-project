// RUN: %cheri_purecap_cc1 -std=gnu99 -ftls-model=local-exec -o - -O0 -emit-llvm  %s | FileCheck %s
// calling a weak function used to crash the compiler

void a(void);
#pragma weak a
void b(void) {
// CHECK: [[VAR0:%.+]] = call i8 addrspace(200)* @llvm.cheri.pcc.get()
// CHECK: [[VAR1:%.+]] = call i8 addrspace(200)* @llvm.cheri.cap.from.pointer(i8 addrspace(200)* [[VAR0]], i64 ptrtoint (void ()* @a to i64))
// CHECK: [[VAR2:%.+]] = bitcast i8 addrspace(200)* [[VAR1]] to void () addrspace(200)*
// CHECK: call void [[VAR2]]()
  a();
}

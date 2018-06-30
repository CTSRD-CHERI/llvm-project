// RUN: %cheri_purecap_cc1 -mllvm -cheri-cap-table-abi=pcrel -std=gnu99 -ftls-model=local-exec -o - -O0 -emit-llvm  %s | FileCheck %s
// RUN: %cheri_purecap_cc1 -mllvm -cheri-cap-table-abi=legacy -std=gnu99 -ftls-model=local-exec -o - -O0 -emit-llvm  %s | FileCheck %s -check-prefix LEGACY
// RUN: %cheri_purecap_cc1 -mllvm -cheri-cap-table-abi=legacy -std=gnu99 -ftls-model=local-exec -o /dev/null -O0 -S  %s
// RUN: %cheri_purecap_cc1 -mllvm -cheri-cap-table-abi=pcrel -std=gnu99 -ftls-model=local-exec -o /dev/null -O0 -S  %s
// calling a weak function used to crash the compiler

void a(void);
#pragma weak a
void b(void) {
  // LEGACY: [[VAR0:%.+]] = call i8 addrspace(200)* @llvm.cheri.pcc.get()
  // LEGACY: [[VAR1:%.+]] = call i8 addrspace(200)* @llvm.cheri.cap.from.pointer(i8 addrspace(200)* [[VAR0]], i64 ptrtoint (void ()* @a to i64))
  // LEGACY: [[VAR2:%.+]] = bitcast i8 addrspace(200)* [[VAR1]] to void () addrspace(200)*
  // LEGACY: call void [[VAR2]]()
  // CHECK: call void @a()
  a();
}

// RUN: %cheri_purecap_cc1 -emit-llvm -o - %s | FileCheck %s -check-prefix FOLDED

static void
crt_init_globals()
{
  void *gdc = __builtin_cheri_global_data_get();
  void *pcc = __builtin_cheri_program_counter_get();
}

void
__start(void)
{
  // CHECK: [[VAR0:%.+]] = call i8 addrspace(200)* @llvm.cheri.pcc.get()
  // CHECK: [[VAR1:%.+]] = call i8 addrspace(200)* @llvm.cheri.cap.offset.set(i8 addrspace(200)* [[VAR0]], i64 ptrtoint (void ()* @crt_init_globals to i64))
  // CHECK: [[VAR2:%.+]] = bitcast i8 addrspace(200)* [[VAR1]] to void (...) addrspace(200)*
  // CHECK: [[CALLEE_KNR_CAST:%.+]] = bitcast void (...) addrspace(200)* [[VAR2]] to void () addrspace(200)*
  // CHECK: call void [[CALLEE_KNR_CAST]]()
  // XXXAR: the generated assembly code seems sensible, so I guess a direct call now works
  // FOLDED: call void @crt_init_globals()
  crt_init_globals();
}

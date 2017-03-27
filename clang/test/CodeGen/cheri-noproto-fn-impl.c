// RUN: %clang_cc1 -triple cheri-unknown-freebsd -target-abi purecap -emit-llvm -o - %s | FileCheck %s

static void
crt_init_globals()
{
  void *gdc = __builtin_cheri_global_data_get();
  void *pcc = __builtin_cheri_program_counter_get();
}

void
__start(void)
{
  // CHECK: %0 = call i8 addrspace(200)* @llvm.cheri.pcc.get()
  // CHECK: %1 = call i8 addrspace(200)* @llvm.cheri.cap.offset.set(i8 addrspace(200)* %0, i64 ptrtoint (void ()* @crt_init_globals to i64))
  // CHECK: %2 = bitcast i8 addrspace(200)* %1 to void (...) addrspace(200)*
  // CHECK: %callee.knr.cast = bitcast void (...) addrspace(200)* %2 to void () addrspace(200)*
  // CHECK: call void %callee.knr.cast()
  crt_init_globals();
}

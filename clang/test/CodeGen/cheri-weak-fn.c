// RUN: %clang_cc1 %s "-target-abi" "purecap" -emit-llvm -triple cheri-unknown-freebsd -o - 
__attribute__((weak))
void fn();

void *v()
{
  // CHECK: @llvm.mips.pcc.get()
  // CHECK: @llvm.mips.cap.from.pointer
  return fn;
}

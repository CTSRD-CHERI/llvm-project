// RUN: %cheri_cc1 %s "-target-abi" "purecap" -emit-llvm  -o -
__attribute__((weak))
void fn();

void *v()
{
  // CHECK: @llvm.mips.pcc.get()
  // CHECK: @llvm.mips.cap.from.pointer
  return fn;
}

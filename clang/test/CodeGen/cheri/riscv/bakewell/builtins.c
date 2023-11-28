// RUN: %riscv32_bakewell_hybrid_cc1 -o - -emit-llvm %s | FileCheck %s --check-prefixes=RV32IZCHERIHYBRID
// RUN: %riscv64_bakewell_hybrid_cc1 -o - -emit-llvm %s | FileCheck %s --check-prefixes=RV32IZCHERIHYBRID

void standard(void *__capability cap){
  // RV32IZCHERIHYBRID-LABEL: @standard(
  // RV32IXCHERI-LABEL: @standard(
  // RV64IXCHERI-LABEL: @standard(
  volatile __SIZE_TYPE__ x;
  x = __builtin_cheri_length_get(cap);
  // RV32IXCHERI: call i32 @llvm.cheri.cap.length.get.i32
  // RV64IXCHERI: call i64 @llvm.cheri.cap.length.get.i64
  x = __builtin_cheri_copy_from_high(cap);
  // RV32IXCHERI: call i32 @llvm.cheri.copy.from.high.i32
  // RV64IXCHERI: call i64 @llvm.cheri.copy.from.high.i64

  void * __capability volatile z;
  z = __builtin_cheri_copy_to_high(cap, x);
  // RV32IXCHERI: call i8 addrspace(200)* @llvm.cheri.copy.to.high.i32(i8 addrspace(200)* {{.+}}, i32 42)
  // RV64IXCHERI: call i8 addrspace(200)* @llvm.cheri.copy.to.high.i64(i8 addrspace(200)* {{.+}}, i64 42)
 }

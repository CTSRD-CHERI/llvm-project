// REQUIRES: mips-registered-target

// RUN: %cheri_purecap_cc1 %s -o - -emit-llvm -O2 | FileCheck %s
// RUN: %cheri_purecap_cc1 %s -o - -S -O2 | FileCheck %s -check-prefix=ASM

typedef __SIZE_TYPE__ vaddr_t;
// The existing inline function macro from cheric.h
static inline void * __capability
cheri_setaddress(const void * __capability dst, vaddr_t addr) {
  return (__builtin_cheri_offset_increment(dst, addr - __builtin_cheri_address_get(dst)));
}

void* use_sys_cheric_function(void* in, vaddr_t new_addr) {
  // CHECK-LABEL: @use_sys_cheric_function(
  // CHECK: [[IN_ADDR:%.*]] = tail call i64 @llvm.cheri.cap.address.get.i64(i8 addrspace(200)* %in)
  // CHECK-NEXT: [[DIFF:%.*]] = sub i64 %new_addr, [[IN_ADDR]]
  // CHECK-NEXT: tail call i8 addrspace(200)* @llvm.cheri.cap.offset.increment.i64(i8 addrspace(200)* %in, i64 [[DIFF]])
  return cheri_setaddress(in, new_addr);
  // ASM-LABEL: .ent use_sys_cheric_function
  // ASM: csetaddr $c3, $c3, $4
  // ASM: .end use_sys_cheric_function
}

void* use_builtin_function(void* in, vaddr_t new_addr) {
  // CHECK-LABEL: @use_builtin_function(
  // CHECK: tail call i8 addrspace(200)* @llvm.cheri.cap.address.set.i64(i8 addrspace(200)* %in, i64 %new_addr)
  return __builtin_cheri_address_set(in, new_addr);
  // ASM-LABEL: .ent use_builtin_function
  // ASM: csetaddr $c3, $c3, $4
  // ASM: .end use_builtin_function
}

void* use_asm(void* in, vaddr_t new_addr) {
  void* result;
  __asm__ ("csetaddr %0, %1, %2\n\t" :"=C"(result) : "C"(in), "r"(new_addr));
  // CHECK-LABEL: @use_asm(
  // CHECK: call i8 addrspace(200)* asm "csetaddr $0, $1, $2\0A\09", "=C,C,r,~{$1}"(i8 addrspace(200)* %in, i64 %new_addr)
  return result;
  // ASM-LABEL: .ent use_asm
  // ASM: csetaddr $c3, $c3, $4
  // ASM: .end use_asm
}

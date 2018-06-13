// RUN: %clang_cc1 -triple cheri-none--elf -target-cpu mips64 -target-abi n64 -o - -O0 -emit-llvm  %s | FileCheck %s -check-prefixes CHECK,N64
// RUN: %clang_cc1 -triple cheri-none--elf -target-cpu mips64 -target-abi purecap -o - -O0 -emit-llvm  %s | FileCheck %s -check-prefixes CHECK,PURECAP

// Also check that we can lower it to assembly correctly (should be tested in LLVM, but this is easier)
// RUN: %clang_cc1 -triple cheri-none--elf -target-cpu mips64 -target-abi n64 -o - -O0 %s
// RUN: %clang_cc1 -triple cheri-none--elf -target-cpu mips64 -target-abi purecap -o - -O0 %s


void *framaddr(void) {
  // https://github.com/CTSRD-CHERI/clang/issues/200
  // CHECK-LABEL: @framaddr
  // N64: call i8* @llvm.frameaddress.p0i8(i32 0)
  // PURECAP: call i8 addrspace(200)* @llvm.frameaddress.p200i8(i32 0)
  void *b = __builtin_frame_address(0);
  return b;
}

void *retaddr(void) {
  // Also check that we do the right thing for __builtin_return_address()
  // CHECK-LABEL: @retaddr
  // N64: call i8* @llvm.returnaddress.p0i8(i32 0)
  // PURECAP: call i8 addrspace(200)* @llvm.returnaddress.p200i8(i32 0)
  void *b = __builtin_return_address(0);
  return b;
}

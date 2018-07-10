// RUN: %clang_cc1 -triple cheri-none--elf -target-cpu mips64 -target-abi n64 -o - -O0 -emit-llvm  %s | FileCheck %s -check-prefixes CHECK,N64
// RUN: %clang_cc1 -triple cheri-none--elf -target-cpu mips64 -target-abi purecap -o - -O0 -emit-llvm  %s | FileCheck %s -check-prefixes CHECK,PURECAP

// Also check that we can lower it to assembly correctly (should be tested in LLVM, but this is easier)
// RUN: %clang_cc1 -triple cheri-none--elf -target-cpu mips64 -target-abi n64 -o - -S -O2 %s | FileCheck %s -check-prefixes ASM,n64-ASM
// RUN: %clang_cc1 -triple cheri-none--elf -target-cpu mips64 -target-abi purecap -o - -S -O2 %s | FileCheck %s -check-prefixes ASM,PURECAP-ASM


void *frameaddr(void) {
  // https://github.com/CTSRD-CHERI/clang/issues/200
  // CHECK-LABEL: @frameaddr
  // N64: call i8* @llvm.frameaddress.p0i8(i32 0)
  // PURECAP: call i8 addrspace(200)* @llvm.frameaddress.p200i8(i32 0)
  void *b = __builtin_frame_address(0);

  // ASM-LABEL: frameaddr:
  // N64-ASM: move	$2, $fp
  // N64-ASM: jr	$ra
  // PURECAP-ASM: cmove	$c3,  $c24
  // PURECAP-ASM: cjr	$c17
  // ASM: .end frameaddr

  return b;
}

void *retaddr(void) {
  // Also check that we do the right thing for __builtin_return_address()
  // CHECK-LABEL: @retaddr
  // N64: call i8* @llvm.returnaddress.p0i8(i32 0)
  // PURECAP: call i8 addrspace(200)* @llvm.returnaddress.p200i8(i32 0)
  void *b = __builtin_return_address(0);

  // ASM-LABEL: retaddr:
  // N64-ASM: move	$2, $ra
  // N64-ASM: jr	$ra
  // in purecap this is moved to the delay slot
  // PURECAP-ASM: cjr	$c17
  // PURECAP-ASM-NEXT: cmove	$c3,  $c17
  // ASM: .end retaddr

  return b;
}

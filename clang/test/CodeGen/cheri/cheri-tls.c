// REQUIRES: mips-registered-target

// RUN: %cheri_purecap_cc1 -emit-llvm -mrelocation-model pic -ftls-model=local-exec -o - %s | FileCheck %s -check-prefix IR-PURECAP
// RUN: %cheri_cc1 -emit-llvm -mrelocation-model pic -ftls-model=local-exec -o - %s | FileCheck %s -check-prefix IR-MIPS
// Check that compiling it to an object doesn't crash
// RUN: %cheri_purecap_cc1 -emit-obj -mrelocation-model pic -O2 -std=gnu99 -ftls-model=local-exec -x c -o - -S %s | FileCheck %s -check-prefix ASM-PURECAP
// RUN: %cheri_cc1 -emit-obj -mrelocation-model pic -O2 -std=gnu99 -ftls-model=local-exec -x c -o - -S %s | FileCheck %s -check-prefix ASM-MIPS

_Thread_local int dtors;
// IR-PURECAP: @dtors = dso_local thread_local(localexec) addrspace(200) global i32 0, align 4
// IR-MIPS:    @dtors = dso_local thread_local(localexec) global i32 0, align 4

int test(void) {
  // ASM-MIPS: rdhwr   $3, $29
  // ASM-MIPS: daddu   [[TLS_ADDR:\$[0-9]+]], $3, ${{[0-9]+}}
  // ASM-MIPS-NEXT: jr $ra
  // ASM-MIPS-NEXT: lw $2, 0([[TLS_ADDR]])

  // ASM-PURECAP: lui   [[TLS_OFF:\$[0-9]+]], %tprel_hi(dtors)
  // ASM-PURECAP-NEXT: daddiu   [[TLS_OFF]], [[TLS_OFF]], %tprel_lo(dtors)
  // ASM-PURECAP-NEXT: creadhwr   [[TLS_BASE:\$c[0-9]+]], $chwr_userlocal
  // ASM-PURECAP-NEXT: cjr $c17
  // ASM-PURECAP-NEXT: clw $2, [[TLS_OFF]], 0([[TLS_BASE]])
  int *i = &dtors;
  return *i;
}

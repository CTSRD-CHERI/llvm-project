// RUN: %cheri_purecap_cc1 -emit-llvm -mrelocation-model pic -pic-level 1 -mthread-model posix -target-feature -noabicalls -O2 -std=gnu99 -ftls-model=local-exec -mllvm -cheri-cap-table-abi=pcrel -mllvm -cheri-cap-tls-abi=cap-equiv -x c -o - %s | FileCheck %s -check-prefix IR-CAP-EQUIV
// RUN: %cheri_purecap_cc1 -emit-llvm -mrelocation-model pic -pic-level 1 -mthread-model posix -target-feature -noabicalls -O2 -std=gnu99 -ftls-model=local-exec -x c -o - %s | FileCheck %s -check-prefix IR-LEGACY
// RUN: %cheri_cc1 -emit-llvm -mrelocation-model pic -pic-level 1 -mthread-model posix -target-feature -noabicalls -O2 -std=gnu99 -ftls-model=local-exec -x c -o - %s | FileCheck %s -check-prefix IR-LEGACY
// check that compiling it to an object doesn't crash
// RUN: %cheri_purecap_cc1 -emit-obj -mrelocation-model pic -pic-level 1 -mthread-model posix -target-feature -noabicalls -O2 -std=gnu99 -ftls-model=local-exec -mllvm -cheri-cap-table-abi=pcrel -mllvm -cheri-cap-tls-abi=cap-equiv -x c -o - -S %s | FileCheck %s -check-prefix ASM-CAP-EQUIV
// RUN: %cheri_purecap_cc1 -emit-obj -mrelocation-model pic -pic-level 1 -mthread-model posix -target-feature -noabicalls -O2 -std=gnu99 -ftls-model=local-exec -x c -o - -S %s | FileCheck %s -check-prefix ASM-LEGACY
// RUN: %cheri_cc1 -emit-obj -mrelocation-model pic -pic-level 1 -mthread-model posix -target-feature -noabicalls -O2 -std=gnu99 -ftls-model=local-exec -x c -o - -S %s | FileCheck %s -check-prefix ASM-LEGACY

_Thread_local int dtors;
// IR-CAP-EQUIV: @dtors = thread_local(localexec) local_unnamed_addr addrspace(200) global i32 0, align 4
// IR-LEGACY:    @dtors = thread_local(localexec) local_unnamed_addr global i32 0, align 4


int test(void) {
  // ASM-LEGACY: rdhwr   $3, $29
  // ASM-LEGACY: daddu   [[TLS_ADDR:\$[0-9]+]], $3, ${{[0-9]+}}
  // ASM-LEGACY: lw $2, 0([[TLS_ADDR]])

  // ASM-CAP-EQUIV: lui   [[TLS_OFF:\$[0-9]+]], %tprel_hi(dtors)
  // ASM-CAP-EQUIV: daddiu   [[TLS_OFF]], [[TLS_OFF]], %tprel_lo(dtors)
  // ASM-CAP-EQUIV: creadhwr   [[TLS_BASE:\$c[0-9]+]], $chwr_userlocal
  // ASM-CAP-EQUIV: clw $2, [[TLS_OFF]], 0([[TLS_BASE]])
  int *i = &dtors;
  return *i;
}

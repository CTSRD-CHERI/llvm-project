// REQUIRES: mips-registered-target

// RUN: %cheri_purecap_cc1 -mllvm -cheri-cap-table-abi=pcrel -std=gnu99 -ftls-model=local-exec -o - -O0 -emit-llvm  %s | FileCheck %s
// calling a weak function used to crash the compiler
// RUN: %cheri_purecap_cc1 -mllvm -cheri-cap-table-abi=legacy -std=gnu99 -ftls-model=local-exec -o /dev/null -O0 -S %s
// RUN: %cheri_purecap_cc1 -mllvm -cheri-cap-table-abi=pcrel -std=gnu99 -ftls-model=local-exec -o /dev/null -O0 -S %s

void a(void);
#pragma weak a
void b(void) {
  a();
}

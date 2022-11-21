// REQUIRES: mips-registered-target
// RUN: %cheri_purecap_cc1 -debug-info-kind=standalone -emit-obj -o %t.purecap.o %s
// RUN: llvm-dwarfdump -debug-frame %t.purecap.o | FileCheck %s -check-prefixes PURECAP,CHECK
// RUN: %cheri_cc1 "-mrelocation-model" "pic" -debug-info-kind=standalone -emit-obj -o %t.hybrid.o %s
// RUN: llvm-dwarfdump -debug-frame %t.hybrid.o | FileCheck %s -check-prefixes HYBRID,CHECK

// Also check that the register is correct when invoking the driver instead of -cc1
// RUN: %cheri_clang -g -c -o %t.hybrid-driver.o %s -fomit-frame-pointer -gdwarf-4 -v -fno-addrsig -fno-unwind-tables -fno-asynchronous-unwind-tables
// RUN: llvm-dwarfdump -debug-frame %t.hybrid-driver.o | FileCheck %s -check-prefixes HYBRID,CHECK
// RUN: %cheri_purecap_clang -g -c -o %t.purecap-driver.o %s -fomit-frame-pointer -gdwarf-4 -fno-addrsig -fno-unwind-tables -fno-asynchronous-unwind-tables
// RUN: llvm-dwarfdump -debug-frame %t.purecap-driver.o | FileCheck %s -check-prefixes PURECAP,CHECK

int* external_func(int);

int* test(int i) {
  return external_func(i);
}

// N64 uses $ra (reg 31) for returns and $sp (reg 29) for the default CFA:
// Purecap uses $c17 (reg 89) for returns and $c11 (reg 83) for the default CFA:

// CHECK-LABEL: 00000000
// CHECK-NEXT: Format: DWARF32
/// NB: Exception handling uses Version 1 unconditionally, so if this ends up
/// being 1, the driver logic may have changed and we need additional flags.
// CHECK-NEXT: Version: 4
// n64 uses ra (register 31)
// HYBRID:  Return address column: 31
// PURECAP: Return address column: 89
// CHECK-EMPTY:
// HYBRID-NEXT: DW_CFA_def_cfa_register: SP_64
// PURECAP-NEXT: DW_CFA_def_cfa_register: C11
// CHECK-NEXT: DW_CFA_nop

// CHECK-LABEL: 00000018
// CHECK-NEXT: Format: DWARF32
// CHECK-NEXT: DW_CFA_advance_loc: 4

// PURECAP-NEXT: DW_CFA_def_cfa_offset: +{{32|64}}
// PURECAP-NEXT: DW_CFA_advance_loc: 4
// PURECAP-NEXT: DW_CFA_offset_extended: C17 -{{16|32}}
// PURECAP-NEXT: DW_CFA_nop:

// HYBRID-NEXT: DW_CFA_def_cfa_offset: +32
// HYBRID-NEXT: DW_CFA_advance_loc: 8
// HYBRID-NEXT: DW_CFA_offset: RA_64 -8
// HYBRID-NEXT: DW_CFA_offset: GP_64 -16

// RUN: %cheri_purecap_cc1 -debug-info-kind=standalone -emit-obj -o %t.purecap.o %s
// RUN: llvm-dwarfdump -debug-frame %t.purecap.o | FileCheck %s -check-prefixes PURECAP,CHECK
// RUN: %cheri_cc1 "-mrelocation-model" "pic" -debug-info-kind=standalone -emit-obj -o %t.hybrid.o %s
// RUN: llvm-dwarfdump -debug-frame %t.hybrid.o | FileCheck %s -check-prefixes HYBRID,CHECK
// RUN: llvm-dwarfdump -debug-frame %t.hybrid.o

// Also check that the register is correct when invoking the driver instead of -cc1
// RUN: %cheri_clang -g -c -o %t.hybrid-driver.o %s -fomit-frame-pointer -gdwarf-4 -v -fno-addrsig -fno-unwind-tables
// RUN: llvm-dwarfdump -debug-frame %t.hybrid-driver.o | FileCheck %s -check-prefixes HYBRID,CHECK
// RUN: %cheri_purecap_clang -g -c -o %t.purecap-driver.o %s -fomit-frame-pointer -gdwarf-4 -fno-addrsig -fno-unwind-tables
// RUN: llvm-dwarfdump -debug-frame %t.purecap-driver.o | FileCheck %s -check-prefixes PURECAP,CHECK




int* external_func(int);

int* test(int i) {
  return external_func(i);
}

// N64 uses $ra (reg 31) for returns and $sp (reg 29) for the default CFA:
// Purecap uses $c17 (reg 89) for returns and $c11 (reg 83) for the default CFA:

// CHECK-LABEL: 00000000
// CHECK-NEXT: Version: 4
// n64 uses ra (register 31)
// HYBRID:  Return address column: 31
// PURECAP: Return address column: 89
// CHECK-EMPTY:
// HYBRID-NEXT: DW_CFA_def_cfa_register: reg29
// PURECAP-NEXT: DW_CFA_def_cfa_register: reg83
// CHECK-NEXT: DW_CFA_nop

// CHECK-LABEL: 00000018
// CHECK-NEXT: DW_CFA_advance_loc: 4

// PURECAP-NEXT: DW_CFA_def_cfa_offset: +{{32|64}}
// PURECAP-NEXT: DW_CFA_advance_loc: 4
// PURECAP-NEXT: DW_CFA_offset_extended: reg89 -{{16|32}}
// PURECAP-NEXT: DW_CFA_nop:

// HYBRID-NEXT: DW_CFA_def_cfa_offset: +32
// HYBRID-NEXT: DW_CFA_advance_loc: 8
// HYBRID-NEXT: DW_CFA_offset: reg31 -8
// HYBRID-NEXT: DW_CFA_offset: reg28 -16

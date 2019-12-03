// REQUIRES: clang

// Since the captable register is not necessarily $cgp (e.g. in the pcrelative ABI)
// we need an intrinsic int_mips_captable_get to get it

// RUN: %cheri_purecap_cc1 -O1 -o - -S %s | FileCheck %s -check-prefix PCREL
// RUN: %cheri_purecap_cc1 -mllvm -cheri-cap-table-abi=pcrel -O1 -o - -S %s | FileCheck %s -check-prefix PCREL
// RUN: %cheri_purecap_cc1 -mllvm -cheri-cap-table-abi=plt -O1 -o - -S %s | FileCheck %s -check-prefix PLT
// RUN: %cheri_purecap_cc1 -mllvm -cheri-cap-table-abi=fn-desc -O1 -o - -S %s | FileCheck %s -check-prefix FNDESC
// Legacy and hybrid don't have a captable -> this should be an error
// RUN: %cheri_purecap_cc1 -mllvm -cheri-cap-table-abi=legacy -O1 -o - -S %s -verify=not-captable | FileCheck %s -check-prefix LEGACY
// RUN: %cheri_cc1 -O1 -o - -S %s -verify=not-captable | FileCheck %s -check-prefix HYBRID

void* __capability get_captable() {
  // Without debug info it reports the function as the error location
  // not-captable-error@-2{{the current ABI does not use a captable}}
  return __builtin_mips_cheri_get_captable();

  // PCREL: lui	$1, %pcrel_hi(_CHERI_CAPABILITY_TABLE_-8)
  // PCREL-NEXT: daddiu	$1, $1, %pcrel_lo(_CHERI_CAPABILITY_TABLE_-4)
  // PCREL-NEXT: cgetpccincoffset $c3, $1
  // PCREL-NEXT: cjr	$c17
  // PCREL-NEXT: nop

  // PLT: cjr	$c17
  // PLT-NEXT: cmove $c3, $c26

  // FNDESC: cjr	$c17
  // FNDESC-NEXT: cmove $c3, $c26

  // LEGACY: cjr $c17
  // LEGACY-NEXT: cgetnull $c3

  // HYBRID: jr $ra
  // HYBRID-NEXT: cgetnull $c3
}

// REQUIRES: mips-registered-target
// RUN: %cheri_purecap_clang -cheri-cap-table-abi=plt -c %s -o - | llvm-readobj --arch-specific - | FileCheck %s -check-prefix PLT
// PLT: ISA Extension: CHERI purecap (PLT based) (0xC2)

// RUN: %cheri_purecap_clang -cheri-cap-table-abi=pcrel -c %s -o - | llvm-readobj --arch-specific - | FileCheck %s -check-prefix PCREL
// PCREL: ISA Extension: CHERI purecap (pc-relative) (0xC3)

// RUN: %cheri_purecap_clang -cheri-cap-table-abi=fn-desc -c %s -o - | llvm-readobj --arch-specific - | FileCheck %s -check-prefix FNDESC
// FNDESC: ISA Extension: CHERI purecap (function descriptor) (0xC4)

// The default is pcrel:
// RUN: %cheri_purecap_clang -c %s -o - | llvm-readobj --arch-specific - | FileCheck %s -check-prefix DEFAULT
// DEFAULT: ISA Extension: CHERI purecap (pc-relative) (0xC3)

int __start(void) {
  return 0;
}

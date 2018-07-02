# RUN: %cheri_purecap_llvm-mc -cheri-cap-table-abi=legacy -filetype=obj %s -o - | llvm-readobj -mips-abi-flags - | FileCheck %s -check-prefix LEGACY
# LEGACY: ISA Extension: CHERI purecap (legacy) (0xC1)

# RUN: %cheri_purecap_llvm-mc -cheri-cap-table-abi=plt -filetype=obj %s -o - | llvm-readobj -mips-abi-flags - | FileCheck %s -check-prefix PLT
# PLT: ISA Extension: CHERI purecap (PLT based) (0xC2)

# RUN: %cheri_purecap_llvm-mc -cheri-cap-table-abi=pcrel -filetype=obj %s -o - | llvm-readobj -mips-abi-flags - | FileCheck %s -check-prefix PCREL
# PCREL: ISA Extension: CHERI purecap (pc-relative) (0xC3)

# RUN: %cheri_purecap_llvm-mc -cheri-cap-table-abi=fn-desc -filetype=obj %s -o - | llvm-readobj -mips-abi-flags - | FileCheck %s -check-prefix FNDESC
# FNDESC: ISA Extension: CHERI purecap (function descriptor) (0xC4)

# The default is pc-relative legacy:
# RUN: %cheri_purecap_llvm-mc -filetype=obj %s -o - | llvm-readobj -mips-abi-flags - | FileCheck %s -check-prefix DEFAULT
# DEFAULT: ISA Extension: CHERI purecap (legacy) (0xC1)

.text
__start:
  nop

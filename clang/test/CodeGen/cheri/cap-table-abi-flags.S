# RUN: %cheri_purecap_clang -cheri-cap-table-abi=plt -c %s -o - -### 2>&1 | FileCheck %s -check-prefix CMDLINE

# Preprocess first then run as (make sure both contain the cap-table-abi flag
# CMDLINE: "-cc1" "-triple" "mips64c{{128|256}}-unknown-freebsd-purecap" "-E"
# CMDLINE-SAME: "-mrelocation-model" "pic"
# CMDLINE-SAME: "-target-abi" "purecap"
# CMDLINE-SAME: "-mllvm" "-cheri-cap-table-abi=plt" "-mllvm" "-mxcaptable=false"

# CMDLINE-NEXT: "-cc1as" "-triple" "mips64c{{128|256}}-unknown-freebsd-purecap"
# CMDLINE-SAME: "-mrelocation-model" "pic"
# CMDLINE-SAME: "-target-abi" "purecap"
# CMDLINE-SAME: "-mllvm" "-cheri-cap-table-abi=plt" "-mllvm" "-mxcaptable=false"

# RUN: %cheri_purecap_clang -cheri-cap-table-abi=plt -c %s -o - | llvm-readobj --arch-specific - | FileCheck %s -check-prefix PLT
# PLT: ISA Extension: CHERI purecap (PLT based) (0xC2)

# RUN: %cheri_purecap_clang -cheri-cap-table-abi=pcrel -c %s -o - | llvm-readobj --arch-specific - | FileCheck %s -check-prefix PCREL
# PCREL: ISA Extension: CHERI purecap (pc-relative) (0xC3)

# RUN: %cheri_purecap_clang -cheri-cap-table-abi=fn-desc -c %s -o - | llvm-readobj --arch-specific - | FileCheck %s -check-prefix FNDESC
# FNDESC: ISA Extension: CHERI purecap (function descriptor) (0xC4)

# The default is pcrel:
# RUN: %cheri_purecap_clang -c %s -o - | llvm-readobj --arch-specific - | FileCheck %s -check-prefix DEFAULT
# DEFAULT: ISA Extension: CHERI purecap (pc-relative) (0xC3)

.text
__start:
  nop

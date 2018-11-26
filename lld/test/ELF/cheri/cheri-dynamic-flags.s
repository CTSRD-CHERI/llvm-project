# RUN: %cheri128_purecap_llvm-mc -cheri-cap-table-abi=legacy -filetype=obj %s -o %t-legacy.o
# RUN: %cheri128_purecap_llvm-mc -cheri-cap-table-abi=pcrel -filetype=obj %s -o %t-pcrel.o
# RUN: %cheri128_purecap_llvm-mc -cheri-cap-table-abi=plt -filetype=obj %s -o %t-plt.o
# RUN: %cheri128_purecap_llvm-mc -cheri-cap-table-abi=fn-desc -filetype=obj %s -o %t-fn-desc.o
# RUN: %cheri128_purecap_llvm-mc -filetype=obj %s -o %t-default.o
# RUN: ld.lld -shared %t-legacy.o -o - | llvm-readobj -dynamic-table - | FileCheck %s -check-prefix LEGACY
# LEGACY: 0x000000007000C002 MIPS_CHERI_FLAGS     0x0
# RUN: ld.lld -shared %t-pcrel.o -o - | llvm-readobj -dynamic-table - | FileCheck %s -check-prefix PCREL
# PCREL: 0x000000007000C002 MIPS_CHERI_FLAGS     0x1
# RUN: ld.lld -shared %t-plt.o -o - | llvm-readobj -dynamic-table - | FileCheck %s -check-prefix PLT
# PLT: 0x000000007000C002 MIPS_CHERI_FLAGS     0x2
# RUN: ld.lld -shared %t-fn-desc.o -o - | llvm-readobj -dynamic-table - | FileCheck %s -check-prefix FNDESC
# FNDESC: 0x000000007000C002 MIPS_CHERI_FLAGS     0x3

# Default is pc-relative
# RUN: ld.lld -shared %t-default.o -o - | llvm-readobj -dynamic-table - | FileCheck %s -check-prefix PCREL

.text
nop

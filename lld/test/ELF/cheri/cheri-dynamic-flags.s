# RUN: %cheri128_purecap_llvm-mc -cheri-cap-table-abi=pcrel -filetype=obj %s -o %t-pcrel.o
# RUN: %cheri128_purecap_llvm-mc -cheri-cap-table-abi=plt -filetype=obj %s -o %t-plt.o
# RUN: %cheri128_purecap_llvm-mc -cheri-cap-table-abi=fn-desc -filetype=obj %s -o %t-fn-desc.o
# RUN: %cheri128_purecap_llvm-mc -filetype=obj %s -o %t-default.o
# RUN: ld.lld -shared %t-pcrel.o -o - | llvm-readobj --dynamic-table - | FileCheck %s --check-prefix PCREL
# PCREL: 0x000000007000C002 MIPS_CHERI_FLAGS     ABI_PCREL
# RUN: ld.lld -shared %t-plt.o -o - | llvm-readobj --dynamic-table - | FileCheck %s --check-prefix PLT
# PLT: 0x000000007000C002 MIPS_CHERI_FLAGS     ABI_PLT
# RUN: ld.lld -shared %t-fn-desc.o -o - | llvm-readobj --dynamic-table - | FileCheck %s --check-prefix FNDESC
# FNDESC: 0x000000007000C002 MIPS_CHERI_FLAGS     ABI_FNDESC

# Default is pc-relative
# RUN: ld.lld -shared %t-default.o -o - | llvm-readobj --dynamic-table - | FileCheck %s --check-prefix PCREL

.text
nop

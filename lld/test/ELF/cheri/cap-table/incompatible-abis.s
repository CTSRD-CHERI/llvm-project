# RUN: %cheri128_purecap_llvm-mc -cheri-cap-table-abi=legacy -filetype=obj %s -o %t-legacy.o
# RUN: %cheri128_purecap_llvm-mc -cheri-cap-table-abi=pcrel -filetype=obj %s -o %t-pcrel.o
# RUN: %cheri128_purecap_llvm-mc -cheri-cap-table-abi=plt -filetype=obj %s -o %t-plt.o
# RUN: %cheri128_purecap_llvm-mc -cheri-cap-table-abi=fn-desc -filetype=obj %s -o %t-fn-desc.o
# RUN: %cheri128_purecap_llvm-mc -filetype=obj %s -o %t-default.o
# RUN: yaml2obj -o %t-non-cheri.o %S/Inputs/non-cheri-isa-ext.yaml
# RUN: not ld.lld %t-legacy.o %t-pcrel.o %t-plt.o %t-fn-desc.o %t-default.o %t-non-cheri.o 2>&1 | FileCheck %s

# CHECK:      error: incompatible pure-capability ABIs:
# CHECK-NEXT: >>> {{.+}}incompatible-abis.s.tmp-legacy.o uses EXT_CHERI_ABI_LEGACY
# CHECK-NEXT: >>> {{.+}}incompatible-abis.s.tmp-pcrel.o uses EXT_CHERI_ABI_PCREL

# CHECK:      error: incompatible pure-capability ABIs:
# CHECK-NEXT: >>> {{.+}}incompatible-abis.s.tmp-pcrel.o uses EXT_CHERI_ABI_PCREL
# CHECK-NEXT: >>> {{.+}}incompatible-abis.s.tmp-plt.o uses EXT_CHERI_ABI_PLT

# CHECK:      error: incompatible pure-capability ABIs:
# CHECK-NEXT: >>> {{.+}}incompatible-abis.s.tmp-plt.o uses EXT_CHERI_ABI_PLT
# CHECK-NEXT: >>> {{.+}}incompatible-abis.s.tmp-fn-desc.o uses EXT_CHERI_ABI_FNDESC

# CHECK:      error: incompatible pure-capability ABIs:
# CHECK-NEXT: >>> {{.+}}incompatible-abis.s.tmp-fn-desc.o uses EXT_CHERI_ABI_FNDESC
# CHECK-NEXT: >>> {{.+}}incompatible-abis.s.tmp-default.o uses EXT_CHERI_ABI_LEGACY

# CHECK:      error: incompatible pure-capability ABIs:
# CHECK-NEXT: >>> {{.+}}incompatible-abis.s.tmp-default.o uses EXT_CHERI_ABI_LEGACY
# CHECK-NEXT: >>> {{.+}}incompatible-abis.s.tmp-non-cheri.o uses EXT_OCTEON

.text
nop

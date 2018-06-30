// RUN: %cheri_purecap_cc1 -mllvm -cheri-cap-table-abi=legacy -E -dM -x c /dev/null | FileCheck %s -check-prefix NOTABLE
// RUN: %cheri_purecap_cc1 -mllvm -cheri-cap-table-abi=plt -E -dM -x c /dev/null | FileCheck %s -check-prefix CAPTABLE-PLT
// RUN: %cheri_purecap_cc1 -mllvm -cheri-cap-table-abi=pcrel -E -dM -x c /dev/null | FileCheck %s -check-prefix CAPTABLE-PCREL
// RUN: %cheri_purecap_cc1 -mllvm -cheri-cap-table-abi=fn-desc -E -dM -x c /dev/null | FileCheck %s -check-prefix CAPTABLE-FNDESC
// RUN: %cheri_purecap_cc1 -E -dM -x c /dev/null | FileCheck %s -check-prefix CAPTABLE-PCREL

// NOTABLE-NOT: __CHERI_CAPABILITY_TABLE__
// CAPTABLE-PLT: __CHERI_CAPABILITY_TABLE__ 2
// CAPTABLE-PCREL: __CHERI_CAPABILITY_TABLE__ 3
// CAPTABLE-FNDESC: __CHERI_CAPABILITY_TABLE__ 4


// RUN: %cheri_purecap_cc1 -E -dM -x c /dev/null | FileCheck %s -check-prefix NOTABLE
// RUN: %cheri_purecap_cc1 -mllvm -cheri-cap-table-abi=plt -E -dM -x c /dev/null | FileCheck %s -check-prefix CAPTABLE

// NOTABLE-NOT: __CHERI_CAPABILITY_TABLE__
// CAPTABLE: __CHERI_CAPABILITY_TABLE__ 1


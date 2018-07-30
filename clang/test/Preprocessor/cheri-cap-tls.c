// RUN: %cheri_purecap_cc1 -mllvm -cheri-cap-table-abi=pcrel -E -dM -x c /dev/null | FileCheck %s -check-prefix LEGACY
// RUN: %cheri_purecap_cc1 -mllvm -cheri-cap-tls-abi=cap-equiv -E -dM -x c /dev/null | FileCheck %s -check-prefix LEGACY
// RUN: %cheri_purecap_cc1 -mllvm -cheri-cap-tls-abi=legacy -E -dM -x c /dev/null | FileCheck %s -check-prefix LEGACY
// RUN: %cheri_purecap_cc1 -mllvm -cheri-cap-table-abi=pcrel -mllvm -cheri-cap-tls-abi=cap-equiv -E -dM -x c /dev/null | FileCheck %s -check-prefix CAP-EQUIV

// LEGACY-NOT: __CHERI_CAPABILITY_TLS__
// CAP-EQUIV: __CHERI_CAPABILITY_TLS__ 1

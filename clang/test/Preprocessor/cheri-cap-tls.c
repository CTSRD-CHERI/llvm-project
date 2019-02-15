// -mllvm -cheri-cap-table-abi != legacy defaults to cap-equiv (but can be overriden:)
// RUN: %cheri_purecap_cc1 -mllvm -cheri-cap-table-abi=pcrel -E -dM -x c /dev/null | FileCheck %s -check-prefix CAP-EQUIV
// RUN: %cheri_purecap_cc1 -mllvm -cheri-cap-table-abi=pcrel -mllvm -cheri-cap-tls-abi=cap-equiv -E -dM -x c /dev/null | FileCheck %s -check-prefix CAP-EQUIV
// RUN: %cheri_purecap_cc1 -mllvm -cheri-cap-table-abi=pcrel -mllvm -cheri-cap-tls-abi=legacy -E -dM -x c /dev/null | FileCheck %s -check-prefix LEGACY
// If -cheri-cap-table-abi=legacy we ignore the cap-tls-abi flag:
// RUN: %cheri_purecap_cc1 -mllvm -cheri-cap-table-abi=legacy -mllvm -cheri-cap-tls-abi=cap-equiv -E -dM -x c /dev/null | FileCheck %s -check-prefix LEGACY

// Without any flags we default to pcrel:
// RUN: %cheri_purecap_cc1 -E -dM -x c /dev/null | FileCheck %s -check-prefix CAP-EQUIV

// LEGACY-NOT: __CHERI_CAPABILITY_TLS__
// CAP-EQUIV: __CHERI_CAPABILITY_TLS__ 1

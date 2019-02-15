// RUN: %cheri_clang -### -mabi=sandbox %s 2>&1 | FileCheck --check-prefixes=WARN,CHECK %s
// WARN: warning: CHERI ABI 'sandbox' is deprecated; use 'purecap' instead
// CHECK: "-target-abi" "purecap"

/// Enabling stack-protector for purecap should warn that it is useless
// RUN: %cheri_purecap_clang -### -c -o /dev/null %s -fstack-protector-strong 2>&1 | \
// RUN:    FileCheck %s --implicit-check-not="-stack-protector" --check-prefix PURECAP-SSP
// RUN: %riscv32_cheri_purecap_clang -### -c -o /dev/null %s -fstack-protector-strong 2>&1 | \
// RUN:    FileCheck %s --implicit-check-not="-stack-protector" --check-prefix PURECAP-SSP
// RUN: %riscv64_cheri_purecap_clang -### -c -o /dev/null %s -fstack-protector-strong 2>&1 | \
// RUN:    FileCheck %s --implicit-check-not="-stack-protector" --check-prefix PURECAP-SSP
// RUN: %cheri_purecap_clang -### -c -o /dev/null %s -fstack-protector-all 2>&1 | \
// RUN:    FileCheck %s --implicit-check-not="-stack-protector" --check-prefix PURECAP-SSP
// RUN: %riscv32_cheri_purecap_clang -### -c -o /dev/null %s -fstack-protector-all 2>&1 | \
// RUN:    FileCheck %s --implicit-check-not="-stack-protector" --check-prefix PURECAP-SSP
// RUN: %riscv64_cheri_purecap_clang -### -c -o /dev/null %s -fstack-protector-all 2>&1 | \
// RUN:    FileCheck %s --implicit-check-not="-stack-protector" --check-prefix PURECAP-SSP
// RUN: %cheri_purecap_clang -### -c -o /dev/null %s -fstack-protector 2>&1 | \
// RUN:    FileCheck %s --implicit-check-not="-stack-protector" --check-prefix PURECAP-SSP
// RUN: %riscv64_cheri_purecap_clang -### -c -o /dev/null %s -fstack-protector 2>&1 | \
// RUN:    FileCheck %s --implicit-check-not="-stack-protector" --check-prefix PURECAP-SSP
// RUN: %riscv64_cheri_purecap_clang -### -c -o /dev/null %s -fstack-protector 2>&1 | \
// RUN:    FileCheck %s --implicit-check-not="-stack-protector" --check-prefix PURECAP-SSP

// RUN: %cheri_purecap_clang -### -c -o /dev/null %s -fno-stack-protector 2>&1 | \
// RUN:    FileCheck %s --implicit-check-not="-stack-protector" --check-prefix PURECAP-NOSSP
// RUN: %riscv32_cheri_purecap_clang -### -c -o /dev/null %s -fno-stack-protector 2>&1 | \
// RUN:    FileCheck %s --implicit-check-not="-stack-protector" --check-prefix PURECAP-NOSSP
// RUN: %riscv64_cheri_purecap_clang -### -c -o /dev/null %s -fno-stack-protector 2>&1 | \
// RUN:    FileCheck %s --implicit-check-not="-stack-protector" --check-prefix PURECAP-NOSSP

// PURECAP-SSP: warning: ignoring 'fstack-protector{{(-strong|-all)?}}' option since stack protector is unnecessary when using pure-capability CHERI compilation [-Woption-ignored]
// PURECAP-NOSSP-NOT: warning:
// PURECAP-NOSSP-NOT: "-stack-protector":

/// However, for hybrid code it should still be parsed normally:
// RUN: %riscv64_cheri_clang -### -c -o /dev/null %s -fstack-protector-strong 2>&1 | \
// RUN:    FileCheck %s --check-prefix NOCAP-SSP-STRONG
// RUN: %riscv64_cheri_clang -### -c -o /dev/null %s -fstack-protector-all 2>&1 | \
// RUN:    FileCheck %s --check-prefix NOCAP-SSP-ALL
// RUN: %riscv64_cheri_clang -### -c -o /dev/null %s -fstack-protector 2>&1 | \
// RUN:    FileCheck %s --check-prefix NOCAP-SSP-DEFAULT
// RUN: %riscv64_cheri_clang -### -c -o /dev/null %s -fno-stack-protector 2>&1 | \
// RUN:    FileCheck %s --check-prefix NOCAP-SSP-NONE
// RUN: %cheri_clang -### -c -o /dev/null %s -fstack-protector-strong 2>&1 | \
// RUN:    FileCheck %s --check-prefix NOCAP-SSP-STRONG
// RUN: %cheri_clang -### -c -o /dev/null %s -fstack-protector-all 2>&1 | \
// RUN:    FileCheck %s --check-prefix NOCAP-SSP-ALL
// RUN: %cheri_clang -### -c -o /dev/null %s -fstack-protector 2>&1 | \
// RUN:    FileCheck %s --check-prefix NOCAP-SSP-DEFAULT
// RUN: %cheri_clang -### -c -o /dev/null %s -fno-stack-protector 2>&1 | \
// RUN:    FileCheck %s --check-prefix NOCAP-SSP-NONE

// NOCAP-SSP-STRONG: "-stack-protector" "2"
// NOCAP-SSP-ALL: "-stack-protector" "3"
// NOCAP-SSP-DEFAULT: "-stack-protector" "1"
// NOCAP-SSP-NONE-NOT: "-stack-protector"

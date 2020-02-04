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


// RUN: %riscv32_cheri_purecap_clang -### -c -o /dev/null %s -fstack-protector-strong 2>&1 | \
// RUN:    FileCheck %s --implicit-check-not="-stack-protector" --check-prefix PURECAP-SSP-ERR
// RUN: %riscv32_cheri_purecap_clang -### -c -o /dev/null %s -fstack-protector-strong 2>&1 | \
// RUN:    FileCheck %s --implicit-check-not="-stack-protector" --check-prefix PURECAP-SSP-ERR
// RUN: %cheri_purecap_clang -### -c -o /dev/null %s -fstack-protector-strong 2>&1 | \
// RUN:    FileCheck %s --implicit-check-not="-stack-protector" --check-prefix PURECAP-SSP-ERR

// PURECAP-SSP-ERR: error: unsupported option '-fstack-protector

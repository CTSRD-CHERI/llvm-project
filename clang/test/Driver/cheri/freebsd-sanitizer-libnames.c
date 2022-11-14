// RUN: %riscv64_cheri_clang --sysroot=%S/Inputs/basic_cheribsd_libXXc_tree \
// RUN:   -fsanitize=undefined %s -### 2>&1 | FileCheck --check-prefixes=UBSAN,UBSAN-RISCV64 %s
// RUN: %riscv64_cheri_purecap_clang --sysroot=%S/Inputs/basic_cheribsd_libXXc_tree \
// RUN:   -fsanitize=undefined %s -### 2>&1 | FileCheck --check-prefixes=UBSAN,UBSAN-PURECAP-RISCV64 %s
// RUN: %riscv32_cheri_clang --sysroot=%S/Inputs/basic_cheribsd_libXXc_tree \
// RUN:   -fsanitize=undefined %s -### 2>&1 | FileCheck --check-prefixes=UBSAN,UBSAN-RISCV32 %s
// RUN: %riscv32_cheri_purecap_clang --sysroot=%S/Inputs/basic_cheribsd_libXXc_tree \
// RUN:   -fsanitize=undefined %s -### 2>&1 | FileCheck --check-prefixes=UBSAN,UBSAN-PURECAP-RISCV32 %s
// RUN: %cheri_clang --sysroot=%S/Inputs/basic_cheribsd_libXXc_tree \
// RUN:   -fsanitize=undefined %s -### 2>&1 | FileCheck --check-prefixes=UBSAN,UBSAN-MIPS64 %s
// RUN: %cheri_purecap_clang --sysroot=%S/Inputs/basic_cheribsd_libXXc_tree \
// RUN:   -fsanitize=undefined %s -### 2>&1 | FileCheck --check-prefixes=UBSAN,UBSAN-PURECAP-MIPS64 %s

// UBSAN: "-resource-dir" "[[RESOURCE_DIR:[^"]+]]" "-isysroot"
// UBSAN-SAME: "-fsanitize=alignment,array-bounds,
// UBSAN-SAME: "-fsanitize-recover=alignment,array-bounds,
// UBSAN-NEXT: "/usr/bin/ld"
// UBSAN-RISCV64-SAME:         "[[RESOURCE_DIR]]/lib/freebsd/libclang_rt.ubsan_standalone-riscv64.a"
// UBSAN-PURECAP-RISCV64-SAME: "[[RESOURCE_DIR]]/lib/freebsd/libclang_rt.ubsan_standalone-riscv64c.a"
// UBSAN-RISCV32-SAME:         "[[RESOURCE_DIR]]/lib/freebsd/libclang_rt.ubsan_standalone-riscv32.a"
// UBSAN-PURECAP-RISCV32-SAME: "[[RESOURCE_DIR]]/lib/freebsd/libclang_rt.ubsan_standalone-riscv32c.a"
// UBSAN-MIPS64-SAME:          "[[RESOURCE_DIR]]/lib/freebsd/libclang_rt.ubsan_standalone-mips64.a"
// UBSAN-PURECAP-MIPS64-SAME:  "[[RESOURCE_DIR]]/lib/freebsd/libclang_rt.ubsan_standalone-mips64c128.a"

// RUN: %riscv64_cheri_clang --sysroot=%S/Inputs/basic_cheribsd_libXXc_tree \
// RUN:   -fsanitize=undefined -fsanitize-minimal-runtime %s -### 2>&1 | FileCheck --check-prefixes=UBSAN-MINIMAL,UBSAN-MINIMAL-RISCV64 %s
// RUN: %riscv64_cheri_purecap_clang --sysroot=%S/Inputs/basic_cheribsd_libXXc_tree \
// RUN:   -fsanitize=undefined -fsanitize-minimal-runtime %s -### 2>&1 | FileCheck --check-prefixes=UBSAN-MINIMAL,UBSAN-MINIMAL-PURECAP-RISCV64 %s
// RUN: %riscv32_cheri_clang --sysroot=%S/Inputs/basic_cheribsd_libXXc_tree \
// RUN:   -fsanitize=undefined -fsanitize-minimal-runtime %s -### 2>&1 | FileCheck --check-prefixes=UBSAN-MINIMAL,UBSAN-MINIMAL-RISCV32 %s
// RUN: %riscv32_cheri_purecap_clang --sysroot=%S/Inputs/basic_cheribsd_libXXc_tree \
// RUN:   -fsanitize=undefined -fsanitize-minimal-runtime %s -### 2>&1 | FileCheck --check-prefixes=UBSAN-MINIMAL,UBSAN-MINIMAL-PURECAP-RISCV32 %s
// RUN: %cheri_clang --sysroot=%S/Inputs/basic_cheribsd_libXXc_tree \
// RUN:   -fsanitize=undefined -fsanitize-minimal-runtime %s -### 2>&1 | FileCheck --check-prefixes=UBSAN-MINIMAL,UBSAN-MINIMAL-MIPS64 %s
// RUN: %cheri_purecap_clang --sysroot=%S/Inputs/basic_cheribsd_libXXc_tree \
// RUN:   -fsanitize=undefined -fsanitize-minimal-runtime %s -### 2>&1 | FileCheck --check-prefixes=UBSAN-MINIMAL,UBSAN-MINIMAL-PURECAP-MIPS64 %s

// UBSAN-MINIMAL: "-resource-dir" "[[RESOURCE_DIR:[^"]+]]" "-isysroot"
// UBSAN-MINIMAL-SAME: "-fsanitize=alignment,array-bounds,
// UBSAN-MINIMAL-SAME: "-fsanitize-recover=alignment,array-bounds,
// UBSAN-MINIMAL-NEXT: "/usr/bin/ld"
// UBSAN-MINIMAL-RISCV64-SAME:         "[[RESOURCE_DIR]]/lib/freebsd/libclang_rt.ubsan_minimal-riscv64.a"
// UBSAN-MINIMAL-PURECAP-RISCV64-SAME: "[[RESOURCE_DIR]]/lib/freebsd/libclang_rt.ubsan_minimal-riscv64c.a"
// UBSAN-MINIMAL-RISCV32-SAME:         "[[RESOURCE_DIR]]/lib/freebsd/libclang_rt.ubsan_minimal-riscv32.a"
// UBSAN-MINIMAL-PURECAP-RISCV32-SAME: "[[RESOURCE_DIR]]/lib/freebsd/libclang_rt.ubsan_minimal-riscv32c.a"
// UBSAN-MINIMAL-MIPS64-SAME:          "[[RESOURCE_DIR]]/lib/freebsd/libclang_rt.ubsan_minimal-mips64.a"
// UBSAN-MINIMAL-PURECAP-MIPS64-SAME:  "[[RESOURCE_DIR]]/lib/freebsd/libclang_rt.ubsan_minimal-mips64c128.a"

// FIXME: fsanitize=fuzzer is not accepted for RISC-V
// RUN: %cheri_clang --sysroot=%S/Inputs/basic_cheribsd_libXXc_tree \
// RUN:   -fsanitize=fuzzer %s -### 2>&1 | FileCheck --check-prefixes=FUZZER,FUZZER-MIPS64 %s
// RUN: %cheri_purecap_clang --sysroot=%S/Inputs/basic_cheribsd_libXXc_tree \
// RUN:   -fsanitize=fuzzer %s -### 2>&1 | FileCheck --check-prefixes=FUZZER,FUZZER-PURECAP-MIPS64 %s


// FUZZER: "-resource-dir" "[[RESOURCE_DIR:[^"]+]]" "-isysroot"
// FUZZER-SAME: "-fsanitize-coverage
// FUZZER-SAME: "-fsanitize=fuzzer,fuzzer-no-link"
// FUZZER: "/usr/bin/ld"
// FUZZER-MIPS64-SAME:          "[[RESOURCE_DIR]]/lib/freebsd/libclang_rt.fuzzer-mips64.a"
// FUZZER-MIPS64-SAME:          "[[RESOURCE_DIR]]/lib/freebsd/libclang_rt.ubsan_standalone-mips64.a"
// FUZZER-PURECAP-MIPS64-SAME:  "[[RESOURCE_DIR]]/lib/freebsd/libclang_rt.fuzzer-mips64c128.a"
// FUZZER-PURECAP-MIPS64-SAME:  "[[RESOURCE_DIR]]/lib/freebsd/libclang_rt.ubsan_standalone-mips64c128.a"

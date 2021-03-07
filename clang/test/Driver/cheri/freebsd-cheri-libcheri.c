// RUN: %cheri_purecap_clang -no-canonical-prefixes \
// RUN:   --sysroot=%S/Inputs/basic_cheribsd_libcheri_tree \
// RUN:   %s -### 2>&1 \
// RUN:   | FileCheck --check-prefixes=PURECAP,PURECAP-MIPS64 %s
// RUN: %riscv32_cheri_purecap_clang -no-canonical-prefixes \
// RUN:   --sysroot=%S/Inputs/basic_cheribsd_libcheri_tree \
// RUN:   %s -### 2>&1 \
// RUN:   | FileCheck --check-prefixes=PURECAP,PURECAP-RISCV32 %s
// RUN: %riscv64_cheri_purecap_clang -no-canonical-prefixes \
// RUN:   --sysroot=%S/Inputs/basic_cheribsd_libcheri_tree \
// RUN:   %s -### 2>&1 \
// RUN:   | FileCheck --check-prefixes=PURECAP,PURECAP-RISCV64 %s
// PURECAP-MIPS64:  "-cc1" "-triple" "mips64c128-unknown-freebsd-purecap"
// PURECAP-MIPS64:  "-target-abi" "purecap"
// PURECAP-RISCV32: "-cc1" "-triple" "riscv32-unknown-freebsd"
// PURECAP-RISCV32: "-target-abi" "il32pc64"
// PURECAP-RISCV64: "-cc1" "-triple" "riscv64-unknown-freebsd"
// PURECAP-RISCV64: "-target-abi" "l64pc128"
// PURECAP: ld{{.*}}" "--sysroot=[[SYSROOT:[^"]+]]"
// PURECAP: "-dynamic-linker" "{{.*}}/libexec/ld-elf.so.1"
// PURECAP: "-L[[SYSROOT]]/usr/libcheri"
// PURECAP-NOT: "-L[[SYSROOT]]/usr/lib"
// PURECAP-NOT: "{{.*}}crti.o"
// PURECAP-NOT: "{{.*}}crtn.o"

// RUN: %cheri_purecap_clang -no-canonical-prefixes \
// RUN:   --sysroot=%S/Inputs/basic_cheribsd_libcheri_tree \
// RUN:   -fsanitize=fuzzer %s -### 2>&1 \
// RUN:  | FileCheck --check-prefix=FUZZER-PURECAP64 %s

// FUZZER-PURECAP64: "/usr/bin/ld"
// FUZZER-PURECAP64: "--whole-archive" "{{.+}}/lib/freebsd/libclang_rt.fuzzer-mips64c128.a" "--no-whole-archive"
// FUZZER-PURECAP64: "--whole-archive" "{{.+}}/lib/freebsd/libclang_rt.ubsan_standalone-mips64c128.a" "--no-whole-archive"

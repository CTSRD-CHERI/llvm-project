// RUN: %plain_clang_cheri_triple_allowed -no-canonical-prefixes \
// RUN:   -target cheri-pc-freebsd11 %s                              \
// RUN:   --sysroot=%S/Inputs/basic_cheribsd_libcheri_tree -### 2>&1 \
// RUN:   | FileCheck --check-prefix=CHECK-CHERI %s
// CHECK-CHERI: "-cc1" "-triple" "cheri-pc-freebsd11"
// CHECK-CHERI: ld{{.*}}" "--sysroot=[[SYSROOT:[^"]+]]"
// CHECK-CHERI: "--eh-frame-hdr" "-dynamic-linker" "{{.*}}ld-elf{{.*}}" "-o" "a.out" "{{.*}}crt1.o" "{{.*}}crti.o" "{{.*}}crtbegin.o" "-L[[SYSROOT]]/usr/lib" "{{.*}}.o" "--start-group" "-lgcc" "--as-needed" "-lgcc_s" "--no-as-needed" "-lc" "-lgcc" "--as-needed" "-lgcc_s" "--no-as-needed" "--end-group" "{{.*}}crtend.o" "{{.*}}crtn.o"
//
// RUN: %plain_clang_cheri_triple_allowed -no-canonical-prefixes \
// RUN:   -target cheri-unknown-freebsd -mabi=purecap %s -cheri=128  \
// RUN:   --sysroot=%S/Inputs/basic_cheribsd_libcheri_tree -### 2>&1 \
// RUN:   | FileCheck --check-prefixes=CHECK-CHERI-PURECAP,PURECAP-MIPS128 %s
// RUN: %cheri_purecap_clang -no-canonical-prefixes \
// RUN:   %s --sysroot=%S/Inputs/basic_cheribsd_libcheri_tree -### 2>&1 \
// RUN:   | FileCheck --check-prefixes=CHECK-CHERI-PURECAP,PURECAP-MIPS128 %s
// RUN: %riscv32_cheri_purecap_clang -no-canonical-prefixes \
// RUN:   %s --sysroot=%S/Inputs/basic_cheribsd_libcheri_tree -### 2>&1 \
// RUN:   | FileCheck --check-prefixes=CHECK-CHERI-PURECAP,PURECAP-RISCV32 %s
// RUN: %riscv64_cheri_purecap_clang -no-canonical-prefixes \
// RUN:   %s --sysroot=%S/Inputs/basic_cheribsd_libcheri_tree -### 2>&1 \
// RUN:   | FileCheck --check-prefixes=CHECK-CHERI-PURECAP,PURECAP-RISCV64 %s
// PURECAP-MIPS128: "-cc1" "-triple" "mips64c128-unknown-freebsd-purecap"
// PURECAP-MIPS128: "-target-abi" "purecap"
// PURECAP-RISCV32: "-cc1" "-triple" "riscv32-unknown-freebsd"
// PURECAP-RISCV32: "-target-abi" "il32pc64"
// PURECAP-RISCV64: "-cc1" "-triple" "riscv64-unknown-freebsd"
// PURECAP-RISCV64: "-target-abi" "l64pc128"
// CHECK-CHERI-PURECAP: ld{{.*}}" "--sysroot=[[SYSROOT:[^"]+]]"
// CHECK-CHERI-PURECAP: "-dynamic-linker" "{{.*}}/libexec/ld-elf.so.1"
// CHECK-CHERI-PURECAP: "-L[[SYSROOT]]/usr/libcheri"
// CHECK-CHERI-PURECAP-NOT: "-L[[SYSROOT]]/usr/lib"
// CHECK-CHERI-PURECAP-NOT: "{{.*}}crti.o"
// CHECK-CHERI-PURECAP-NOT: "{{.*}}crtn.o"


// RUN: %plain_clang_cheri_triple_allowed -no-canonical-prefixes \
// RUN:   -target cheri-unknown-freebsd11 -mabi=purecap %s -cheri -### 2>&1 \
// RUN:   | FileCheck --check-prefix=CHECK-CHERI-PURECAP128 %s
// CHECK-CHERI-PURECAP128: "-cc1" "-triple" "mips64c128-unknown-freebsd11-purecap"

// RUN: %plain_clang_cheri_triple_allowed -no-canonical-prefixes \
// RUN:   -target cheri-unknown-freebsd11 -mabi=purecap %s -cheri=256 -### 2>&1 \
// RUN:   | FileCheck --check-prefix=CHECK-CHERI-PURECAP256 %s
// CHECK-CHERI-PURECAP256: "-cc1" "-triple" "mips64c256-unknown-freebsd11-purecap"

// RUN: %plain_clang_cheri_triple_allowed -no-canonical-prefixes \
// RUN:   -target cheri-unknown-freebsd11 -mabi=purecap %s -cheri=64 -### 2>&1 \
// RUN:   | FileCheck --check-prefix=CHECK-CHERI-PURECAP64 %s
// CHECK-CHERI-PURECAP64: "-cc1" "-triple" "mips64c64-unknown-freebsd11-purecap"

// RUN: %plain_clang_cheri_triple_allowed -no-canonical-prefixes \
// RUN:   -target cheri-pc-freebsd11 -mabi=purecap %s -fsanitize=fuzzer -cheri=64 \
// RUN:   --sysroot=%S/Inputs/basic_cheribsd_libcheri_tree -### 2>&1 | FileCheck --check-prefix=CHECK-FUZZER-PURECAP64 %s

// CHECK-FUZZER-PURECAP64: "/usr/bin/ld"
// CHECK-FUZZER-PURECAP64: "--whole-archive" "{{.+}}/lib/freebsd/libclang_rt.fuzzer-mips64c64.a" "--no-whole-archive"
// CHECK-FUZZER-PURECAP64: "--whole-archive" "{{.+}}/lib/freebsd/libclang_rt.ubsan_standalone-mips64c64.a" "--no-whole-archive"

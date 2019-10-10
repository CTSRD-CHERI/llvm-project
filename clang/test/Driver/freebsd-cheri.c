// RUN: %plain_clang_cheri_triple_allowed -no-canonical-prefixes \
// RUN:   -target cheri-pc-freebsd11 %s                              \
// RUN:   --sysroot=%S/Inputs/basic_cheribsd_libcheri_tree -### 2>&1 \
// RUN:   | FileCheck --check-prefix=CHECK-CHERI %s
// CHECK-CHERI: "-cc1" "-triple" "cheri-pc-freebsd11"
// CHECK-CHERI: ld{{.*}}" "--sysroot=[[SYSROOT:[^"]+]]"
// CHECK-CHERI: "--eh-frame-hdr" "-dynamic-linker" "{{.*}}ld-elf{{.*}}" "-o" "a.out" "{{.*}}crt1.o" "{{.*}}crti.o" "{{.*}}crtbegin.o" "-L[[SYSROOT]]/usr/lib" "{{.*}}.o" "--start-group" "-lgcc" "--as-needed" "-lgcc_s" "--no-as-needed" "-lc" "-lgcc" "--as-needed" "-lgcc_s" "--no-as-needed" "--end-group" "{{.*}}crtend.o" "{{.*}}crtn.o"
//
// RUN: %plain_clang_cheri_triple_allowed -no-canonical-prefixes \
// RUN:   -target cheri-pc-freebsd11 -mabi=purecap %s -cheri=128  \
// RUN:   --sysroot=%S/Inputs/basic_cheribsd_libcheri_tree -### 2>&1 \
// RUN:   | FileCheck --check-prefix=CHECK-CHERI-SANDBOX %s
// CHECK-CHERI-SANDBOX: "-cc1" "-triple" "mips64c128-pc-freebsd11-purecap"
// CHECK-CHERI-SANDBOX: ld{{.*}}" "--sysroot=[[SYSROOT:[^"]+]]"
// CHECK-CHERI-SANDBOX: "-dynamic-linker" "{{.*}}/libexec/ld-elf.so.1"
// CHECK-CHERI-SANDBOX: "-L[[SYSROOT]]/usr/libcheri"
// CHECK-CHERI-SANDBOX-NOT: "-L[[SYSROOT]]/usr/lib"
// CHECK-CHERI-SANDBOX-NOT: "{{.*}}crti.o"
// CHECK-CHERI-SANDBOX-NOT: "{{.*}}crtn.o"


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

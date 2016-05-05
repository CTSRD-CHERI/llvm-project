// RUN: %clang -no-canonical-prefixes \
// RUN:   -target cheri-pc-freebsd11 %s                              \
// RUN:   --sysroot=%S/Inputs/basic_freebsd64_tree -### 2>&1 \
// RUN:   | FileCheck --check-prefix=CHECK-CHERI %s
// CHECK-CHERI: "-cc1" "-triple" "cheri-pc-freebsd11"
// CHECK-CHERI: ld{{.*}}" "--sysroot=[[SYSROOT:[^"]+]]"
// CHECK-CHERI: "--eh-frame-hdr" "-dynamic-linker" "{{.*}}ld-elf{{.*}}" "-o" "a.out" "{{.*}}crt1.o" "{{.*}}crti.o" "{{.*}}crtbegin.o" "-L[[SYSROOT]]/usr/lib" "{{.*}}.o" "-lgcc" "--as-needed" "-lgcc_s" "--no-as-needed" "-lc" "-lgcc" "--as-needed" "-lgcc_s" "--no-as-needed" "{{.*}}crtend.o" "{{.*}}crtn.o"
//
// RUN: %clang -no-canonical-prefixes \
// RUN:   -target cheri-pc-freebsd11 -mabi=sandbox %s        \
// RUN:   --sysroot=%S/Inputs/basic_freebsd64_tree -### 2>&1 \
// RUN:   | FileCheck --check-prefix=CHECK-CHERI-SANDBOX %s
// CHECK-CHERI-SANDBOX: "-cc1" "-triple" "cheri-pc-freebsd11"
// CHECK-CHERI-SANDBOX: ld{{.*}}" "--sysroot=[[SYSROOT:[^"]+]]"
// CHECK-CHERI-SANDBOX: "-dynamic-linker" "{{.*}}/libexec/ld-cheri-elf.so.1"
// CHECK-CHERI-SANDBOX: "-L[[SYSROOT]]/usr/libcheri"
// CHECK-CHERI-SANDBOX-NOT: "-L[[SYSROOT]]/usr/lib"
// CHECK-CHERI-SANDBOX-NOT: "{{.*}}crti.o"
// CHECK-CHERI-SANDBOX-NOT: "{{.*}}crtn.o"

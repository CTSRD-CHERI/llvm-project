// RUN: %clang %s -### -no-canonical-prefixes --target=x86_64-unknown-rtems5 \
// RUN:     --sysroot=%S/platform 2>&1 \
// RUN:     | FileCheck -check-prefixes=CHECK,CHECK-X86_64 %s
// RUN: %clang %s -### -no-canonical-prefixes --target=aarch64-unknown-rtems5 \
// RUN:     --sysroot=%S/platform 2>&1 \
// RUN:     | FileCheck -check-prefixes=CHECK,CHECK-AARCH64 %s
// CHECK: {{.*}}clang{{.*}}" "-cc1"
// CHECK: "-munwind-tables"
// CHECK: "-fuse-init-array"
// CHECK: "-isysroot" "[[SYSROOT:[^"]+]]"
// CHECK: "-internal-externc-isystem" "[[SYSROOT]]{{/|\\\\}}include"
// CHECK: {{.*}}ld.lld{{.*}}" "-z" "rodynamic"
// CHECK: "--sysroot=[[SYSROOT]]"
// CHECK: "--build-id"
// CHECK-NOT: "--hash-style=gnu"
// CHECK: crt0.o
// CHECK-NOT: crti.o
// CHECK-NOT: crtbegin.o
// CHECK: "-L[[SYSROOT]]{{/|\\\\}}lib"
// CHECK-X86_64: "[[SYSROOT]]{{/|\\\\}}lib{{[/\\]}}libclang_rt.builtins-x86_64.a"
// CHECK-AARCH64: "[[SYSROOT]]{{/|\\\\}}lib{{[/\\]}}libclang_rt.builtins-aarch64.a"
// CHECK: "-lc"
// CHECK-NOT: crtend.o
// CHECK-NOT: crtn.o

// RUN: %clang %s -### --target=x86_64-unknown-rtems5 -rtlib=libgcc 2>&1 \
// RUN:     | FileCheck %s -check-prefix=CHECK-RTLIB
// CHECK-RTLIB: error: invalid runtime library name in argument '-rtlib=libgcc'

// RUN: %clang %s -### --target=x86_64-unknown-rtems5 -static 2>&1 \
// RUN:     | FileCheck %s -check-prefix=CHECK-STATIC
// CHECK-STATIC: "-Bstatic"
// CHECK-STATIC: "-lc"

// RUN: %clang %s -### --target=mips64-unknown-rtems5 -shared 2>&1 | FileCheck %s -check-prefix=CHECK-SHARED
// CHECK-SHARED: error: unsupported option '-shared'

// RUN: %clang %s -### --target=x86_64-unknown-rtems5 -r 2>&1 \
// RUN:     | FileCheck %s -check-prefix=CHECK-RELOCATABLE
// CHECK-RELOCATABLE-NOT: "-pie"
// CHECK-RELOCATABLE-NOT: "--build-id"
// CHECK-RELOCATABLE: "-r"

// Check the -qrtems flag toggles start.o vs crt0.o:
// RUN: %clang %s -### --target=mips64-unknown-rtems5 -qrtems --sysroot=%S/platform 2>&1 | FileCheck \
// RUN:   -check-prefixes=QRTEMS %s -implicit-check-not crt0.o
// RUN: %clang %s -### --target=mips64-unknown-rtems5 --sysroot=%S/platform 2>&1 | FileCheck \
// RUN:   -check-prefixes=NO-QRTEMS %s -implicit-check-not start.o

// QRTEMS: {{.+}}bin/ld.lld"
// QRTEMS-SAME: "-Tlinkcmds" "-lrtemsbsp" "-lrtemscpu"
// NO-QRTEMS: {{.+}}bin/ld.lld"
// NO-QRTEMS-SAME: lib/crt0.o"
// NO-QRTEMS-NOT: "-lrtemsbsp"
// NO-QRTEMS-NOT: "-lrtemscpu"
